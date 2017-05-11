#ifndef __MOVE_H__
#define __MOVE_H__

#include "Types.h"
#include <string>
#include <cassert>

// The data for a single move.
struct Move
{
    Colour Col;
    int Point;
};

// Get the string representation of the move.
inline std::string mtos(const Move& move, int n)
{
    assert(move.Col != None);
    std::string s = move.Col == Black ? "B " : "W "; 
    s += "ABCDEFGHJKLMNOPQRSTUVWXYZ"[move.Point % n];
    s += std::to_string(move.Point / n + 1);
    return s;
}

#endif // __MOVE_H__

