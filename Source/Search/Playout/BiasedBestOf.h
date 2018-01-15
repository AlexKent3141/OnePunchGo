#ifndef __BIASED_BEST_OF_N_PLAYOUT_POLICY_H__
#define __BIASED_BEST_OF_N_PLAYOUT_POLICY_H__

#include "PlayoutPolicy.h"
#include "../../RandomGenerator.h"
#include "../../Patterns/PatternMatcher.h"
#include <cfloat>
#include <iostream>

// Best-of-N playout policy.
template<unsigned int N>
class BiasedBestOf : public PlayoutPolicy
{
public:
    // Randomly select N legal moves and decide which one looks more promising.
    Move Select(const Board& board, const Move& lastMove)
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

        if (_gen.NextDouble() < 0.55 && lastMove.Coord != PassCoord)
        {
            selection = GetLocals(board, moves, lastMove.Coord);
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
    std::vector<Move> GetMatches(const std::vector<Move>& moves, int moveType) const
    {
        std::vector<Move> movesOfType;
        for (const Move& m : moves)
        {
            bool rightType = m.Info & moveType;
            if (rightType)
            {
                movesOfType.push_back(m);
            }
        }

        return movesOfType;
    }

    // Get the local moves which match a 3x3 pattern.
    std::vector<Move> GetLocals(const Board& board, const std::vector<Move>& moves, int lastCoord) const
    {
        std::vector<Move> locals;
        PatternMatcher matcher;

        int boardSize = board.Size();
        int lx = lastCoord % boardSize;
        int ly = lastCoord / boardSize;
        for (const Move& m : moves)
        {
            int c = m.Coord;
            bool local = (c == lastCoord + 1 && lx < boardSize - 1)
                      || (c == lastCoord - 1 && lx > 0)
                      || (c == lastCoord - boardSize && ly > 0)
                      || (c == lastCoord + boardSize && ly < boardSize - 1);
            
            bool urgent = m.Info & (Atari | Capture);
            bool match = matcher.HasMatch(board, 3, c);
            if (local && (urgent || match))
            {
                locals.push_back(m);
            }
        }

        return locals;
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

        return bestMove;
    }
};

#endif // __BIASED_BEST_OF_N_PLAYOUT_POLICY_H__
