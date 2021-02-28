#ifndef __PATTERN_TYPES_H__
#define __PATTERN_TYPES_H__

const int MaxPatternSize = 5;

enum Location
{
    Empty,
    Player,
    Opponent,
    OffBoard
};

inline Location LocationFromChar(char c)
{
    Location loc = Empty;
    if (c == 'P') loc = Player;
    else if (c == 'O') loc = Opponent;
    else if (c == 'X') loc = OffBoard;
    return loc;
}

inline char CharFromLocation(Location loc)
{
    char c = '.';
    if (loc == Player) c = 'P';
    else if (loc == Opponent) c = 'O';
    else if (loc == OffBoard) c = 'X';
    return c;
}

#endif // __PATTERN_TYPES_H__
