#ifndef __TREE_WORKER_H__
#define __TREE_WORKER_H__

#include "core/Board.h"
#include "Node.h"
#include "Playout/PlayoutPolicy.h"
#include "Selection/SelectionPolicy.h"
#include "core/RandomGenerator.h"
#include <mutex>
#include <thread>
#include "lurien.h"

// This class performs the MCTS algorithm to find the best move.
// It contains the code that is executed by each worker thread.
template<class SP, class PP>
class TreeWorker
{
public:
    TreeWorker(const Board& pos, Node* root, uint64_t seed) : _pos(&pos)
    {
        _root = root;
        _gen = std::make_unique<RandomGenerator>(seed);
        _sp = std::make_unique<SP>();
        _pp = std::make_unique<PP>();
    }

    // Start a searching thread.
    void Start()
    {
        _stop = false;
        std::thread worker([&] { DoSearch(); });
        worker.detach();
    }

    // Stop the currently executing search.
    void Stop()
    {
        _stop = true;

        // Block until the mutex is available.
        std::unique_lock<std::mutex> lock(_mtx);
    }

private:
    bool _stop = false;
    Node* _root;
    std::unique_ptr<SP> _sp;
    std::unique_ptr<PP> _pp;
    std::unique_ptr<RandomGenerator> _gen;
    Board const* _pos;
    std::mutex _mtx;

    // This method keeps searching until a call to Stop is made.
    void DoSearch()
    {
        LURIEN_SCOPE(search)

        std::unique_lock<std::mutex> lock(_mtx);

        int boardSize = _pos->Size();
        int boardArea = boardSize*boardSize;
        Board temp(_pos->Size());
        Colour* playerOwned = new Colour[boardArea];
        while (!_stop)
        {
            // Clone the board state.
            temp.CloneFrom(*_pos);

            // Reset the player ownership map.
            memset(playerOwned, None, boardArea*sizeof(Colour));

            Node* leaf = SelectNode(temp, playerOwned);

            // Perform a playout and record the result.
            int res = Simulate(temp, leaf->Stats.LastMove, playerOwned);

            // Backpropagate the scores.
            UpdateScores(leaf, playerOwned, res);
        }

        delete[] playerOwned;
    }

    Node* SelectNode(Board& temp, Colour* playerOwned) const
    {
        LURIEN_SCOPE(select)

        // Find the leaf node which must be expanded.
        Node* leaf = Select(temp, _root, playerOwned);

        // Expand the leaf node.
        leaf = Expand(temp, leaf, playerOwned);

        return leaf;
    }

    // Select a node to expand.
    Node* Select(Board& temp, Node* root, Colour* playerOwned) const
    {
        Node* current = root;
        while (current->Stats.Visits >= (int)current->Children.size() && current->HasChildren())
        {
            std::lock_guard<std::mutex> lk(current->Obj);
            current = _sp->Select(current->Children);
            current->Stats.VirtualLoss();

            const Move& move = current->Stats.LastMove;
            temp.MakeMove(move);

            // Update the ownership map.
            UpdateOwnership(move, playerOwned);
        }

        return current;
    }

    // Expand the chosen leaf node.
    Node* Expand(Board& temp, Node* leaf, Colour* playerOwned) const
    {
        LURIEN_SCOPE(expand)

        Node* expanded = leaf;
        std::lock_guard<std::mutex> lk(expanded->Obj);
        if (!expanded->HasChildren())
        {
            expanded->Moves = temp.GetMoves();
            expanded->AddChildren();

            if (expanded->HasChildren())
            {
                // Select the best according to the priors.
                expanded = _sp->Select(expanded->Children);
                expanded->Stats.VirtualLoss();

                const Move& move = expanded->Stats.LastMove;
                temp.MakeMove(move);

                // Update the ownership map.
                UpdateOwnership(move, playerOwned);
            }
        }

        return expanded;
    }

    // Perform a simulation from the specified game state.
    int Simulate(Board& temp, const Move& lastMove, Colour* playerOwned) const
    {
        LURIEN_SCOPE(simulate)

        // Make moves according to the playout policy until a terminal state is reached.
        Move move = lastMove;
        while ((move = _pp->Select(temp, move)) != BadMove)
        {
            UpdateOwnership(move, playerOwned);
            temp.MakeMove(move);
        }

        return temp.Score();
    }

    // Update the ownership map used in RAVE calculations.
    void UpdateOwnership(const Move& move, Colour* playerOwned) const
    {
        int coord = move.Coord;
        if (coord != PassCoord && playerOwned[coord] == None)
        {
            playerOwned[coord] = move.Col;
        }
    }

    // Backpropagate the score from the simulation up the tree.
    void UpdateScores(Node* leaf, Colour* playerOwned, int score) const
    {
        LURIEN_SCOPE(update)

        // Backtrack the scores up the tree.
        while (leaf != nullptr)
        {
            std::lock_guard<std::mutex> lk(leaf->Obj);

            // RAVE update all children of leaf.
            RaveUpdate(leaf, playerOwned, score);

            MoveStats& stats = leaf->Stats;

            // Reverse the effects of virtual losses.
            stats.VirtualWin();
            stats.UpdateScore(score);
            leaf = leaf->Parent;
        }

        // Give the root node a visit (some selection policies need this).
        _root->Stats.Visits++;
    }

    // The RAVE update effects all children of this node.
    // The ultimate effect is that all siblings of nodes that were traversed during selection phase 
    // will potentially be updated.
    void RaveUpdate(Node* node, Colour* playerOwned, int score) const
    {
        // Update the node if possible.
        if (node != nullptr)
        {
            for (Node* child : node->Children)
            {
                // Update this child's stats.
                MoveStats& stats = child->Stats;
                int coord = stats.LastMove.Coord;
                Colour col = stats.LastMove.Col;
                if (coord != PassCoord && playerOwned[coord] == col)
                {
                    // This is valid evidence for the node.
                    stats.UpdateRaveScore(score);
                }
            }
        }
    }
};

#endif // __TREE_WORKER_H__
