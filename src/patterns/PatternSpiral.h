#ifndef __PATTERN_SPIRAL_H__
#define __PATTERN_SPIRAL_H__

#include <vector>

// This class defines the spiral path that is taken around a pattern.
// Each spiral starts in the upper-left corner of the innermost ring and moves outwards once the
// ring is complete.
class PatternSpiral
{
public:
    PatternSpiral() = delete;

    PatternSpiral(int n)
    {
        _side = n;
        _size = n*n - 1;
        ComputePath();
    }

    inline size_t Size() const { return _size; }

    int operator[](const int i) const
    {
        return _path[i];
    }

private:
    size_t _side = 0;
    size_t _size = 0;

    // The indices for each location on the path following the spiral.
    std::vector<int> _path;

    // Calculate the path.
    void ComputePath()
    {
        int numRings = _size/2;
        int ringSize, ringStart;
        for (int r = numRings-1; r >= 0; r--)
        {
            ringSize = _side - 2*r;
            ringStart = r*(_side+1);
            AddRing(ringSize, ringStart);
        }
    }

    // Add the specified ring.
    void AddRing(int ringSize, int ringStart)
    {
        int current = ringStart;

        // Go down the left side.
        for (int i = 1; i < ringSize; i++)
        {
            current += _side;
            _path.push_back(current);
        }

        // Go across to the right.
        for (int i = 1; i < ringSize; i++)
        {
            ++current;
            _path.push_back(current);
        }

        // Go up the right side.
        for (int i = 1; i < ringSize; i++)
        {
            current -= _side;
            _path.push_back(current);
        }

        // Go across to the left (back to the start).
        for (int i = 1; i < ringSize; i++)
        {
            --current;
            _path.push_back(current);
        }
    }
};

#endif // __PATTERN_SPIRAL_H__
