#ifndef __BOARD_H__
#define __BOARD_H__

#include "Move.h"
#include "Types.h"
#include <vector>
#include <iostream>

// The data for a single point on the Go board.
struct Point
{
    Colour Col;
    int Liberties;
    std::vector<Point*> Neighbours;
};

// The board object which can be incrementally updated.
template<unsigned int N>
class Board
{
static_assert(N < 26, "Board is too large to represent");
public:
    Board()
    {
        InitialiseNeighbours();
    }

    // Check the legality of the specified move in this position.
    bool IsLegal(Colour col, int point) const
    {
        return true;
    }

    // Update the board state with the specified move.
    void MakeMove(const Move& move)
    {
    }

    // Get all moves available for the current colour.
    std::vector<Move> GetMoves() const
    {
        std::vector<Move> moves;
        for (int i = 0; i < N*N; i++)
        {
            if (IsLegal(_colourToMove, i))
            {
                moves.push_back({_colourToMove, i});
            }
        }

        return moves;
    }

private:
    Colour _colourToMove = Black;
    Point _points[N*N] = {{None, 0}};

    // Initialise the neighbours for each point.
    void InitialiseNeighbours()
    {
        int r, c;
        for (int i = 0; i < N*N; i++)
        {
            r = i / N, c = i % N;
            _points[i].Neighbours.clear();
            if (r > 0) _points[i].Neighbours.push_back(&_points[i-N]);
            if (r < N-1) _points[i].Neighbours.push_back(&_points[i+N]);
            if (c > 0) _points[i].Neighbours.push_back(&_points[i-1]);
            if (c < N-1) _points[i].Neighbours.push_back(&_points[i+1]);
        }
    }
};

#endif // __BOARD_H__
