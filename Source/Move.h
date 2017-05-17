#ifndef __MOVE_H__
#define __MOVE_H__

#include "Types.h"
#include <string>
#include <cassert>

// The data for a single move.
struct Move
{
    Colour Col;
    int Coord;
};

inline int StringToCoord(const std::string& str, int n)
{
    int file = std::string("ABCDEFGHJKLMNOPQRSTUVWXYZ").find(str[0]);
    int rank = stoi(str.substr(1)) - 1;
    return file + rank*n;
}

// Get the string representation of the move.
inline std::string MoveToString(const Move& move, int n)
{
    assert(move.Col != None);
    std::string s = move.Col == Black ? "B " : "W "; 
    s += "ABCDEFGHJKLMNOPQRSTUVWXYZ"[move.Coord % n];
    s += std::to_string(move.Coord / n + 1);
    return s;
}

inline Move StringToMove(const std::string& str, int n)
{
    Colour col = str[0] == 'B' ? Black : White;
    int coord = StringToCoord(str.substr(2), n);
    return {col, coord};
}

#endif // __MOVE_H__

