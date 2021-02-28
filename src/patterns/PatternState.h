#ifndef __PATTERN_STATE_H__
#define __PATTERN_STATE_H__

#include "PatternCommon.h"
#include <vector>

// This class is a state in the pattern matching state machine.
class PatternState
{
public:
    ~PatternState()
    {
        if (_nextStates != nullptr)
        {
            delete[] _nextStates;
            _nextStates = nullptr;
        }
    }

    void Expand()
    {
        if (!_expanded)
        {
            _nextStates = new PatternState[4];
            _expanded = true;
        }
    }

    // Get the number of patterns that match so far.
    size_t NumPatterns() const
    {
        return _matchingPatterns;
    }

    // Add a pattern to this state.
    void AddPattern()
    {
        ++_matchingPatterns;
    }

    // Get the next state given the next location in the pattern.
    PatternState* Child(Location loc)
    {
        return &_nextStates[(int)loc];
    }

private:
    // Boolean indicating whether this state is expanded.
    bool _expanded = false;

    // The number of matching patterns at this point.
    size_t _matchingPatterns = 0;

    // The reachable states from this one depending on the next location type.
    PatternState* _nextStates = nullptr;
};

#endif // __PATTERN_STATE_H__
