#ifndef __NODE_H__
#define __NODE_H__

#include "../Move.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>

// The statistics for the move.
struct MoveStats
{
    Move LastMove;
    int Visits;
    int Wins;

    int RaveVisits;
    int RaveWins;

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
        return (double)Wins / Visits;
    }
};

// A node in the dynamically generated MCTS tree.
struct Node : std::enable_shared_from_this<Node>
{
    MoveStats Stats;
    std::weak_ptr<Node> Parent;
    std::vector<Move> Moves; // The moves that are available.
    std::vector<std::shared_ptr<Node>> Children; // The child nodes.

    // This is used to synchronise access to the node from each TreeWorker.
    std::mutex Obj;

    // Check whether the node has children.
    bool HasChildren() const
    {
        return Children.size() > 0;
    }

    void AddChildren()
    {
        for (const Move& move : Moves)
        {
            auto next = std::make_shared<Node>();
            next->Stats = { move, 0, 0, 0, 0, false };
            next->Parent = shared_from_this();
            Children.push_back(next);
        }
    }
};

// Make the root node for the tree.
inline std::shared_ptr<Node> MakeRoot()
{
    auto root = std::make_shared<Node>();
    root->Stats = {};
    root->Parent = std::weak_ptr<Node>();
    return root;
}

#endif // __NODE_H__
