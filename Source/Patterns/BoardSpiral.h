#ifndef __BOARD_SPIRAL_H__
#define __BOARD_SPIRAL_H__

#include <vector>
#include <cstddef>

// This class defines the spiral path that is taken around a location on the board.
// Due to the need for bounds checking we must row and column deltas.
class BoardSpiral
{
public:
    BoardSpiral()
    {
    }

    BoardSpiral(int n)
    {
        _side = n;
        _size = n*n - 1;
        ComputePath();
    }

    inline size_t Size() const { return _size; }

    std::pair<int, int> operator[](const int i) const
    {
        return {_rowPath[i], _colPath[i]};
    }

private:
    size_t _side = 0;
    size_t _size = 0;

    // Row and column deltas for the spiral.
    std::vector<int> _rowPath;
    std::vector<int> _colPath;

    // Calculate the path.
    void ComputePath()
    {
        int numRings = _size/2;
        int ringSize, rowStart, colStart;
        for (int r = numRings-1; r >= 0; r--)
        {
            ringSize = _side - 2*r;
            rowStart = -ringSize/2;
            colStart = rowStart;
            AddRing(ringSize, rowStart, colStart);
        }
    }

    // Add the specified ring.
    void AddRing(int ringSize, int rowStart, int colStart)
    {
        int row = rowStart;
        int col = colStart;

        // Go down the left side.
        for (int i = 1; i < ringSize; i++)
        {
            ++row;
            _rowPath.push_back(row);
            _colPath.push_back(col);
        }

        // Go across to the right.
        for (int i = 1; i < ringSize; i++)
        {
            ++col;
            _rowPath.push_back(row);
            _colPath.push_back(col);
        }

        // Go up the right side.
        for (int i = 1; i < ringSize; i++)
        {
            --row;
            _rowPath.push_back(row);
            _colPath.push_back(col);
        }

        // Go across to the left (back to the start).
        for (int i = 1; i < ringSize; i++)
        {
            --col;
            _rowPath.push_back(row);
            _colPath.push_back(col);
        }
    }
};

#endif // __BOARD_SPIRAL_H__
