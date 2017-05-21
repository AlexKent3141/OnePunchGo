#ifndef __NODE_H__
#define __NODE_H__

#include "Move.h"
#include <cassert>

// The statistics for the move.
struct MoveStats
{
    Move LastMove;
    int Visits;
    int Wins;

    // Update with the new score.
    void UpdateScore(int score)
    {
        ++Visits;
        Wins += LastMove.Col == Black ? score : -score;
    }
};

// A node in the dynamically generated MCTS tree.
struct Node
{
    MoveStats Stats;
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
        Node* next = new Node;
        next->Stats = { Moves[Unexpanded++], 0, 0 };
        next->Unexpanded = 0;
        next->Parent = this;
        Children.push_back(next);
        return next;
    }
};

// Make the root node for the tree.
inline Node* MakeRoot()
{
    Node* root = new Node;
    root->Stats = { {}, 0, 0 };
    root->Unexpanded = 0;
    root->Parent = nullptr;
    return root;
}

#endif // __NODE_H__
