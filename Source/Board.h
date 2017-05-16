#ifndef __BOARD_H__
#define __BOARD_H__

#include "Move.h"
#include "Types.h"
#include "Zobrist.h"
#include <cassert>
#include <algorithm>
#include <vector>
#include <string>

// The data for a single point on the Go board.
struct Point
{
    Colour Col;
    int GroupSize;
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
        _hashes[0] = Zobrist<N>::Instance()->BlackTurn();
    }

    // Get the colour at the specified location.
    inline Colour PointColour(int loc) const { return _points[loc].Col; }

    // Check the legality of the specified move in this position.
    bool IsLegal(Colour col, int loc) const
    {
        // Check occupancy.
        const Point& pt = this->_points[loc];
        bool legal = pt.Col == None;
        if (legal)
        {
            // Check for suicide and ko.
            int liberties = 0;
            int captureLoc;
            int capturesWithRepetition = 0; // Note: captured neighbours could be in the same group!
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
                    {
                        ++liberties;
                        captureLoc = n->Loc;
                        capturesWithRepetition += n->GroupSize;
                    }
                }
            }

            bool suicide = liberties == 0;
            bool repetition = capturesWithRepetition == 1 && IsKoRepetition(col, loc, captureLoc);
            legal = !suicide && !repetition;
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

        auto z = Zobrist<N>::Instance();
        uint64_t nextHash = _hashes[_turnNumber-1];
        nextHash ^= z->Key(move.Col, move.Point);

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
                if (n->Col != move.Col && n->Liberties == 1)
                {
                    uint64_t groupHash = 0;
                    FFCapture(n, requireUpdate, groupHash);
                    nextHash ^= groupHash;
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
            if (requireUpdate[i])
            {
                int liberties = 0;
                std::vector<Point*> group;
                std::vector<Point*> considered;
                FFLiberties(&_points[i], requireUpdate, liberties, group, considered);
            }
        }

        // Update the colour to move.
        if (move.Col == this->_colourToMove)
        {
            this->_colourToMove = this->_colourToMove == Black ? White : Black;
            nextHash ^= z->BlackTurn();
        }

        _hashes[_turnNumber++] = nextHash;
    }

    std::string ToString() const
    {
        std::string s;
        Colour col;
        for (int r = N-1; r >= 0; r--)
        {
            for (int c = 0; c < N; c++)
            {
                col = _points[r*N+c].Col;
                s += col == None ? '.' : col == Black ? 'B' : 'W';
            }

            s += '\n';
        }

        return s;
    }

private:
    Colour _colourToMove = Black;
    Point _points[N*N] = {{None, 0, 0}};
    uint64_t _hashes[2*N*N] = {0};
    int _turnNumber = 1;

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

    // Check whether the specified move and capture would result in a board repetition.
    bool IsKoRepetition(Colour col, int loc, int captureLoc) const
    {
        Colour enemyCol = col == Black ? White : Black;
        auto z = Zobrist<N>::Instance();
        uint64_t nextHash = _hashes[_turnNumber-1] ^ z->Key(col, loc) ^ z->Key(enemyCol, captureLoc);

        // Has this hash occurred previously?
        bool repeat = false;
        for (int i = _turnNumber-1; i >= 0 && !repeat; i--)
            repeat = _hashes[i] == nextHash;

        return repeat;
    }

    // Flood fill algorithm which updates the liberties for all stones in the group containing
    // the specified point.
    void FFLiberties(Point* const pt, bool* requireUpdate, int& liberties, std::vector<Point*>& group, std::vector<Point*>& considered, bool root = true)
    {
        group.push_back(pt);
        for (Point* const n : pt->Neighbours)
        {
            if (n->Col == pt->Col &&
                std::find(group.begin(), group.end(), n) == group.end())
            {
                FFLiberties(n, requireUpdate, liberties, group, considered, false);
            }
            else if (n->Col == None &&
                std::find(considered.begin(), considered.end(), n) == considered.end())
            {
                ++liberties;
                considered.push_back(n);
            }
        }

        if (root)
        {
            for (Point* const groupPt : group)
            {
                groupPt->Liberties = liberties;
                groupPt->GroupSize = group.size();
                requireUpdate[groupPt->Loc] = false;
            }
        }
    }

    // Flood fill algorithm which removes all stones in the group containing the specified point.
    // Points which are affected by this capture get flagged as requiring an update.
    void FFCapture(Point* const pt, bool* requireUpdate, uint64_t& groupHash)
    {
        Colour origCol = pt->Col;
        pt->Col = None;
        pt->Liberties = 0;
        groupHash ^= Zobrist<N>::Instance()->Key(origCol, pt->Loc);

        for (Point* const n : pt->Neighbours)
        {
            if (n->Col == origCol)
            {
                FFCapture(n, requireUpdate, groupHash);
            }
            else if (n->Col != None)
            {
                requireUpdate[n->Loc] = true;
            }
        }
    }
};

#endif // __BOARD_H__
