#ifndef __UCB_SELECTION_POLICY_H__
#define __UCB_SELECTION_POLICY_H__

#include "SelectionPolicy.h"
#include <cfloat>
#include <iostream>

// UCB selection policy.
// The template argument is the exploration constant multipled by 100.
template <unsigned int N>
class UCB : SelectionPolicy
{
public:
    // Select the most promising child according to the UCB algorithm.
    Node* Select(const std::vector<Node*>& children) const
    {
        Node* bestChild = nullptr;
        double bestVal = -DBL_MAX;
        double val;
        for (Node* const c : children)
        {
            val = Policy(c);

            if (val > bestVal)
            {
                bestVal = val;
                bestChild = c;
            }
        }

        return bestChild;
    }

protected:
    // This method applies the UCB formula.
    virtual double Policy(Node* const n) const
    {
        const MoveStats& stats = n->Stats;
        int totalVisits = n->Parent->Stats.Visits;
        double wr = (double)stats.Wins / stats.Visits;
        return wr + ExplorationTerm(stats.Visits, totalVisits);
    }

    // Calculate the exploration term.
    double ExplorationTerm(int visits, int parentVisits) const
    {
        return sqrt(N*log(parentVisits) / (100.0*visits));
    }
};

#endif // __UCB_SELECTION_POLICY_H__
