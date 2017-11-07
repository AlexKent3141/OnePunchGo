#ifndef __NODE_H__
#define __NODE_H__

#include "../Move.h"
#include <cassert>
#include <iostream>
#include <mutex>

// The statistics for the move.
struct MoveStats
{
    Move LastMove;
    int Visits;
    int Wins;

    int RaveVisits;
    int RaveWins;

    // Add a virtual loss.
    void VirtualLoss()
    {
        ++Visits;
        Wins += LastMove.Col == Black ? -1 : 1;
    }

    // Add a virtual win.
    void VirtualWin()
    {
        --Visits;
        Wins += LastMove.Col == Black ? 1 : -1;
    }

    // Update with the new score.
    void UpdateScore(int score)
    {
        ++Visits;
        Wins += LastMove.Col == Black ? score : -score;
    }

    // Update the rave score.
    void UpdateRaveScore(int score)
    {
        ++RaveVisits;
        RaveWins += LastMove.Col == Black ? score : -score;
    }

    // Get the probability of winning for this node.
    double WinningChance() const
    {
        return ((double)Wins / Visits + 1) / 2;
    }
};

// A node in the dynamically generated MCTS tree.
struct Node
{
    MoveStats Stats;
    size_t Unexpanded;
    Node* Parent;
    std::vector<Move> Moves; // The moves that are available.
    std::vector<Node*> Children; // The child nodes.
    std::mutex Obj; // This is used to synchronise access to the node from each TreeWorker.

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
