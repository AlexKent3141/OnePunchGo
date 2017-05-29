#ifndef __TYPES_H__
#define __TYPES_H__

enum Colour
{
    None,
    Black,
    White
};

typedef int MoveInfo;

const MoveInfo Legal = 1 << 0;
const MoveInfo Occupied = 1 << 1;
const MoveInfo Suicide = 1 << 2;
const MoveInfo Ko = 1 << 3;
const MoveInfo Capture = 1 << 4;
const MoveInfo Atari = 1 << 5;
const MoveInfo SelfAtari = 1 << 6;

#endif // __TYPES_H__
