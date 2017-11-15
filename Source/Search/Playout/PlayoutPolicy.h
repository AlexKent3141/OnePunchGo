#ifndef __PLAYOUT_POLICY_H__
#define __PLAYOUT_POLICY_H__

#include "../../Board.h"
#include <vector>

// A playout policy defines a method which attempts to select the most promising move during
// a playout.
class PlayoutPolicy
{
public:
    virtual Move Select(const Board& board, const Move& lastMove)
    {
        return board.GetMoves(true)[0];
    }

    virtual ~PlayoutPolicy() {}
};

#endif // __PLAYOUT_POLICY_H__
