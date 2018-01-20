#ifndef __BOARD_H__
#define __BOARD_H__

#include "BitSet.h"
#include "Globals.h"
#include "Move.h"
#include "MoveHistory.h"
#include "Rules.h"
#include "Types.h"
#include "Zobrist.h"
#include <cassert>
#include <cstring>
#include <algorithm>
#include <list>
#include <vector>
#include <string>

// A chain of stones.
struct StoneChain 
{
    BitSet* Stones;
    BitSet* Neighbours;
    uint64_t Hash;
};

// Represents a single point on the goban.
struct Point
{
    Colour Col;
    int Coord;
    std::vector<Point*> Neighbours;
    BitSet* Orthogonals;
    BitSet* Diagonals;
    int ChainId;
};

// The board object which can be incrementally updated.
class Board
{
public:
    Board() = delete;
    Board(int);
    Board(Colour, const std::vector<std::string>&);
    Board(Colour, int, const MoveHistory&);

    ~Board();

    inline int Size() const { return _boardSize; }

    inline Colour ColourToMove() const { return _colourToMove; }

    // Get the colour at the specified location.
    inline Colour PointColour(int loc) const
    {
        return _points[loc].Col;
    }

    // Get the latest hash.
    inline uint64_t CurrentHash() const { return _hashes[_turnNumber-1]; }

    // Check whether the game has finished (both players passed).
    inline bool GameOver() const { return _passes[0] && _passes[1]; }

    // Clone fields from other.
    void CloneFrom(const Board&);

    // Roughly check whether this point can possible be an eye.
    bool IsEye(Colour, int) const;

    // Check the legality of the specified move in this position.
    MoveInfo CheckMove(int, bool duringPlayout = false) const;

    // Check the legality of the specified move in this position.
    MoveInfo CheckMove(Colour, int, bool duringPlayout = false) const;

    // Get all moves available for the current colour.
    std::vector<Move> GetMoves(bool duringPlayout = false) const;

    // Update the board state with the specified move.
    void MakeMove(const Move&);

    // Compute the score of the current position.
    // Area scoring is used and we assume that all empty points are fully surrounded by one
    // colour.
    // The score is determined from black's perspective (positive score indicates black win).
    int Score() const;

    std::string ToString() const;

private:
    const int NoChain = -1;

    Colour _colourToMove = Black;
    Point* _points = nullptr;
    std::vector<uint64_t> _hashes;
    int _turnNumber = 1;
    int _boardSize;
    int _boardArea;
    bool _passes[2] = {false};

    BitSet* _empty = nullptr;
    BitSet* _blackStones = nullptr;
    BitSet* _whiteStones = nullptr;
    std::vector<StoneChain> _chains;

    // Initialise an empty board of the specified size.
    void InitialiseEmpty(int);

    // Initialise the neighbours for each point.
    void InitialiseNeighbours();

    // Check whether the specified move and capture would result in a board repetition.
    bool IsKoRepetition(Colour, int, int) const;

    // Count the liberties of the given chain.
    int CountChainLiberties(int) const;

    int CountChainSize(int) const;

    // Capture the chain with the specified ID.
    // Returns the hash of the chain.
    uint64_t CaptureChain(int);

    void CreateNewChainForMove(const Move&, uint64_t);

    // Merge the specified chains.
    void CombineChainsForMove(const Move&, uint64_t, const std::vector<int>&);
};

#endif // __BOARD_H__
