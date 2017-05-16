#ifndef __ZOBRIST_H__
#define __ZOBRIST_H__

#include "RandomGenerator.h"
#include "Types.h"
#include <cassert>

// Zobrist keys for each piece type in each position and orientation.
template<unsigned int N>
class Zobrist
{
public:
    static Zobrist<N>* Instance() 
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
        assert(loc < N*N);
        return _keys[(int)col-1][loc]; 
    }

private:
    static Zobrist<N>* _instance;

    // This key is present if it's black's turn.
    uint64_t _blackTurn;

    // Need a hash key for each colour in each location.
    uint64_t _keys[2][N*N];

    Zobrist()
    {
        // Note: This is the seed that Stockfish uses.
        RandomGenerator gen(1070372);
        _blackTurn = gen.Next();
        for (int col = 0; col < 2; col++)
        {
            for (int loc = 0; loc < N*N; loc++)
            {
                _keys[col][loc] = gen.Next();
            }
        }
    }
};

template<unsigned int N>
Zobrist<N>* Zobrist<N>::_instance = nullptr;

#endif // __ZOBRIST_H__
