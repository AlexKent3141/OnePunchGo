#ifndef __UNIFORM_PLAYOUT_POLICY_H__
#define __UNIFORM_PLAYOUT_POLICY_H__

#include "PlayoutPolicy.h"
#include "../RandomGenerator.h"

class Uniform : public PlayoutPolicy
{
public:
    // Choose a move using uniform random distribution.
    Move Select(const std::vector<Move>& moves)
    {
        return moves[_gen.Next() % moves.size()];
    }

private:
    RandomGenerator _gen;
};

#endif // __UNIFORM_PLAYOUT_POLICY_H__
