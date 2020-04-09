#ifndef __BEST_OF_N_PLAYOUT_POLICY_H__
#define __BEST_OF_N_PLAYOUT_POLICY_H__

#include "PlayoutPolicy.h"
#include "../../RandomGenerator.h"
#include <cfloat>

// Best-of-N playout policy.
template<unsigned int N>
class BestOf : public PlayoutPolicy
{
public:
    // Randomly select N legal moves and decide which one looks more promising.
    Move Select(
        const Board& board,
        const Move& lastMove)
    {
        if (board.GameOver())
            return BadMove;

        auto moves = board.GetRandomLegalMoves(N, _gen);

        // Assess the randomly selected moves.
        double bestScore = -DBL_MAX;
        Move bestMove = BadMove;
        for (const Move& move : moves)
        {
            double score = MoveScore(move);
            if (score > bestScore)
            {
                bestScore = score;
                bestMove = move;
            }
        }

        return bestMove;
    }

protected:
    RandomGenerator _gen;

private:
    const double CaptureScore = 10;
    const double AtariScore = 5;
    const double SelfAtariScore = -8;
    const double SaveScore = 10;
    const double ConnectionScore = 1;
    const double PonnukiScore = 1;

    // Score the move according to how good/bad it looks.
    double MoveScore(const Move& move) const
    {
        double score = 0;
        const MoveInfo& info = move.Info;
        if (info & Capture) score += CaptureScore;
        if (info & Atari) score += AtariScore;
        if (info & SelfAtari) score += SelfAtariScore;
        if (info & Save) score += SaveScore;
        if (info & Connection) score += ConnectionScore;
        if (info & EyeShape) score += PonnukiScore;

        return score;
    }
};

#endif // __BEST_OF_N_PLAYOUT_POLICY_H__
