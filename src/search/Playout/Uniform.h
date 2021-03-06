#ifndef __UNIFORM_PLAYOUT_POLICY_H__
#define __UNIFORM_PLAYOUT_POLICY_H__

#include "PlayoutPolicy.h"
#include "core/RandomGenerator.h"

class Uniform : public PlayoutPolicy
{
public:
    // Choose a move using uniform random distribution.
    Move Select(const Board& board, const Move& lastMove)
    {
        if (board.GameOver())
            return BadMove;

        // Try and pick a legal move by guessing to save generating all legal moves.
        const int NumTries = 4;
        int boardArea = board.Size() * board.Size();
        Colour col = board.ColourToMove();
        for (int i = 0; i < NumTries; i++)
        {
            int coord = _gen.Next(boardArea);
            MoveInfo info = board.CheckMove(coord);
            if ((info & Legal) && !(info & FillsEye))
                return { col, coord };
        }

        auto moves = board.GetMoves(true);
        return moves[_gen.Next(moves.size())];
    }

private:
    RandomGenerator _gen;
};

#endif // __UNIFORM_PLAYOUT_POLICY_H__
