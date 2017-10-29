#ifndef __BIASED_BEST_OF_N_PLAYOUT_POLICY_H__
#define __BIASED_BEST_OF_N_PLAYOUT_POLICY_H__

#include "PlayoutPolicy.h"
#include "../RandomGenerator.h"
#include <cfloat>

// Best-of-N playout policy.
template<unsigned int N>
class BiasedBestOf : public PlayoutPolicy
{
public:
    // Randomly select N legal moves and decide which one looks more promising.
    Move Select(const Board& board)
    {
        if (board.GameOver())
            return BadMove;

        auto moves = board.GetMoves(true);

        Move bestMove = BadMove;
        std::vector<Move> selection;
        if (_gen.NextDouble() < 0.45)
        {
            selection = GetMatches(moves, Capture);
            if (!selection.empty())
            {
                return selection[_gen.Next(selection.size())];
            }
        }

        if (_gen.NextDouble() < 0.55)
        {
            selection = GetMatches(moves, Save);
            if (!selection.empty())
            {
                return selection[_gen.Next(selection.size())];
            }
        }

        bestMove = BestOf(board, moves);

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

    // Get the moves within the list that are of a certain type.
    std::vector<Move> GetMatches(const std::vector<Move>& moves, int moveType)
    {
        std::vector<Move> movesOfType;
        for (const Move& m : moves)
        {
            bool selfAtari = m.Info & SelfAtari;
            bool rightType = m.Info & moveType;
            if (!selfAtari && rightType)
            {
                movesOfType.push_back(m);
            }
        }

        return movesOfType;
    }

    Move BestOf(const Board& board, const std::vector<Move>& moves)
    {
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

        // If only bad moves are available then return a pass.
        if (bestScore < 0)
        {
            bestMove = { board.ColourToMove(), PassCoord };
        }

        return bestMove;
    }
};

#endif // __BIASED_BEST_OF_N_PLAYOUT_POLICY_H__
