#ifndef __TYPES_H__
#define __TYPES_H__

enum Colour
{
    None,
    Black,
    White
};

typedef int MoveInfo;

// Types of move (can be combined).
const MoveInfo Occupied = 1 << 0;
const MoveInfo Suicide = 1 << 1;
const MoveInfo Repetition = 1 << 2;
const MoveInfo Legal = 1 << 3;
const MoveInfo SelfAtari = 1 << 4;
const MoveInfo Atari = 1 << 5;
const MoveInfo Save = 1 << 6;
const MoveInfo Capture = 1 << 7;
const MoveInfo FillsEye = 1 << 8;
const MoveInfo Pat3Match = 1 << 9;
const MoveInfo Pat5Match = 1 << 10;
const MoveInfo Local = 1 << 11;
const MoveInfo Connection = 1 << 12;
const MoveInfo EyeShape = 1 << 13;

#endif // __TYPES_H__
