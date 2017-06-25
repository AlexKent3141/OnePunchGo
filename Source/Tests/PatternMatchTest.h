#ifndef __PATTERN_MATCH_TEST_H__
#define __PATTERN_MATCH_TEST_H__

#include "TestBase.h"
#include "../Board.h"
#include "../Move.h"
#include "../Utils.h"
#include "../Patterns/PatternMatcher.h"
#include <iostream>

class PatternMatchTest : public TestBase
{
public:
    std::string TestFileName() const
    {
        return "PatternMatchTests.suite";
    }

    // Parse the lines describing the test and execute it.
    bool Run(const std::vector<std::string>& lines)
    {
        std::vector<Move> moves;
        Colour colourToMove = Black;
        bool isMatch = false;

        bool readingMoves = false;
        bool readingResult = false;
        for (const std::string& line : lines)
        {
            if (StartsWith(line, "Moves"))
            {
                readingMoves = true;
                readingResult = false;
            }
            else if (StartsWith(line, "Result"))
            {
                readingMoves = false;
                readingResult = true;
            }
            else if (readingMoves)
            {
                moves.push_back(StringToMove(line, N));
            }
            else if (readingResult)
            {
                auto split = Utils::GetInstance()->Split(line, ' ');
                colourToMove = split[0] == "B" ? Black : White;
                isMatch = split[1] == "True";
            }
        }

        return RunTest(moves, colourToMove, isMatch);
    }

private:
    // All test cases are 3x3.
    const int N = 3;

    bool RunTest(const std::vector<Move>& moves, Colour colourToMove, bool isMatch) const
    {
        Board board(N);
        for (Move move : moves)
        {
            board.MakeMove(move);
        }

        std::cout << board.ToString() << std::endl;

        bool hasMatch = CheckForPattern(board, colourToMove);
        std::cout << "Match found: " << hasMatch << std::endl;
        return hasMatch == isMatch;
    }

    // Check whether there is a matching pattern for this 3x3 position.
    bool CheckForPattern(const Board& board, Colour colourToMove) const
    {
        int boardCentreLoc = N*N/2;
        PatternMatcher matcher;
        return matcher.HasMatch(board, colourToMove, N, boardCentreLoc);
    }

    bool StartsWith(const std::string& str, const std::string& sub) const
    {
        return str.compare(0, sub.size(), sub) == 0;
    }
};

#endif // __PATTERN_MATCH_TEST_H__

