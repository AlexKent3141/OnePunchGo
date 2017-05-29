#ifndef __MOVE_H__
#define __MOVE_H__

#include "Types.h"
#include "Utils.h"
#include <string>
#include <cassert>

const int PassCoord = -1;
const int BadCoord = -2;

// The data for a single move.
struct Move
{
    Colour Col;
    int Coord;
    MoveInfo Info;

    bool operator==(const Move& other)
    {
        return Col == other.Col && Coord == other.Coord;
    }

    bool operator!=(const Move& other)
    {
        return !(*this == other);
    }
};

// This represents an invalid move.
const Move BadMove = { None, BadCoord };

inline int StringToCoord(const std::string& str, int n)
{
    int file = std::string("ABCDEFGHJKLMNOPQRSTUVWXYZ").find(str[0]);
    int rank = stoi(str.substr(1)) - 1;
    return file + rank*n;
}

inline std::string CoordToString(int coord, int n)
{
    std::string s;
    if (coord == PassCoord)
    {
        s += "pass";
    }
    else
    {
        s += "ABCDEFGHJKLMNOPQRSTUVWXYZ"[coord % n];
        s += std::to_string(coord / n + 1);
    }

    return s;
}

// Get the string representation of the move.
inline std::string MoveToString(const Move& move, int n)
{
    assert(move.Col != None);
    std::string s = move.Col == Black ? "B " : "W "; 
    s += CoordToString(move.Coord, n);
    return s;
}

inline Move StringToMove(const std::string& str, int n)
{
    Colour col = str[0] == 'B' ? Black : White;
    std::string coordStr = str.substr(2);
    int coord = Utils::GetInstance()->ToLower(coordStr) == "pass"
        ? PassCoord
        : StringToCoord(coordStr, n);

    return {col, coord};
}

#endif // __MOVE_H__

