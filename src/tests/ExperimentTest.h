#ifndef __EXPERIMENT_TEST_H__
#define __EXPERIMENT_TEST_H__

#include "TestBase.h"
#include "core/Board.h"
#include "core/Move.h"
#include "search/Current.h"
#include <chrono>
#include <iostream>

class ExperimentTest : public TestBase
{
public:
    std::string TestFileName() const
    {
        return "ExperimentTests.suite";
    }

    // Parse the lines describing the test and execute it.
    bool Run(const std::vector<std::string>& lines)
    {
        Colour col = None;
        std::vector<std::string> board;
        double solution = 0;
        auto state = State::None;
        for (const std::string& line : lines)
        {
            if (StartsWith(line, "Player"))
            {
                state = State::Player;
            }
            else if (StartsWith(line, "Position"))
            {
                state = State::Position;
            }
            else if (StartsWith(line, "Solution"))
            {
                state = State::Solution;
            }
            else if (state == State::Player)
            {
                col = line == "B" ? Black : White;
            }
            else if (state == State::Position)
            {
                board.push_back(line);
            }
            else if (state == State::Solution)
            {
                solution = stof(line);
            }
        }

        return RunTest(col, board, solution);
    }

private:
    enum class State
    {
        None,
        Player,
        Position,
        Solution
    };

    // All test cases are 9x9.
    const int N = 9;

    // Attempt to solve the tsumego.
    // It is always black to move.
    bool RunTest(Colour col, const std::vector<std::string>& lines, double solution) const
    {
        Board board(col, lines);
        std::cout << board.ToString() << std::endl;
        CurrentRules.Komi = 7.5;

        // Spawn a searching thread for this position.
        CurrentSearch search;
        search.Start(board);

        // Allow the search to continue for 5 seconds.
        std::chrono::seconds duration(5);
        std::this_thread::sleep_for(duration);
        search.Stop();

        const MoveStats& best = search.Best();
        double wr = best.WinningChance();
        double error = wr - solution;
        std::cout << "Expected: " << solution << std::endl;
        std::cout << "Got: " << wr << std::endl;
        std::cout << "Error: " << error << std::endl;

        return true;
    }

    bool StartsWith(const std::string& str, const std::string& sub) const
    {
        return str.compare(0, sub.size(), sub) == 0;
    }
};

#endif // __TSUMEGO_TEST_H__

