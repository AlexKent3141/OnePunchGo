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

    // The neural network evaluation for the node.
    double NetValue;

    // The depth of this node in the MCTS tree.
    int Depth;

    bool Prioritised;

    // Add a virtual loss.
    void VirtualLoss()
    {
        ++Visits;
    }

    // Add a virtual win.
    void VirtualWin()
    {
        --Visits;
    }

    // Update with the new score.
    void UpdateScore(int score)
    {
        ++Visits;

        bool win = (LastMove.Col == Black && score > 0) ||
                   (LastMove.Col == White && score < 0);

        Wins += win ? 1 : 0;
    }

    // Update the rave score.
    void UpdateRaveScore(int score)
    {
        ++RaveVisits;

        bool win = (LastMove.Col == Black && score > 0) ||
                   (LastMove.Col == White && score < 0);

        RaveWins += win ? 1 : 0;
    }

    // Get the probability of winning for this node.
    double WinningChance() const
    {
        assert(Visits > 0);
        return (double)Wins / Visits;
    }
};

// A node in the dynamically generated MCTS tree.
struct Node
{
    MoveStats Stats;
    Node* Parent;
    std::vector<Node*> Children; // The child nodes.
    std::mutex Obj; // This is used to synchronise access to the node from each TreeWorker.

    ~Node()
    {
        for (size_t i = 0; i < Children.size(); i++)
        {
            delete Children[i];
        }
    }

    // Check whether the node has children.
    bool HasChildren() const
    {
        return Children.size() > 0;
    }

    // Fully expand the node.
    void Expand(const std::vector<Move>& moves)
    {
        for (const Move& m : moves)
        {
            Node* next = new Node;
            next->Stats = { m, 0, 0, 0, 0, 0, Stats.Depth + 1, false };
            next->Parent = this;
            Children.push_back(next);
        }
    }
};

// Make the root node for the tree.
inline Node* MakeRoot()
{
    Node* root = new Node;
    root->Stats = {};
    root->Parent = nullptr;
    return root;
}

#endif // __NODE_H__
