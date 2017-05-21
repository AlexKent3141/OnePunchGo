#ifndef __BOARD_H__
#define __BOARD_H__

#include "Move.h"
#include "Types.h"
#include "Zobrist.h"
#include <cassert>
#include <cstring>
#include <algorithm>
#include <vector>
#include <string>

// The data for a single point on the Go board.
struct Point
{
    Colour Col;
    int GroupSize;
    int Liberties;
    int Coord;
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

    // Get the latest hash.
    inline uint64_t CurrentHash() const { return _hashes[_turnNumber-1]; }

    // Check whether the game has finished (both players passed).
    inline bool GameOver() const { return _passes[0] && _passes[1]; }

    // Clone fields from other.
    void CloneFrom(const Board<N>& other)
    {
        _colourToMove = other._colourToMove;
        _turnNumber = other._turnNumber;
        memcpy(_hashes, other._hashes, 3*N*N*sizeof(uint64_t));
        memcpy(_passes, other._passes, 2*sizeof(bool));

        for (int i = 0; i < N*N; i++)
        {
            Point& pt = _points[i];
            const Point& opt = other._points[i];
            pt.Col = opt.Col;
            pt.GroupSize = opt.GroupSize;
            pt.Liberties = opt.Liberties;
        }
    }

    // Check the legality of the specified move in this position.
    Legality CheckLegal(int loc) const
    {
        return CheckLegal(this->_colourToMove, loc);
    }

    // Check the legality of the specified move in this position.
    Legality CheckLegal(Colour col, int loc) const
    {
        // Passing is always legal.
        if (loc == PassCoord)
            return Legal;

        // Check occupancy.
        const Point& pt = this->_points[loc];
        Legality res = pt.Col == None ? Legal : Occupied;
        if (res == Legal)
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
                        captureLoc = n->Coord;
                        capturesWithRepetition += n->GroupSize;
                    }
                }
            }

            bool suicide = liberties == 0;
            bool repetition = capturesWithRepetition == 1 && IsKoRepetition(col, loc, captureLoc);
            res = suicide ? Suicide : repetition ? Ko : Legal;
        }

        return res;
    }

    // Check whether the specified move will fill an eye.
    // This is currently more of a pseudo-eye check as it does not look at the diagonals.
    bool FillsEye(Colour col, int loc) const
    {
        int friendlyNeighbours = 0;
        const Point& pt = this->_points[loc];
        for (const Point* const n : pt.Neighbours)
        {
            friendlyNeighbours += n->Col == col && n->Liberties > 1 ? 1 : 0;
        }

        return friendlyNeighbours == pt.Neighbours.size();
    }

    // Get all moves available for the current colour.
    std::vector<Move> GetMoves(bool allowFillOwnEye = true) const
    {
        std::vector<Move> moves;
        if (!GameOver())
        {
            for (int i = 0; i < N*N; i++)
            {
                if (CheckLegal(i) == Legal)
                {
                    if (allowFillOwnEye || !FillsEye(this->_colourToMove, i))
                    {
                        moves.push_back({this->_colourToMove, i});
                    }
                }
            }

            // Passing is always legal.
            moves.push_back({this->_colourToMove, PassCoord});
        }

        return moves;
    }

    // Update the board state with the specified move.
    void MakeMove(const Move& move)
    {
        assert(!GameOver());
        assert(CheckLegal(move.Col, move.Coord) == Legal);

        auto z = Zobrist<N>::Instance();
        uint64_t nextHash = _hashes[_turnNumber-1];

        if (move.Coord != PassCoord)
        {
            nextHash ^= z->Key(move.Col, move.Coord);

            Point& pt = this->_points[move.Coord];
            pt.Col = move.Col;

            // Cache which points need updating.
            bool requireUpdate[N*N] = {false};
            requireUpdate[pt.Coord] = true;

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
                        requireUpdate[n->Coord] = true;
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
        }

        // Update the colour to move.
        if (move.Col == this->_colourToMove)
        {
            this->_colourToMove = this->_colourToMove == Black ? White : Black;
            nextHash ^= z->BlackTurn();
        }

        _hashes[_turnNumber++] = nextHash;
        _passes[(int)move.Col-1] = move.Coord == PassCoord;
    }

    // Compute the score of the current position.
    // Area scoring is used and we assume that all empty points are fully surrounded by one
    // colour.
    // The score is determined from black's perspective (positive score indicates black win).
    int Score() const
    {
        const double Komi = 7.5; // TODO: Make this configurable.
        double score = -Komi;
        for (int i = 0; i < N*N; i++)
        {
            const Point& pt = _points[i];
            if (pt.Col == None)
            {
                score += pt.Col == Black ? 1 : -1;
            }
            else
            {
                // Look at a neighbour.
                Point const* const n = pt.Neighbours[0];
                score += n->Col == Black ? 1 : -1;
            }
        }

        return score > 0 ? 1 : score < 0 ? -1 : 0;
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
    uint64_t _hashes[3*N*N] = {0};
    int _turnNumber = 1;
    bool _passes[2] = {false};

    // Initialise the neighbours for each point.
    void InitialiseNeighbours()
    {
        int r, c;
        for (int i = 0; i < N*N; i++)
        {
            r = i / N, c = i % N;
            _points[i].Coord = i;
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
        uint64_t nextHash =
            _hashes[_turnNumber-1]
            ^ z->Key(col, loc)
            ^ z->Key(enemyCol, captureLoc)
            ^ z->BlackTurn();

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
                requireUpdate[groupPt->Coord] = false;
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
        groupHash ^= Zobrist<N>::Instance()->Key(origCol, pt->Coord);

        for (Point* const n : pt->Neighbours)
        {
            if (n->Col == origCol)
            {
                FFCapture(n, requireUpdate, groupHash);
            }
            else if (n->Col != None)
            {
                requireUpdate[n->Coord] = true;
            }
        }
    }
};

#endif // __BOARD_H__
