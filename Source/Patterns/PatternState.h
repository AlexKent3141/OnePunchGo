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

    void AddStates()
    {
        _nextStates = new PatternState[4];
    }

    // Get the number of patterns that match so far.
    size_t NumPatterns() const
    {
        return _matchingPatterns.size();
    }

    // Add a pattern to this state.
    void AddPattern(Pattern* pat)
    {
        _matchingPatterns.push_back(pat);
    }

    // Get the next state given the next location in the pattern.
    PatternState& Child(Location loc)
    {
        return _nextStates[(int)loc];
    }

private:
    // The patterns which match so far.
    std::vector<Pattern*> _matchingPatterns;

    // The reachable states from this one depending on the next location type.
    PatternState* _nextStates = nullptr;
};

#endif // __PATTERN_STATE_H__
