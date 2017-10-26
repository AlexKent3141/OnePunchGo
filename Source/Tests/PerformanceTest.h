#ifndef __PERFORMANCE_TEST_H__
#define __PERFORMANCE_TEST_H__

#include "TestBase.h"
#include "../Board.h"
#include "../Move.h"
#include "../Node.h"
#include "../Playout/Uniform.h"
#include "../Playout/BestOf.h"
#include "../Playout/BiasedBestOf.h"
#include "../Search.h"
#include "../Selection/UCBPriors.h"
#include "../Selection/UCB1.h"
#include "../Utils.h"
#include <cassert>
#include <chrono>
#include <iostream>

class PerformanceTest : public TestBase
{
public:
    std::string TestFileName() const
    {
        return "PerformanceTests.suite";
    }

    // Parse the lines describing the test and execute it.
    bool Run(const std::vector<std::string>& lines)
    {
        // There should be one line containing the board size and search duration.
        assert(lines.size() == 1);
        auto split = Utils::GetInstance()->Split(lines[0], ' ');
        assert(split.size() == 2);

        int boardSize = stoi(split[0]);
        int duration = stoi(split[1]);

        return RunTest(boardSize, duration);
    }

private:
    bool RunTest(int boardSize, int duration) const
    {
        Board board(boardSize);

        // Spawn a searching thread for this position.
        Search<UCB1, BiasedBestOf<4>> search;
        search.Start(board);

        // Allow the search to continue for the specified duration.
        std::chrono::seconds searchTime(duration);
        std::this_thread::sleep_for(searchTime);
        search.Stop();

        const MoveStats& best = search.Best();
        const Move& move = best.LastMove;
        std::cout << MoveToString(move, boardSize) << std::endl;
        std::cout << "Tree size: " << search.TreeSize() << std::endl;

        return true;
    }
};

#endif // __PERFORMANCE_TEST_H__

