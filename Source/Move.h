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
inline std::string MoveToString(const Move& move, int n)
{
    assert(move.Col != None);
    std::string s = move.Col == Black ? "B " : "W "; 
    s += "ABCDEFGHJKLMNOPQRSTUVWXYZ"[move.Point % n];
    s += std::to_string(move.Point / n + 1);
    return s;
}

inline Move StringToMove(const std::string& str, int n)
{
    Colour col = str[0] == 'B' ? Black : White;
    int file = std::string("ABCDEFGHJKLMNOPQRSTUVWXYZ").find(str[2]);
    int rank = stoi(str.substr(3)) - 1;
    return {col, file + rank*n};
}

#endif // __MOVE_H__

