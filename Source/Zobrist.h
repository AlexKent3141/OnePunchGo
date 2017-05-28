#ifndef __ZOBRIST_H__
#define __ZOBRIST_H__

#include "Globals.h"
#include "RandomGenerator.h"
#include "Types.h"
#include <cassert>

// Zobrist keys for stones of each colour in each location.
class Zobrist
{
public:
    static Zobrist* Instance() 
    {
        if (_instance == nullptr)
        {
            _instance = new Zobrist();
        }

        return _instance;
    }

    inline uint64_t BlackTurn() const { return _blackTurn; }

    inline uint64_t Key(Colour col, int loc) const 
    {
        assert(col != None);
        assert(loc < MaxBoardArea);
        return _keys[(int)col-1][loc]; 
    }

private:
    static Zobrist* _instance;

    // This key is present if it's black's turn.
    uint64_t _blackTurn;

    // Need a hash key for each colour in each location.
    uint64_t _keys[2][MaxBoardArea];

    Zobrist()
    {
        // Note: This is the seed that Stockfish uses.
        RandomGenerator gen(1070372);
        _blackTurn = gen.Next();
        for (int col = 0; col < 2; col++)
        {
            for (int loc = 0; loc < MaxBoardArea; loc++)
            {
                _keys[col][loc] = gen.Next();
            }
        }
    }
};

#endif // __ZOBRIST_H__
