#ifndef __MC_RAVE_PRIORS_H__
#define __MC_RAVE_PRIORS_H__

#include "MCRave.h"
#include <utility>
#include <cassert>

// The MCRave selection policy with priors associated to some types of moves.
class MCRavePriors : public MCRave
{
public:
    Node* Select(const Board& board, const std::vector<Node*>& children, NeuralNet* net) const
    {
        // Prioritise the nodes if not already done.
        if (!children.empty() && !children[0]->Stats.Prioritised)
        {
            bool useNN = net != nullptr;
            if (useNN && children[0]->Stats.Depth < 3)
            {
                // Update the stats with the neural network output for this board state.
                assert(board.Size() == 19);
                NetworkValue(board, children, net);
            }

            for (Node* const c : children)
            {
                PriorUpdate(c->Stats, useNN);
            }
        }

        return MCRave::Select(board, children, net);
    }

private:
    // Define the priors that should be used.
    // The definitions consist of:
    // Number of visits added and number of wins in those visits.
    typedef std::pair<int, int> Prior;

    // These priors are tied to the move type.
    Prior CapturePrior = { 30, 30 };
    Prior SavePrior = { 20, 20 };
    Prior SelfAtariPrior = { 20, 0 };
    Prior LocalPrior = { 30, 30 };

    // The maximum size of the neural network prior.
    const int MaxNetPrior = 1000;

    void PriorUpdate(MoveStats& stats, bool useNN) const
    {
        PriorUpdate(stats, Capture, CapturePrior);
        PriorUpdate(stats, Save, SavePrior);
        PriorUpdate(stats, SelfAtari, SelfAtariPrior);
        PriorUpdate(stats, Local, LocalPrior);

        if (useNN)
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

    void NetworkValue(const Board& board, const std::vector<Node*>& children, NeuralNet* net) const
    {
        double total;
        std::vector<double> values = net->Evaluate(board, total);
        for (auto& child : children)
        {
            MoveStats& stats = child->Stats;
            if (stats.LastMove.Coord >= 0) stats.NetValue = values[stats.LastMove.Coord];
        }
    }
};

#endif // __MC_RAVE_PRIORS_H__
