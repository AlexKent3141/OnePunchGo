#ifndef __KO_DETECTION_TEST_H__
#define __KO_DETECTION_TEST_H__

#include "TestBase.h"
#include "../Move.h"
#include "../Utils.h"
#include <iostream>

class KoDetectionTest : public TestBase
{
public:
    std::string TestFileName() const
    {
        return "KoDetectionTests.suite";
    }

    // Parse the lines describing the test and execute it.
    bool Run(const std::vector<std::string>& lines)
    {
        std::vector<Move> moves;
        std::string koCoord;
        bool readingMoves = false;
        bool readingKo = false;
        for (const std::string& line : lines)
        {
            if (StartsWith(line, "Moves"))
            {
                readingMoves = true;
                readingKo = false;
            }
            else if (StartsWith(line, "Ko"))
            {
                readingMoves = false;
                readingKo = true;
            }
            else if (readingMoves)
            {
                moves.push_back(StringToMove(line, N));
            }
            else if (readingKo)
            {
                koCoord = line;
            }
        }

        return RunTest(moves, koCoord);
    }

private:
    static const int N = 9;

    bool RunTest(const std::vector<Move>& moves, const std::string& koCoord) const
    {
        Board<N> board;
        return CheckMoves(board, moves) && CheckKo(board, koCoord);
    }

    // Check that the sequence of moves is legal.
    bool CheckMoves(Board<N>& board, const std::vector<Move>& moves) const
    {
        bool pass = true;
        for (Move move : moves)
        {
            std::cout << MoveToString(move, N) << std::endl;
            if (board.CheckLegal(move.Col, move.Coord) == Legal)
            {
                board.MakeMove(move);
            }
            else
            {
                std::cout << board.ToString() << std::endl;
                pass = false;
                break;
            }
        }

        return pass;
    }

    // Check whether the specified ko coordinate is correctly recognised.
    bool CheckKo(const Board<9>& board, const std::string& koCoord) const
    {
        std::cout << board.ToString() << std::endl;
        int coord = StringToCoord(koCoord, N);
        return board.CheckLegal(coord) == Ko;
    }

    bool StartsWith(const std::string& str, const std::string& sub) const
    {
        return str.compare(0, sub.size(), sub) == 0;
    }

    Colour ParseColour(char c) const
    {
        return c == '.' ? None : c == 'B' ? Black : White;
    }
};

#endif // __KO_DETECTION_TEST_H__

