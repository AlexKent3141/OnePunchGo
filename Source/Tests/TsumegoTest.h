#ifndef __TSUMEGO_TEST_H__
#define __TSUMEGO_TEST_H__

#include "TestBase.h"
#include "../Board.h"
#include "../Move.h"
#include "../Node.h"
#include "../Playout/Uniform.h"
#include "../Search.h"
#include "../Selection/UCB1.h"
#include <chrono>
#include <iostream>
#include <thread>

class TsumegoTest : public TestBase
{
public:
    std::string TestFileName() const
    {
        return "TsumegoTests.suite";
    }

    // Parse the lines describing the test and execute it.
    bool Run(const std::vector<std::string>& lines)
    {
        std::vector<std::string> board;
        Move solution;
        bool readingBoard = false;
        bool readingSolution = false;
        for (const std::string& line : lines)
        {
            if (StartsWith(line, "Position"))
            {
                readingBoard = true;
                readingSolution = false;
            }
            else if (StartsWith(line, "Solution"))
            {
                readingBoard = false;
                readingSolution = true;
            }
            else if (readingBoard)
            {
                board.push_back(line);
            }
            else if (readingSolution)
            {
                solution = StringToMove(line, N);
            }
        }

        return RunTest(board, solution);
    }

private:
    // All test cases are 9x9.
    const int N = 9;

    // Attempt to solve the tsumego.
    // It is always black to move.
    bool RunTest(const std::vector<std::string>& lines, const Move& solution) const
    {
        Board board(Black, lines);
        std::cout << board.ToString() << std::endl;
        CurrentRules.Komi = N*N - 1; // Black must kill everything to win.

        // Spawn a searching thread for this position.
        Search<UCB1, Uniform> search;
        std::thread searchThread([&] { search.Start(board); });
        searchThread.detach();

        // Allow the search to continue for 5 seconds.
        std::chrono::seconds duration(5);
        std::this_thread::sleep_for(duration);
        search.Stop();

        // Wait a little longer to ensure the search has completed.
        std::chrono::seconds delay(1);
        std::this_thread::sleep_for(delay);

        const MoveStats& best = search.Best();
        const Move& move = best.LastMove;
        std::cout << MoveToString(move, N) << std::endl;
        std::cout << "Win rate: " << (double)best.Wins / best.Visits << std::endl;

        return move.Col == solution.Col && move.Coord == solution.Coord;
    }

    bool StartsWith(const std::string& str, const std::string& sub) const
    {
        return str.compare(0, sub.size(), sub) == 0;
    }
};

#endif // __TSUMEGO_TEST_H__

