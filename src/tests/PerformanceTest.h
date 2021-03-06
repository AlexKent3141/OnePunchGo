#ifndef __PERFORMANCE_TEST_H__
#define __PERFORMANCE_TEST_H__

#include "TestBase.h"
#include "core/Board.h"
#include "core/Move.h"
#include "core/Utils.h"
#include "search/Current.h"
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
        Utils utils;
        auto split = utils.Split(lines[0], ' ');
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
        CurrentSearch search;
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

