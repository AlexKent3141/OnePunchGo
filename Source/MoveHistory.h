#ifndef __MOVE_HISTORY_H__
#define __MOVE_HISTORY_H__

#include "Move.h"
#include <vector>

// Store the move history for the current game.
// Consecutive moves can be for the same colour.
class MoveHistory
{
public:
    inline std::vector<Move> Moves() const { return _history; }

    inline size_t Size() const { return _history.size(); }

    // Add a move to the history.
    void AddMove(const Move& move)
    {
        _history.push_back(move);
    }

    // Undo the last move.
    void UndoLast()
    {
        _history.pop_back();
    }

    // Clear the history.
    void Clear()
    {
        _history.clear();
    }

    bool empty() const { return _history.empty(); }

    size_t size() const { return _history.size(); }

    Move LastMove() const { return _history.back(); }
    
private:
    std::vector<Move> _history;
};

#endif // __MOVE_HISTORY_H__
