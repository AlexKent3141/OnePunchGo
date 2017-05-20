#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "Board.h"
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
    template<unsigned int N>
    void Start(const Board<N>& pos)
    {
    }

    void Stop()
    {
    }

private:
};

#endif // __SEARCH_H__
