#ifndef __NODE_H__
#define __NODE_H__

#include "Move.h"

// A node in the dynamically generated MCTS tree.
struct Node
{
    Move LastMove;
    int Visits;
    int Wins;
    std::vector<Node*> Children;
};

#endif // __NODE_H__
