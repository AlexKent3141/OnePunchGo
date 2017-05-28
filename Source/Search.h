#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "Board.h"
#include "Node.h"
#include "Playout/PlayoutPolicy.h"
#include "Selection/SelectionPolicy.h"

// This class performs executes the MCTS algorithm to find the best move.
// The template parameters are:
// SP: The selection policy to be used.
// PP: The playout policy to be used.
template<class SP, class PP>
class Search
{
static_assert(std::is_base_of<SelectionPolicy, SP>::value, "Not a valid selection policy.");
static_assert(std::is_base_of<PlayoutPolicy, PP>::value, "Not a valid playout policy.");
public:
    inline MoveStats Best() const { return _best; }

    // This method keeps searching until a call to Stop is made.
    void Start(const Board& pos)
    {
        Node* root = MakeRoot();
        root->Moves = pos.GetMoves();

        Board temp(pos.BoardSize());
        _stop = false;
        while (!_stop)
        {
            // Clone the board state.
            temp.CloneFrom(pos);

            // Find the leaf node which must be expanded.
            Node* leaf = Select(temp, root);

            // Expand the leaf node.
            leaf = Expand(temp, leaf);

            // Perform a playout and record the result.
            int res = Simulate(temp);

            // Backpropagate the scores.
            UpdateScores(leaf, res);
        }

        // Find the most promising move.
        int highestVisits = -1;
        for (size_t i = 0; i < root->Children.size(); i++)
        {
            Node const* const child = root->Children[i];
            if (child->Stats.Visits > highestVisits)
            {
                highestVisits = child->Stats.Visits;
                _best = child->Stats;
            }
        }

        delete root;
    }

    // Stop the currently executing search.
    void Stop()
    {
        _stop = true;
    }

private:
    bool _stop = false;
    SP _sp;
    PP _pp;
    MoveStats _best;

    // Select a node to expand.
    Node* Select(Board& temp, Node* root)
    {
        Node* current = root;
        while (current->FullyExpanded() && current->HasChildren())
        {
            current = _sp.Select(current->Children);
            temp.MakeMove(current->Stats.LastMove);
        }

        return current;
    }

    // Expand the chosen leaf node.
    Node* Expand(Board& temp, Node* leaf) const
    {
        Node* expanded = leaf;
        if (!expanded->FullyExpanded())
        {
            expanded = expanded->ExpandNext();
            temp.MakeMove(expanded->Stats.LastMove);
            expanded->Moves = temp.GetMoves();
        }

        return expanded;
    }

    // Perform a simulation from the specified game state.
    int Simulate(Board& temp)
    {
        // Make moves according to the playout policy until a terminal state is reached.
        std::vector<Move> moves = temp.GetMoves(true);
        while (moves.size() > 0)
        {
            Move move = _pp.Select(moves);
            temp.MakeMove(move);
            moves = temp.GetMoves(true);
        }

        return temp.Score();
    }

    // Backpropagate the score from the simulation up the tree.
    void UpdateScores(Node* leaf, int score) const
    {
        while (leaf != nullptr)
        {
            leaf->Stats.UpdateScore(score);
            leaf = leaf->Parent;
        }
    }
};

#endif // __SEARCH_H__
