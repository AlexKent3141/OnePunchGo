#ifndef __MOVE_HISTORY_H__
#define __MOVE_HISTORY_H__

#include "Move.h"
#include <vector>

// Store the move history for the current game.
// Consecutive moves can be for the same colour.
class MoveHistory
{
public:
    void AddMove(const Move& move)
    {
        _history.push_back(move);
    }

    void UndoLast()
    {
        _history.pop_back();
    }

    void Clear()
    {
        _history.clear();
    }
    
private:
    std::vector<Move> _history;
};

#endif // __MOVE_HISTORY_H__
