#ifndef __MC_RAVE_PRIORS_H__
#define __MC_RAVE_PRIORS_H__

#include "MCRave.h"
#include "../../NeuralNet.h"
#include <utility>
#include <cassert>

// The MCRave selection policy with priors associated to some types of moves.
class MCRavePriors : public MCRave
{
public:
    Node* Select(const Board& board, const std::vector<Node*>& children) const
    {
        // Prioritise the nodes if not already done.
        if (!children.empty() && !children[0]->Stats.Prioritised)
        {
            if (_useNN)
            {
                // Update the stats with the neural network output for this board state.
                assert(board.Size() == 19);
                NetworkValue(board, children);
            }

            for (Node* const c : children)
            {
                PriorUpdate(c->Stats);
            }
        }

        return MCRave::Select(board, children);
    }

private:
    // Define the priors that should be used.
    // The definitions consist of:
    // Number of visits added and number of wins in those visits.
    typedef std::pair<int, int> Prior;

    // These priors are tied to the move type.
    // If using a neural network for selection then ignore these.
    Prior CapturePrior = { 30, 30 };
    Prior SavePrior = { 20, 20 };
    Prior SelfAtariPrior = { 20, 0 };
    Prior LocalPrior = { 30, 30 };

    const int MaxNetPrior = 200;

    void PriorUpdate(MoveStats& stats) const
    {
        PriorUpdate(stats, Capture, CapturePrior);
        PriorUpdate(stats, Save, SavePrior);
        PriorUpdate(stats, SelfAtari, SelfAtariPrior);
        PriorUpdate(stats, Local, LocalPrior);

        if (_useNN)
        {
            NetPriorUpdate(stats);
        }

        stats.Prioritised = true;
    }

    void PriorUpdate(MoveStats& stats, MoveInfo moveType, const Prior& prior) const
    {
        if (stats.LastMove.Info & moveType)
        {
            // Apply the priors to the RAVE values so that they effect early selections.
            stats.RaveVisits += prior.first;
            stats.RaveWins += prior.second;
        }
    }

    void NetPriorUpdate(MoveStats& stats) const
    {
        int netPrior = stats.NetValue * MaxNetPrior;
        stats.RaveVisits += netPrior;
        stats.RaveWins += netPrior;
    }

    void NetworkValue(const Board& board, const std::vector<Node*>& children) const
    {
        double total;
        std::vector<double> values = NeuralNet::Select(board, total);
        for (auto& child : children)
        {
            MoveStats& stats = child->Stats;
            stats.NetValue = values[stats.LastMove.Coord];
        }
    }
};

#endif // __MC_RAVE_PRIORS_H__
