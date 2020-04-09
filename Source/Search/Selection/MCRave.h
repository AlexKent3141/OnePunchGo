#ifndef __MC_RAVE_H__
#define __MC_RAVE_H__

#include <cmath>

class MCRave : SelectionPolicy
{
public:
    std::shared_ptr<Node> Select(
        const std::vector<std::shared_ptr<Node>>& children) const
    {
        auto score = std::bind(&MCRave::NodeScore, *this, std::placeholders::_1);
        return ArgMax<Node>(children, score);
    }

private:
    // This constant defines the cross-over point where the weight of the
    // MC values and the RAVE values is equal.
    const int K = 1000;

    double MCVal(
        std::shared_ptr<Node> const n) const
    {
        const MoveStats& stats = n->Stats;
        return stats.Visits > 0 ? (double)stats.Wins / stats.Visits : 0;
    }

    double RaveVal(
        std::shared_ptr<Node> const n) const
    {
        const MoveStats& stats = n->Stats;
        return stats.RaveVisits > 0 ? (double)stats.RaveWins / stats.RaveVisits : 0;
    }

    double Beta(
        std::shared_ptr<Node> const n) const
    {
        double b = 1;
        const MoveStats& stats = n->Stats;
        if (stats.Visits > 0)
        {
            b = sqrt((double)K / (3*stats.Visits + K));
        }

        return b;
    }

    double NodeScore(
        std::shared_ptr<Node> const n) const
    {
        double b = Beta(n);
        return (1-b)*MCVal(n) + b*RaveVal(n);
    }
};

#endif // __MC_RAVE_H__
