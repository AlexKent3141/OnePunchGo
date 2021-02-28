#ifndef __MC_RAVE_PRIORS_H__
#define __MC_RAVE_PRIORS_H__

#include "MCRave.h"
#include <utility>
#include <cassert>

// The MCRave selection policy with priors associated to some types of moves.
class MCRavePriors : public MCRave
{
public:
    Node* Select(const std::vector<Node*>& children) const
    {
        for (Node* const c : children)
        {
            PriorUpdateAll(c->Stats);
        }

        return MCRave::Select(children);
    }

private:
    // Define the priors that should be used.
    // The definitions consist of:
    // Number of visits added and number of wins in those visits.
    typedef std::pair<int, int> Prior;

    Prior CapturePrior = { 30, 30 };
    Prior SavePrior = { 20, 20 };
    Prior SelfAtariPrior = { 20, 0 };
    Prior LocalPrior = { 30, 30 };

    void PriorUpdateAll(MoveStats& stats) const
    {
        if (!stats.Prioritised)
        {
            PriorUpdate(stats, Capture, CapturePrior);
            PriorUpdate(stats, Save, SavePrior);
            PriorUpdate(stats, SelfAtari, SelfAtariPrior);
            PriorUpdate(stats, Local, LocalPrior);
            stats.Prioritised = true;
        }
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
};

#endif // __MC_RAVE_PRIORS_H__
