#ifndef __MAKE_MOVE_TEST_H__
#define __MAKE_MOVE_TEST_H__

#include "TestBase.h"
#include "../Board.h"
#include "../Move.h"
#include "../Utils.h"
#include <iostream>

class MakeMoveTest : public TestBase
{
public:
    std::string TestFileName() const
    {
        return "MakeMoveTests.suite";
    }

    // Parse the lines describing the test and execute it.
    bool Run(const std::vector<std::string>& lines)
    {
        std::vector<Move> moves;
        std::vector<std::string> finalBoard;
        bool readingMoves = false;
        bool readingBoard = false;
        for (const std::string& line : lines)
        {
            if (StartsWith(line, "Moves"))
            {
                readingMoves = true;
                readingBoard = false;
            }
            else if (StartsWith(line, "Final"))
            {
                readingMoves = false;
                readingBoard = true;
            }
            else if (readingMoves)
            {
                moves.push_back(StringToMove(line, N));
            }
            else if (readingBoard)
            {
                finalBoard.push_back(line);
            }
        }

        return RunTest(moves, finalBoard);
    }

private:
    // All test cases are 9x9.
    const int N = 9;

    bool RunTest(const std::vector<Move>& moves, const std::vector<std::string>& finalBoard) const
    {
        Board board(N);
        return CheckMoves(board, moves) && CheckBoard(board, finalBoard);
    }

    // Check that the sequence of moves is legal.
    bool CheckMoves(Board& board, const std::vector<Move>& moves) const
    {
        bool pass = true;
        for (Move move : moves)
        {
            std::cout << MoveToString(move, N) << std::endl;
            if (board.CheckMove(move.Col, move.Coord) & Legal)
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

    // Check that the final board state is correct.
    bool CheckBoard(const Board& board, const std::vector<std::string>& finalBoard) const
    {
        bool pass = true;
        for (int r = N-1; r >= 0 && pass; r--)
        {
            for (int c = 0; c < N && pass; c++)
            {
                pass = board.PointColour(r*N+c) == ParseColour(finalBoard[N-1-r][c]);
            }
        }

        std::cout << board.ToString() << std::endl;
        return pass;
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

#endif // __MAKE_MOVE_TEST_H__

