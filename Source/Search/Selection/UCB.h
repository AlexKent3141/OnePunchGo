#ifndef __UCB_SELECTION_POLICY_H__
#define __UCB_SELECTION_POLICY_H__

#include "SelectionPolicy.h"
#include <cfloat>
#include <cmath>
#include <iostream>

// UCB selection policy.
// The template argument is the exploration constant multipled by 100.
template <unsigned int N>
class UCB : SelectionPolicy
{
public:
    // Select the most promising child according to the UCB algorithm.
    std::shared_ptr<Node> Select(
        const std::vector<std::shared_ptr<Node>>& children) const
    {
        auto score = std::bind(&UCB::Policy, *this, std::placeholders::_1);
        return ArgMax<Node>(children, score);
    }

protected:
    // This method applies the UCB formula.
    virtual double Policy(
        std::shared_ptr<Node> const n) const
    {
        int totalVisits = n->Parent->Stats.Visits;
        return totalVisits > 0
            ? MCVal(n) + ExplorationTerm(n->Stats.Visits, totalVisits)
            : 0;
    }

    double MCVal(
        std::shared_ptr<Node> const n) const
    {
        const MoveStats& stats = n->Stats;
        return stats.Visits > 0 ? (double)stats.Wins / stats.Visits : 0;
    }

    // Calculate the exploration term.
    double ExplorationTerm(int visits, int parentVisits) const
    {
        return  sqrt(N*log(parentVisits) / (100.0*visits));
    }
};

#endif // __UCB_SELECTION_POLICY_H__
