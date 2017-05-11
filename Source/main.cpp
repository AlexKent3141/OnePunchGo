#include "Board.h"
#include "Move.h"
#include <vector>
#include <iostream>

int main()
{
    const unsigned int N = 19;
    Board<N> board;
    auto moves = board.GetMoves();
    for (auto& move : moves)
    {
        std::cout << mtos(move, N) << std::endl;
    }

    return 0;
}
