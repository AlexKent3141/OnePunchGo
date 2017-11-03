#ifndef __TREE_WORKER_H__
#define __TREE_WORKER_H__

#include "../Board.h"
#include "Node.h"
#include "../Playout/PlayoutPolicy.h"
#include "../Selection/SelectionPolicy.h"
#include "../RandomGenerator.h"
#include <mutex>
#include <thread>

// This class performs the MCTS algorithm to find the best move.
// It contains the code that is executed by each worker thread.
template<class SP, class PP>
class TreeWorker
{
public:
    TreeWorker(const Board& pos, Node* root, RandomGenerator* gen)
    {
        _root = root;
        _gen = gen;
        _sp = new SP();
        _pp = new PP();
    }

    ~TreeWorker()
    {
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
    std::mutex _mtx;

    // This method keeps searching until a call to Stop is made.
    void DoSearch(const Board& pos)
    {
        std::unique_lock<std::mutex> lock(_mtx);

        _root->Moves = pos.GetMoves();

        int boardSize = pos.Size();
        int boardArea = boardSize*boardSize;
        Board temp(pos.Size());
        _treeSize = 0;
        _stop = false;
        int playoutMoves[boardArea];
        while (!_stop)
        {
            // Clone the board state.
            temp.CloneFrom(pos);

            // Find the leaf node which must be expanded.
            Node* leaf = Select(temp, root);

            Colour selectedPlayer = leaf->Stats.LastMove.Col;

            // Expand the leaf node.
            leaf = Expand(temp, leaf);

            // Perform a playout and record the result.
            memset(playoutMoves, -1, boardArea*sizeof(int));
            int res = Simulate(temp, playoutMoves);

            // Backpropagate the scores.
            UpdateScores(leaf, selectedPlayer, playoutMoves, res);

            ++_treeSize;
        }

        // Find the most promising move.
        int highestVisits = -1;
        for (size_t i = 0; i < root->Children.size(); i++)
        {
            Node const* const child = root->Children[i];
            if (child->Stats.Visits > highestVisits)
            {
                highestVisits = child->Stats.Visits;
                _best = child->Stats;
            }
        }

        delete root;
    }

    // Select a node to expand.
    Node* Select(Board& temp, Node* root)
    {
        Node* current = root;
        while (current->FullyExpanded() && current->HasChildren())
        {
            current = _sp.Select(current->Children);
            temp.MakeMove(current->Stats.LastMove);
        }

        return current;
    }

    // Expand the chosen leaf node.
    Node* Expand(Board& temp, Node* leaf) const
    {
        Node* expanded = leaf;
        if (!expanded->FullyExpanded())
        {
            expanded = expanded->ExpandNext();
            temp.MakeMove(expanded->Stats.LastMove);
            expanded->Moves = temp.GetMoves();
        }

        return expanded;
    }

    // Perform a simulation from the specified game state.
    int Simulate(Board& temp, int* playoutMoves)
    {
        // Make moves according to the playout policy until a terminal state is reached.
        int turnNo = 0;
        Move move;
        while ((move = _pp.Select(temp)) != BadMove)
        {
            if ((turnNo & 1) && move.Coord != PassCoord && playoutMoves[move.Coord] == -1)
                playoutMoves[move.Coord] = turnNo;

            temp.MakeMove(move);
            ++turnNo;
        }

        return temp.Score();
    }

    // Backpropagate the score from the simulation up the tree.
    void UpdateScores(Node* leaf, Colour selectedPlayer, int* playoutMoves, int score) const
    {
        while (leaf != nullptr)
        {
            MoveStats& stats = leaf->Stats;
            if (stats.LastMove.Col == selectedPlayer)
            {
                RaveUpdate(leaf, playoutMoves, score);
            }

            stats.UpdateScore(score);
            leaf = leaf->Parent;
        }
    }

    void RaveUpdate(Node* node, int* playoutMoves, int score) const
    {
        for (Node* child : node->Children)
        {
            MoveStats& stats = child->Stats;
            int coord = stats.LastMove.Coord;
            if (coord != PassCoord && playoutMoves[coord] != -1)
            {
                stats.UpdateRaveScore(score);
            }
        }
    }
};

#endif // __TREE_WORKER_H__
