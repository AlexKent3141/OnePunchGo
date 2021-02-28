#ifndef __PATTERN_H__
#define __PATTERN_H__

#include "PatternCommon.h"
#include <cassert>
#include <cstring>
#include <vector>
#include <string>

// This class represents a pattern.
class Pattern
{
public:
    Pattern() = delete;

    Pattern(const Pattern& other) : _n(other._n)
    {
        _locations = new Location[_n*_n];
        memcpy(_locations, other._locations, _n*_n*sizeof(Location));
    }

    Pattern(Location* locations, int n) : _n(n)
    {
        _locations = new Location[_n*_n];
        memcpy(_locations, locations, _n*_n*sizeof(Location));
    }

    Pattern(const std::string& patternDef, size_t n) : _n(n)
    {
        assert(n*n == patternDef.size());
        _locations = new Location[patternDef.size()];
        for (size_t i = 0; i < patternDef.size(); i++)
        {
            _locations[i] = LocationFromChar(patternDef[i]);
        }
    }

    ~Pattern()
    {
        if (_locations != nullptr)
        {
            delete[] _locations;
            _locations = nullptr;
        }
    }

    bool operator==(const Pattern& other) const
    {
        if (_n != other._n) return false;
        for (int i = 0; i < _n*_n; i++)
            if (_locations[i] != other._locations[i])
                return false;
        return true;
    }

    bool operator!=(const Pattern& other) const
    {
        return !(*this == other);
    }

    Location operator[](const int i) const
    {
        return _locations[i];
    }

    std::vector<Pattern*> Mirrors()
    {
        std::vector<Pattern*> mirrors;

        // Enumerate all possible rotations/reflections (possibly with duplication).
        std::vector<Pattern*> temp = { this };
        Pattern* current = this;
        Pattern* ref = nullptr, *rot = nullptr;
        for (int i = 0; i < 4; i++)
        {
            ref = current->Reflect();
            rot = current->Rotate();

            temp.push_back(ref);
            temp.push_back(rot);

            current = rot;
        }

        // Now ensure that we don't keep any duplicates.
        for (size_t i = 0; i < temp.size(); i++)
        {
            if (!FindPattern(mirrors, temp[i]))
            {
                mirrors.push_back(temp[i]);
            }
            else
            {
                delete temp[i];
            }
        }

        return mirrors;
    }

    std::string ToString() const
    {
        std::string s;
        for (int i = 0; i < _n; i++)
        {
            for (int j = 0; j < _n; j++)
            {
                s += CharFromLocation(_locations[i*_n + j]);
            }

            s += '\n';
        }

        return s;
    }

private:
    int _n;
    Location* _locations = nullptr;

    // Get the 90 degree anti-clockwise rotation of this pattern.
    Pattern* Rotate() const
    {
        auto locations = new Location[_n*_n];
        for (int r = 0; r < _n; r++)
        {
            // This row becomes a column in the rotated pattern.
            for (int c = 0; c < _n; c++)
            {
                locations[(_n-1-c)*_n + r] = _locations[r*_n + c];
            }
        }

        auto pat = new Pattern(locations, _n);
        delete[] locations;
        
        return pat;
    }

    // Get the reflection of this pattern in the vertical.
    Pattern* Reflect() const
    {
        auto locations = new Location[_n*_n];
        for (int r = 0; r < _n; r++)
        {
            for (int c = 0; c < _n; c++)
            {
                locations[r*_n + (_n-1-c)] = _locations[r*_n + c];
            }
        }

        auto pat = new Pattern(locations, _n);
        delete[] locations;
        
        return pat;
    }

    bool FindPattern(const std::vector<Pattern*>& pats, Pattern* pat) const
    {
        for (size_t i = 0; i < pats.size(); i++)
            if (*pats[i] == *pat)
                return true;
        return false;
    }
};

#endif // __PATTERN_H__
