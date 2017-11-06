#ifndef __UCBRAVE_SELECTION_POLICY_H__
#define __UCBRAVE_SELECTION_POLICY_H__

#include "SelectionPolicy.h"
#include "UCB1.h"
#include <iostream>

class UCBRave : public UCB1
{
protected:
    virtual double Policy(Node* const n) const
    {
        const MoveStats& stats = n->Stats;

        double raveTerm = 0;
        if (stats.RaveVisits > 0)
        {
            raveTerm = (double)stats.RaveWins / stats.RaveVisits;
        }

        return RaveWeight * raveTerm + (1 - RaveWeight) * UCB1::Policy(n);
    }

private:
    const double RaveWeight = 0.1;
};

#endif // __UCBRAVE_SELECTION_POLICY_H__
