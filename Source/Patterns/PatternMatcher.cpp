#include "PatternMatcher.h"
#include "Pattern.h"
#include <fstream>

std::vector<Pattern*>* PatternMatcher::_patterns = nullptr;
PatternState* PatternMatcher::_roots = nullptr;

void PatternMatcher::Load(const std::string& source, size_t n)
{
    if (_patterns == nullptr) _patterns = new std::vector<Pattern*>[MaxPatternSize];
    if (_roots == nullptr) _roots = new PatternState[MaxPatternSize];

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

bool PatternMatcher::HasMatch(const Board& board, int n, int row, int col) const
{
    return false;
}

void PatternMatcher::LoadPatterns(const std::string& source, size_t n)
{
    std::ifstream s(source);
    std::string line, currentPattern;
    while (std::getline(s, line))
    {
        if (line.size() >= n)
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
    }

    s.close();
}

void PatternMatcher::InitialiseDFA(int n)
{
}
