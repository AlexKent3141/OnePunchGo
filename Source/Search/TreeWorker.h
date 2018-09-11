#ifndef __TREE_WORKER_H__
#define __TREE_WORKER_H__

#include "../Board.h"
#include "Node.h"
#include "Playout/PlayoutPolicy.h"
#include "Selection/SelectionPolicy.h"
#include "../RandomGenerator.h"
#include "../NeuralNet.h"
#include <mutex>
#include <thread>

// This class performs the MCTS algorithm to find the best move.
// It contains the code that is executed by each worker thread.
template<class SP, class PP>
class TreeWorker
{
public:
    TreeWorker(size_t id, const Board& pos, Node* root, RandomGenerator* gen, bool useNN) : _pos(&pos)
    {
        _root = root;
        _gen = gen;
        _sp = new SP();
        _pp = new PP();

        if (useNN)
        {
            _selNet = NeuralNet::GetSelectionNetwork(id);
            _valNet = NeuralNet::GetValueNetwork(id);
        }
    }

    ~TreeWorker()
    {
        Stop();

        if (_gen != nullptr)
        {
            delete _gen;
            _gen = nullptr;
        }

        if (_sp != nullptr)
        {
            delete _sp;
            _sp = nullptr;
        }
        
        if (_pp != nullptr)
        {
            delete _pp;
            _pp = nullptr;
        }
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
    Node* _root = nullptr;
    SP* _sp = nullptr;
    PP* _pp = nullptr;
    RandomGenerator* _gen = nullptr;
    NeuralNet* _selNet = nullptr;
    NeuralNet* _valNet = nullptr;
    Board const* _pos;
    std::mutex _mtx;

    // This method keeps searching until a call to Stop is made.
    void DoSearch()
    {
        std::unique_lock<std::mutex> lock(_mtx);

        int boardSize = _pos->Size();
        int boardArea = boardSize*boardSize;
        Board temp(_pos->Size());
        Colour playerOwned[boardArea];
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
    }

    Node* SelectNode(Board& temp, Colour* playerOwned)
    {
        // Find the leaf node which must be expanded.
        Node* leaf = Select(temp, _root, playerOwned);

        // Expand the leaf node.
        leaf = Expand(temp, leaf, playerOwned);

        return leaf;
    }

    // Select a node to expand.
    Node* Select(Board& temp, Node* root, Colour* playerOwned)
    {
        Node* current = root;
        while (current->HasChildren())
        {
            std::lock_guard<std::mutex> lk(current->Obj);
            current = _sp->Select(temp, current->Children, _selNet);
            current->Stats.VirtualLoss();

            const Move& move = current->Stats.LastMove;
            temp.MakeMove(move);

            // Update the ownership map.
            int coord = move.Coord;
            if (coord != PassCoord && playerOwned[coord] == None)
            {
                playerOwned[coord] = move.Col;
            }
        }

        return current;
    }

    // Expand the chosen leaf node.
    Node* Expand(Board& temp, Node* leaf, Colour* playerOwned) const
    {
        Node* current = leaf;
        std::lock_guard<std::mutex> lk(current->Obj);

        // Expand the selected node (if it's been visited sufficiently many times).
        const int MinVisitsForExpansion = 4;
        if (current->Stats.Visits >= MinVisitsForExpansion)
        {
            current->Expand(temp.GetMoves());
            if (current->HasChildren())
            {
                current = _sp->Select(temp, current->Children, _selNet);
                current->Stats.VirtualLoss();

                const Move& move = current->Stats.LastMove;
                temp.MakeMove(move);

                // Update the ownership map.
                int coord = move.Coord;
                if (coord != PassCoord && playerOwned[coord] == None)
                {
                    playerOwned[coord] = move.Col;
                }
            }
        }

        return current;
    }

    // Perform a simulation from the specified game state.
    int Simulate(Board& temp, const Move& lastMove, Colour* playerOwned)
    {
        if (_valNet != nullptr)
        {
            return NNScore(temp, playerOwned);
        }

        // Make moves according to the playout policy until a terminal state is reached.
        Move move = lastMove;
        while ((move = _pp->Select(temp, move)) != BadMove)
        {
            int coord = move.Coord;
            if (coord != PassCoord && playerOwned[coord] == None)
            {
                playerOwned[coord] = move.Col;
            }

            temp.MakeMove(move);
        }

        return temp.Score();
    }

    int NNScore(const Board& board, Colour* playerOwned)
    {
        // Feed the board through the value net.
        double total;
        std::vector<double> output = _valNet->Evaluate(board, total);

        // Compute the score from the perspective of the player to move.
        double count = 0;
        for (const auto& out : output) count += (2*out - 1);

        // Adjust the sign so that a black win would be positive.
        if (board.ColourToMove() == White) count *= -1;

        // Take komi into account.
        count -= CurrentRules.Komi;

        // Attempt to account for errors in the network evaluation.
        const int PossibleError = 50;
        double guessCount = count - PossibleError + _gen->NextDouble() * 2 * PossibleError;

        return guessCount > 0 ? 1 : guessCount < 0 ? -1 : 0;
    }

    // Backpropagate the score from the simulation up the tree.
    void UpdateScores(Node* leaf, Colour* playerOwned, int score) const
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
            leaf = leaf->Parent;
        }
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
