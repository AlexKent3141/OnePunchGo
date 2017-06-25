#ifndef __PATTERN_MATCHER_H__
#define __PATTERN_MATCHER_H__

#include "Pattern.h"
#include "PatternState.h"
#include "BoardSpiral.h"
#include "../Board.h"
#include <vector>

class PatternMatcher
{
public:
    // Load the nxn patterns from the source file and update the state.
    static void Load(const std::string& source, size_t n);

    // Clean up the dynamically allocated memory.
    static void CleanUp();

    // Check whether there is a matching nxn pattern for the specified board location.
    bool HasMatch(const Board& board, int patternSize, int loc) const;
    
    bool HasMatch(const Board& board, Colour colourToMove, int patternSize, int loc) const;

private:
    // Store the patterns for each pattern size.
    static std::vector<Pattern*>* _patterns;

    // The root states for the matcher for each pattern size.
    static PatternState* _roots;

    // Board spirals for each pattern size.
    static BoardSpiral* _boardSpirals;

    // Initialise the board spirals.
    static void InitialiseSpirals();

    // Load the nxn patterns from the source file.
    static void LoadPatterns(const std::string& source, size_t n);

    // Initialise the DFA for the nxn patterns.
    static void InitialiseDFA(int n);
};

#endif // __PATTERN_MATCHER_H__
