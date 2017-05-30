#ifndef __UCBPRIORS_SELECTION_POLICY_H__
#define __UCBPRIORS_SELECTION_POLICY_H__

#include "SelectionPolicy.h"
#include "UCB1.h"
#include <iostream>

class UCBPriors : public UCB<100>
{
protected:
    virtual double Policy(Node* const n) const
    {
        const MoveStats& stats = n->Stats;
        double priorTerm = Prior(stats.LastMove) / stats.Visits;
        return priorTerm + UCB<100>::Policy(n);
    }

private:
    const double CaptureScore = 10;
    const double AtariScore = 5;
    const double SelfAtariScore = -10;

    // Calculate a prior score for the move in order to bias the search.
    double Prior(const Move& move) const
    {
        double score = 0;
        const MoveInfo& info = move.Info;
        if (info & Capture) score += CaptureScore;
        if (info & Atari) score += AtariScore;
        if (info & SelfAtari) score += SelfAtariScore;

        return score;
    }
};

#endif // __UCBPRIORS_SELECTION_POLICY_H__
