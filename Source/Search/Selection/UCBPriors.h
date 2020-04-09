#ifndef __UCBPRIORS_SELECTION_POLICY_H__
#define __UCBPRIORS_SELECTION_POLICY_H__

#include "SelectionPolicy.h"
#include "UCB1.h"
#include <iostream>

class UCBPriors : public UCB1
{
protected:
    virtual double Policy(
        std::shared_ptr<Node> const n) const
    {
        const MoveStats& stats = n->Stats;

        double priorTerm = 0;
        if (stats.Visits > 0)
            priorTerm = Prior(stats.LastMove) / stats.Visits;

        return priorTerm + UCB1::Policy(n);
    }

private:
    const double CaptureScore = 10;

    // Calculate a prior score for the move in order to bias the search.
    double Prior(const Move& move) const
    {
        double score = 0;
        const MoveInfo& info = move.Info;
        if (info & Capture) score += CaptureScore;

        return score;
    }
};

#endif // __UCBPRIORS_SELECTION_POLICY_H__
