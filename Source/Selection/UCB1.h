#ifndef __UCB1_SELECTION_POLICY_H__
#define __UCB1_SELECTION_POLICY_H__

#include "SelectionPolicy.h"
#include <cfloat>
#include <iostream>

class UCB1 : SelectionPolicy
{
public:
    // Apply the UCB1 formula.
    Node* Select(const std::vector<Node*>& children) const
    {
        int totalVisits = children[0]->Parent->Stats.Visits;
        Node* bestChild = nullptr;
        double bestVal = -DBL_MAX;
        double wr, ex, val;
        for (Node* const c : children)
        {
            const MoveStats& stats = c->Stats;
            wr = (double)stats.Wins / stats.Visits;
            ex = sqrt(log(totalVisits) / stats.Visits);
            val = wr + ex;

            if (val > bestVal)
            {
                bestVal = val;
                bestChild = c;
            }
        }

        return bestChild;
    }
};

#endif // __UCB1_SELECTION_POLICY_H__
