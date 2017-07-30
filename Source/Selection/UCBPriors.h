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

        double priorTerm = 0, visitWeight;
        if (stats.Visits < PriorUsed)
        {
            visitWeight = (PriorUsed - stats.Visits) / PriorUsed;
            priorTerm = PriorWeight * visitWeight * Prior(stats.LastMove);
        }

        double raveTerm = 0;
        if (stats.Visits < RaveUsed && stats.RaveVisits > 0)
        {
            visitWeight = (RaveUsed - stats.Visits) / RaveUsed;
            raveTerm = (RaveWeight * visitWeight * stats.RaveWins) / stats.RaveVisits;
        }

        return priorTerm + raveTerm + UCB<100>::Policy(n);
    }

private:
    double RaveUsed = 45.05423048454979;
    double PriorUsed = 61.45724637713563;

    double RaveWeight = 2.0989194045089272;
    double PriorWeight = 2.2810335298065847;

    const double CaptureScore = 10;
    const double AtariScore = 5;
    const double SelfAtariScore = -10;
    const double Pat3MatchScore = 1;
    const double Pat5MatchScore = 2;

    // Calculate a prior score for the move in order to bias the search.
    double Prior(const Move& move) const
    {
        double score = 0;
        const MoveInfo& info = move.Info;
        if (info & Capture) score += CaptureScore;
        if (info & Atari) score += AtariScore;
        if (info & SelfAtari) score += SelfAtariScore;
        if (info & Pat3Match) score += Pat3MatchScore;
        if (info & Pat5Match) score += Pat5MatchScore;

        return score;
    }
};

#endif // __UCBPRIORS_SELECTION_POLICY_H__
