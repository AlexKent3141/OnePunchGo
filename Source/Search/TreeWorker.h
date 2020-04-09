#ifndef __TREE_WORKER_H__
#define __TREE_WORKER_H__

#include "../Board.h"
#include "Node.h"
#include "Playout/PlayoutPolicy.h"
#include "Selection/SelectionPolicy.h"
#include "../RandomGenerator.h"
#include <mutex>
#include <thread>

// This class performs the MCTS algorithm to find the best move.
// It contains the code that is executed by each worker thread.
template<class SP, class PP>
class TreeWorker
{
public:
    TreeWorker(
        const Board& pos,
        std::shared_ptr<Node> root,
        uint64_t seed)
    :
        _pos(&pos)
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
    std::shared_ptr<Node> _root;
    std::unique_ptr<SP> _sp;
    std::unique_ptr<PP> _pp;
    std::unique_ptr<RandomGenerator> _gen;
    Board const* _pos;
    std::mutex _mtx;

    // This method keeps searching until a call to Stop is made.
    void DoSearch()
    {
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

            std::shared_ptr<Node> leaf =
                SelectNode(temp, playerOwned);

            // Perform a playout and record the result.
            int res = Simulate(temp, leaf->Stats.LastMove, playerOwned);

            // Backpropagate the scores.
            UpdateScores(leaf, playerOwned, res);
        }

        delete[] playerOwned;
    }

    std::shared_ptr<Node> SelectNode(
        Board& temp,
        Colour* playerOwned) const
    {
        // Find the leaf node which must be expanded.
        std::shared_ptr<Node> leaf =
            Select(temp, playerOwned);

        // Expand the leaf node.
        leaf = Expand(temp, leaf, playerOwned);

        return leaf;
    }

    // Select a node to expand.
    std::shared_ptr<Node> Select(
        Board& temp,
        Colour* playerOwned) const
    {
        std::shared_ptr<Node> current = _root;
        while (
            current->Stats.Visits >= (int)current->Children.size() &&
            current->HasChildren())
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
    std::shared_ptr<Node> Expand(
        Board& temp,
        std::shared_ptr<Node> leaf,
        Colour* playerOwned) const
    {
        std::shared_ptr<Node> expanded = leaf;
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
    int Simulate(
        Board& temp,
        const Move& lastMove,
        Colour* playerOwned) const
    {
        // Make moves according to the playout policy until a
        // terminal state is reached.
        Move move = lastMove;
        while ((move = _pp->Select(temp, move)) != BadMove)
        {
            UpdateOwnership(move, playerOwned);
            temp.MakeMove(move);
        }

        return temp.Score();
    }

    // Update the ownership map used in RAVE calculations.
    void UpdateOwnership(
        const Move& move,
        Colour* playerOwned) const
    {
        int coord = move.Coord;
        if (coord != PassCoord && playerOwned[coord] == None)
        {
            playerOwned[coord] = move.Col;
        }
    }

    // Backpropagate the score from the simulation up the tree.
    void UpdateScores(
        std::shared_ptr<Node> leaf,
        Colour* playerOwned,
        int score) const
    {
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

            leaf = leaf->Parent.expired() ? nullptr : leaf->Parent.lock();
        }

        // Give the root node a visit (some selection policies need this).
        _root->Stats.Visits++;
    }

    // The RAVE update effects all children of this node.
    // The ultimate effect is that all siblings of nodes that were
    // traversed during selection phase will potentially be updated.
    void RaveUpdate(
        std::shared_ptr<Node> node,
        Colour* playerOwned,
        int score) const
    {
        // Update the node if possible.
        if (node != nullptr)
        {
            for (std::shared_ptr<Node> child : node->Children)
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
