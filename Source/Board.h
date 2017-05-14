#ifndef __BOARD_H__
#define __BOARD_H__

#include "Move.h"
#include "Types.h"
#include <cassert>
#include <vector>

// The data for a single point on the Go board.
struct Point
{
    Colour Col;
    int Liberties;
    int Loc;
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
        this->InitialiseNeighbours();
    }

    // Check the legality of the specified move in this position.
    bool IsLegal(Colour col, int point) const
    {
        // Check occupancy.
        const Point& pt = this->_points[point];
        bool legal = pt.Col == None;
        if (legal)
        {
            // Check for suicide.
            int liberties = 0;
            for (const Point* const n : pt.Neighbours)
            {
                if (n->Col == None)
                {
                    ++liberties;
                }
                else if (n->Col == col)
                {
                    liberties += n->Liberties-1;
                }
                else
                {
                    if (n->Liberties == 1)
                        ++liberties;
                }
            }

            legal = liberties > 0;

            // TODO: Check for board state repetition.
        }

        return legal;
    }

    // Get all moves available for the current colour.
    std::vector<Move> GetMoves() const
    {
        std::vector<Move> moves;
        for (int i = 0; i < N*N; i++)
        {
            if (IsLegal(this->_colourToMove, i))
            {
                moves.push_back({this->_colourToMove, i});
            }
        }

        return moves;
    }

    // Update the board state with the specified move.
    void MakeMove(const Move& move)
    {
        assert(IsLegal(move.Col, move.Point));
        Point& pt = this->_points[move.Point];
        pt.Col = move.Col;

        // Cache which points need updating.
        bool requireUpdate[N*N] = {false};
        requireUpdate[pt.Loc] = true;

        // First detect stones which will get captured and remove them.
        for (Point* const n : pt.Neighbours)
        {
            if (n->Col != None)
            {
                if (n->Col != pt.Col && n->Liberties == 1)
                {
                    FFCapture(n, requireUpdate);
                }
                else
                {
                    requireUpdate[n->Loc] = true;
                }
            }
        }

        // Update the liberties of the affected stones.
        for (int i = 0; i < N*N; i++)
        {
            if (requireUpdate(i))
            {
                FFLiberties(&_points[i]);
            }
        }
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
            _points[i].Loc = i;
            _points[i].Neighbours.clear();
            if (r > 0) _points[i].Neighbours.push_back(&_points[i-N]);
            if (r < N-1) _points[i].Neighbours.push_back(&_points[i+N]);
            if (c > 0) _points[i].Neighbours.push_back(&_points[i-1]);
            if (c < N-1) _points[i].Neighbours.push_back(&_points[i+1]);
        }
    }

    // Flood fill algorithm which updates the liberties for all stones in the group containing
    // the specified point.
    void FFLiberties(Point* const pt, bool* requireUpdate, int liberties = 0, std::vector<Point*> group = std::vector<Point*>(), bool root = true)
    {
        Colour origCol = pt->Col;
        for (Point* const n : pt->Neighbours)
        {
            if (n->Col == origCol)
            {
                group.push_back(n);
                FFLiberties(n, requireUpdate, liberties, group, false);
            }
            else if (n->Col == None)
            {
                ++liberties;
            }
        }

        if (root)
        {
            for (Point* const groupPt : group)
            {
                groupPt->Liberties = liberties;
                requireUpdate[groupPt->Loc] = false;
            }
        }
    }

    // Flood fill algorithm which removes all stones in the group containing the specified point.
    // Points which are affected by this capture get flagged as requiring an update.
    void FFCapture(Point* const pt, bool* requireUpdate)
    {
        Colour origCol = pt->Col;
        pt->Col = None;
        pt->Liberties = 0;

        for (Point* const n : pt->Neighbours)
        {
            if (n->Col == origCol)
            {
                FFCapture(n, requireUpdate);
            }
            else if (n->Col != None)
            {
                requireUpdate[n->Loc] = true;
            }
        }
    }
};

#endif // __BOARD_H__
