#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "../Args.h"
#include "../Board.h"
#include "Node.h"
#include "Playout/PlayoutPolicy.h"
#include "Selection/SelectionPolicy.h"
#include "../RandomGenerator.h"
#include "TreeWorker.h"
#include <mutex>
#include <thread>

// This class performs executes the MCTS algorithm to find the best move.
// The template parameters are:
// SP: The selection policy to be used.
// PP: The playout policy to be used.
template<class SP, class PP>
class Search
{
static_assert(std::is_base_of<SelectionPolicy, SP>::value, "Not a valid selection policy.");
static_assert(std::is_base_of<PlayoutPolicy, PP>::value, "Not a valid playout policy.");
public:
    Search()
    {
        // Decide on how many threads to use.
        auto args = Args::Get();
        if (!args->TryParse("-threads", _numWorkersToUse))
        {
            int numCores = std::thread::hardware_concurrency();
            _numWorkersToUse = numCores == 0 ? DefaultNumWorkers : numCores;
        }
    }

    ~Search()
    {
        if (_root != nullptr)
        {
            delete _root;
            _root = nullptr;
        }

        for (TreeWorker<SP, PP>* worker : _workers) delete worker;
        _workers.clear();
    }

    inline MoveStats Best() const { return _best; }

    inline int TreeSize() const { return _treeSize; }

    // Kick off the searching threads.
    void Start(const Board& pos)
    {
        // Create the root of the tree.
        if (_root != nullptr) delete _root;
        _root = MakeRoot();
        _root->Moves = pos.GetMoves();

        // Create a PRNG which is used to seed each worker's PRNG.
        RandomGenerator seeder;

        // Create the workers.
        for (TreeWorker<SP, PP>* worker : _workers) delete worker;
        _workers.clear();

        for (int i = 0; i < _numWorkersToUse; i++)
        {
            auto gen = new RandomGenerator(seeder.Next());
            _workers.push_back(new TreeWorker<SP, PP>(pos, _root, gen));
        }

        // Start the workers.
        for (TreeWorker<SP, PP>* worker : _workers) worker->Start();
    }

    // Stop the worker threads.
    void Stop()
    {
        for (TreeWorker<SP, PP>* worker : _workers) worker->Stop();
        CollateResults();
    }

private:
    const int DefaultNumWorkers = 2;
    int _numWorkersToUse;

    bool _stop = false;
    Node* _root = nullptr;
    std::vector<TreeWorker<SP, PP>*> _workers;

    int _treeSize = 0;
    MoveStats _best;

    void CollateResults()
    {
        // Find the most promising move and cache stats.
        int highestVisits = -1;
        for (size_t i = 0; i < _root->Children.size(); i++)
        {
            Node const* const child = _root->Children[i];
            if (child->Stats.Visits > highestVisits)
            {
                highestVisits = child->Stats.Visits;
                _best = child->Stats;
            }

            _treeSize += child->Stats.Visits;
        }
    }
};

#endif // __SEARCH_H__
