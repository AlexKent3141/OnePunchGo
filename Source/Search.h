#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "Board.h"
#include "Node.h"
#include "PlayoutPolicy.h"
#include "SelectionPolicy.h"

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
    // This method keeps searching until a call to Stop is made.
    template<unsigned int N>
    void Start(const Board<N>& pos)
    {
        Node* root = new Node;
        Board<N> temp;
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

    // Select a node to expand.
    template<unsigned int N>
    Node* Select(Board<N>& temp, Node* root) const
    {
        Node* current = root;
        while (current->FullyExpanded() && current->HasChildren())
        {
            current = _sp.Select(current->Children);
            temp.MakeMove(current->LastMove);
        }

        return current;
    }

    // Expand the chosen leaf node.
    template<unsigned int N>
    Node* Expand(Board<N>& temp, Node* leaf) const
    {
        Node* expanded = leaf;
        if (!expanded->FullyExpanded())
        {
            expanded = expanded->ExpandNext();
            temp.MakeMove(expanded->LastMove);
        }

        return expanded;
    }

    // Perform a simulation from the specified game state.
    template<unsigned int N>
    int Simulate(Board<N>& temp) const
    {
        // Make moves according to the playout policy until a terminal state is reached.
        std::vector<Move> moves = temp.GetMoves();
        while (moves.size() > 0)
        {
            Move move = _pp.Select(moves);
            temp.MakeMove(move);
            moves = temp.GetMoves();
        }

        return temp.Score();
    }

    // Backpropagate the score from the simulation up the tree.
    void UpdateScores(Node* leaf, int score) const
    {
        while (leaf != nullptr)
        {
            leaf->UpdateScore(score);
            leaf = leaf->Parent;
        }
    }
};

#endif // __SEARCH_H__
