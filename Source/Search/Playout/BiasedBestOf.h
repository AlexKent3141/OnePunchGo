#ifndef __BIASED_BEST_OF_N_PLAYOUT_POLICY_H__
#define __BIASED_BEST_OF_N_PLAYOUT_POLICY_H__

#include "PlayoutPolicy.h"
#include "BestOf.h"
#include "../../RandomGenerator.h"
#include <cfloat>
#include <iostream>

// Best-of-N playout policy with biases.
template<unsigned int N>
class BiasedBestOf : protected BestOf<N>
{
public:
    // Randomly select N legal moves and decide which one looks more promising.
    Move Select(
        const Board& board,
        const Move& lastMove)
    {
        if (board.GameOver())
            return BadMove;

        Move bestMove = BadMove;

        // Global capturing move.
        if (BestOf<N>::_gen.NextDouble() < 0.45)
        {
            bestMove = board.GetRandomMoveAttackingLiberties(1, BestOf<N>::_gen);
            if (bestMove != BadMove)
            {
                return bestMove;
            }
        }

        // Global saving move.
        if (BestOf<N>::_gen.NextDouble() < 0.55)
        {
            bestMove = board.GetRandomMoveSaving(BestOf<N>::_gen);
            if (bestMove != BadMove)
            {
                return bestMove;
            }
        }

        // Local urgent move.
        if (BestOf<N>::_gen.NextDouble() < 0.55 && lastMove.Coord != PassCoord)
        {
            const MoveInfo Urgent = Capture | Atari;
            bestMove = board.GetRandomMoveLocal(lastMove.Coord, Urgent, BestOf<N>::_gen);
            if (bestMove != BadMove)
            {
                return bestMove;
            }
        }

        // If none of the above then default to the base policy.
        return BestOf<N>::Select(board, lastMove);
    }
};

#endif // __BIASED_BEST_OF_N_PLAYOUT_POLICY_H__
