#ifndef __BEST_OF_N_PLAYOUT_POLICY_H__
#define __BEST_OF_N_PLAYOUT_POLICY_H__

#include "PlayoutPolicy.h"
#include "../RandomGenerator.h"
#include <cfloat>

// Best-of-N playout policy.
template<unsigned int N>
class BestOf : public PlayoutPolicy
{
public:
    // Randomly select N legal moves and decide which one looks more promising.
    Move Select(const Board& board)
    {
        if (board.GameOver())
            return BadMove;

        auto moves = board.GetMoves(true);

        double bestScore = -DBL_MAX;
        Move bestMove = BadMove;
        for (size_t i = 0; i < N; i++)
        {
            // Choose a move randomly.
            const Move& move = moves[_gen.Next(moves.size())];
            double score = MoveScore(move);
            if (score > bestScore)
            {
                bestScore = score;
                bestMove = move;
            }
        }

        return bestMove;
    }

private:
    RandomGenerator _gen;

    const double CaptureScore = 10;
    const double AtariScore = 5;
    const double SelfAtariScore = -8;
    const double SaveScore = 10;

    // Score the move according to how good/bad it looks.
    double MoveScore(const Move& move) const
    {
        double score = 0;
        const MoveInfo& info = move.Info;
        if (info & Capture) score += CaptureScore;
        if (info & Atari) score += AtariScore;
        if (info & SelfAtari) score += SelfAtariScore;
        if (info & Save) score += SaveScore;

        return score;
    }
};

#endif // __BEST_OF_N_PLAYOUT_POLICY_H__
