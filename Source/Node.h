#ifndef __NODE_H__
#define __NODE_H__

#include "Move.h"
#include <cassert>

// A node in the dynamically generated MCTS tree.
struct Node
{
    Move LastMove;
    int Visits;
    int Wins;
    int Unexpanded;
    Node* Parent;
    std::vector<Move> Moves; // The moves that are available.
    std::vector<Node*> Children; // The child nodes.

    ~Node()
    {
        for (size_t i = 0; i < Children.size(); i++)
        {
            delete Children[i];
        }
    }

    // Check whether the node is fully expanded.
    bool FullyExpanded() const
    {
        return Unexpanded >= Moves.size();
    }

    // Check whether the node has children.
    bool HasChildren() const
    {
        return Children.size() > 0;
    }

    // Expand the next available move.
    Node* ExpandNext()
    {
        assert(!FullyExpanded());
        const Move& move = Moves[Unexpanded++];
        Node* next = new Node;
        next->LastMove = move;
        next->Parent = this;
        Children.push_back(next);
        return next;
    }

    // Update the stats for this node with the new score.
    void UpdateScore(int score)
    {
        ++Visits;
        Wins += LastMove.Col == Black ? score : -score;
    }
};

#endif // __NODE_H__
