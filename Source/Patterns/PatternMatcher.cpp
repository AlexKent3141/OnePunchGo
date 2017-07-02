#include "BoardSpiral.h"
#include "Pattern.h"
#include "PatternMatcher.h"
#include "PatternSpiral.h"
#include "../Types.h"
#include <fstream>

std::vector<Pattern*>* PatternMatcher::_patterns = nullptr;
PatternState* PatternMatcher::_roots = nullptr;
BoardSpiral* PatternMatcher::_boardSpirals = nullptr;

void PatternMatcher::Load(const std::string& source, size_t n)
{
    if (_patterns == nullptr) _patterns = new std::vector<Pattern*>[MaxPatternSize+1];
    if (_roots == nullptr) _roots = new PatternState[MaxPatternSize+1];
    if (_boardSpirals == nullptr) InitialiseSpirals();

    if (_patterns[n].size() == 0)
    {
        // Load the patterns and construct the DFA for them.
        LoadPatterns(source, n);
        InitialiseDFA(n);
    }
}

void PatternMatcher::CleanUp()
{
    if (_patterns != nullptr)
    {
        delete[] _patterns;
        _patterns = nullptr;
    }

    if (_roots != nullptr)
    {
        delete[] _roots;
        _roots = nullptr;
    }
}

// Check whether the specified location on the board matches one of the nxn patterns.
bool PatternMatcher::HasMatch(const Board& board, int patternSize, int loc) const
{
    const Colour CurrentPlayer = board.ColourToMove();
    return HasMatch(board, CurrentPlayer, patternSize, loc);
}

bool PatternMatcher::HasMatch(const Board& board, Colour colourToMove, int patternSize, int loc) const
{
    const int BoardSize = board.Size();
    BoardSpiral& sp = _boardSpirals[patternSize];

    PatternState* current = &_roots[patternSize];

    int currentRow, currentCol;
    int row = loc / BoardSize;
    int col = loc % BoardSize;
    bool onBoard;
    for (size_t i = 0; i < sp.Size(); i++)
    {
        auto next = sp[i];
        currentRow = row + next.first;
        currentCol = col + next.second;

        // What is the location type on the board at this point.
        onBoard = currentRow >= 0 && currentRow < BoardSize &&
                  currentCol >= 0 && currentCol < BoardSize;

        Location type = OffBoard;
        if (onBoard)
        {
            int loc = currentRow*BoardSize + currentCol;
            Colour col = board.PointColour(loc);
            type = col == None ? Empty
                : col == colourToMove ? Player
                : Opponent;
        }

        current = current->Child(type);
        if (current->NumPatterns() == 0)
        {
            return false;
        }
    }

    return true;
}

void PatternMatcher::InitialiseSpirals()
{
    _boardSpirals = new BoardSpiral[MaxPatternSize+1];
    for (int i = 3; i <= MaxPatternSize; i++)
    {
        _boardSpirals[i] = BoardSpiral(i);
    }
}

// Load the patterns from the file and store all reflections/rotations of each pattern.
void PatternMatcher::LoadPatterns(const std::string& source, size_t n)
{
    std::ifstream s(source);
    std::string line, currentPattern;
    size_t lineNo = 0;
    while (std::getline(s, line))
    {
        if (lineNo > 0 && lineNo < n+1)
        {
            // Append to the current pattern.
            currentPattern += line.substr(0, n);
        }
        else if (currentPattern.size() > 0)
        {
            // Save this pattern and all reflections.
            auto pat = new Pattern(currentPattern, n);
            auto mirrors = pat->Mirrors();
            _patterns[n].insert(_patterns[n].end(), mirrors.begin(), mirrors.end());
            currentPattern = "";
        }

        lineNo = (lineNo+1) % (n+2);
    }

    s.close();
}

// Once all of the nxn patterns have been loaded construct the DFA.
void PatternMatcher::InitialiseDFA(int n)
{
    PatternSpiral sp(n);
    int spLoc;
    for (size_t i = 0; i < _patterns[n].size(); i++)
    {
        // Classify this pattern.
        Pattern const* pat = _patterns[n][i];

        PatternState* current = &_roots[n];
        for (size_t j = 0; j < sp.Size(); j++)
        {
            spLoc = sp[j];
            Location loc = (*pat)[spLoc];

            current->AddPattern();
            current->Expand();
            current = current->Child(loc);
        }

        current->AddPattern();
    }
}
