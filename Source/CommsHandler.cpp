#include "Board.h"
#include "CommsHandler.h"
#include "CustomParameters.h"
#include "Globals.h"
#include "Move.h"
#include "Rules.h"
#include "Search.h"
#include "Utils.h"
#include "Selection/UCB1.h"
#include "Playout/Uniform.h"
#include "Playout/BestOf.h"
#include "Playout/BiasedBestOf.h"
#include <cctype>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

bool CommsHandler::Process(const std::string& message)
{
    Log("Received: " + message);
    bool alive = true;

    std::string cmd = PreProcess(message);
    auto tokens = Utils::GetInstance()->Split(cmd, ' ');
    if (tokens.size() > 0)
    {
        int i = 0;

        // Is the first token an integer (id)?
        int id = NoID;
        if (IsInteger(tokens[i]))
        {
            id = stoi(tokens[i++]);
        }

        // Check the type of command.
        std::string command = tokens[i++];
        if (command == "protocol_version")
        {
            SuccessResponse(id, "2.0");
        }
        else if (command == "name")
        {
            SuccessResponse(id, "OnePunchGo");
        }
        else if (command == "version")
        {
            SuccessResponse(id, "1.0");
        }
        else if (command == "known_command")
        {
            std::string cmd = tokens[i++];
            auto it = std::find(_knownCommands.begin(), _knownCommands.end(), cmd);
            bool known = it != _knownCommands.end();
            SuccessResponse(id, known ? "true" : "false");
        }
        else if (command == "list_commands")
        {
            std::string knownCommands = "";
            for (size_t i = 0; i < _knownCommands.size(); i++)
                knownCommands += _knownCommands[i] + " ";
            
            SuccessResponse(id, knownCommands);
        }
        else if (command == "quit")
        {
            alive = false;
        }
        else if (command == "boardsize")
        {
            int size = stoi(tokens[i++]);
            if (size > 0 && size <= MaxBoardSize)
            {
                _boardSize = size;
                _history.Clear();
                SuccessResponse(id, "");
            }
            else
            {
                FailureResponse(id, "unacceptable size");
            }
        }
        else if (command == "clear_board")
        {
            _history.Clear();
            SuccessResponse(id, "");
        }
        else if (command == "komi")
        {
            CurrentRules.Komi = stof(tokens[i++]);
            SuccessResponse(id, "");
        }
        else if (command == "play")
        {
            // A move was specified.
            std::string move = tokens[i] + " " + tokens[i+1];
            _history.AddMove(StringToMove(move, _boardSize));
            SuccessResponse(id, "");
        }
        else if (command == "genmove")
        {
            // Construct the current board state.
            std::string colString = Utils::GetInstance()->ToLower(tokens[i]);
            Colour col = colString == "black" || colString == "b" ? Black : White;
            Board board(col, _boardSize, _history);

            Log(board.ToString());

            // Search for a fixed amount of time.
            Search<UCB1, BiasedBestOf<4>> search;
            search.Start(board);

            const TimeInfo& timeInfo = _timeInfos[(int)col-1];
            int timeForMove = timeInfo.TimeForMove(_boardSize, _history.Size());
            Log("TimeForMove: " + std::to_string(timeForMove));
            std::chrono::milliseconds searchTime(timeForMove);
            std::this_thread::sleep_for(searchTime);
            search.Stop();

            const MoveStats& best = search.Best();
            const Move& move = best.LastMove;
            double winRate = best.WinningChance();

            Log("Visits: " + std::to_string(best.Visits));
            Log("WinRate: " + std::to_string(winRate));

            const double ResignThreshold = 0.1;
            const double PassThreshold = 0.9999;
            if (winRate > PassThreshold)
            {
                _history.AddMove({col, PassCoord});
                SuccessResponse(id, "pass");
            }
            else if (winRate > ResignThreshold)
            {
                _history.AddMove(move);
                SuccessResponse(id, CoordToString(move.Coord, _boardSize));
            }
            else
            {
                SuccessResponse(id, "resign");
            }
        }
        else if (command == "undo")
        {
            _history.UndoLast();
            SuccessResponse(id, "");
        }
        else if (command == "time_settings")
        {
            int mainTime = stoi(tokens[i]);
            int byoyomiTime = stoi(tokens[i+1]);
            int byoyomiStones = stoi(tokens[i+2]);
            _timeInfos[0] = TimeInfo(mainTime, byoyomiTime, byoyomiStones);
            _timeInfos[1] = TimeInfo(mainTime, byoyomiTime, byoyomiStones);
            SuccessResponse(id, "");
        }
        else if (command == "time_left")
        {
            std::string colString = Utils::GetInstance()->ToLower(tokens[i]);
            Colour col = colString == "black" || colString == "b" ? Black : White;
            int timeLeft = stoi(tokens[i+1]);
            int stonesLeft = stoi(tokens[i+2]);
            _timeInfos[(int)col-1].TimeLeft(timeLeft, stonesLeft);
            SuccessResponse(id, "");
        }
        else if (command == "opg_parameters")
        {
            auto params = CustomParameters::GetInstance();
            for (size_t j = i; j < tokens.size(); j++)
            {
                params->AddParameter(stof(tokens[j]));
            }

            SuccessResponse(id, "");
        }
        else
        {
            FailureResponse(id, "unknown command");
        }
    }

    return alive;
}

std::string CommsHandler::PreProcess(const std::string& command) const
{
    std::string processedCommand = "";

    // Cannot be whitespace only.
    if (command.size() == 0 || IsWhitespaceLine(command))
        return processedCommand;

    // Check the individual characters.
    char current;
    for (size_t i = 0; i < command.size(); i++)
    {
        current = command[i];

        // If a hash is found then ignore the rest (it's commented).
        // Line feed also indicates that the command has finished.
        if (IsHash(current) || IsLF(current))
            break;

        // Don't allow many control characters.
        if (IsControl(current))
        {
            // Convert horizontal tabs to spaces.
            if (IsHT(current))
            {
                processedCommand += ' ';
            }
        }
        else
        {
            // Pass through normal characters.
            processedCommand += current;
        }
    }

    return processedCommand;
}

bool CommsHandler::IsWhitespaceLine(const std::string& line) const
{
    bool res = true;
    for (size_t i = 0; i < line.size(); i++)
        res &= IsWhitespace(line[i]);

    return res;
}

bool CommsHandler::IsWhitespace(char c) const
{
    return c == ' '  || c == '\t' || c == '\r' ||
           c == '\n' || c == '\v' || c == '\f';
}

bool CommsHandler::IsControl(char c) const
{
    return c < 32;
}

bool CommsHandler::IsHT(char c) const
{
    return c == 9;
}

bool CommsHandler::IsLF(char c) const
{
    return c == 10;
}

bool CommsHandler::IsHash(char c) const
{
    return c == 35;
}

bool CommsHandler::IsInteger(const std::string& s) const
{
    bool res = true;
    for (size_t i = 0; i < s.size(); i++)
        res &= isdigit(s[i]);

    return res;
}

void CommsHandler::SuccessResponse(int id, const std::string& data) const
{
    Response("=", id, data);
}

void CommsHandler::FailureResponse(int id, const std::string& data) const
{
    Response("?", id, data);
}

void CommsHandler::Response(const std::string& prefix, int id, const std::string& data) const
{
    std::string res = prefix
        + (id != NoID ? std::to_string(id) : "")
        + (data.size() > 0 ? " " + data : "")
        + "\n\n";

    std::cout << res;
    Log("Sent: " + res);
}

void CommsHandler::Log(const std::string& message) const
{
    std::ofstream log("CommsLog.txt", std::ios_base::app);
    log << message << std::endl;
    log.close();
}
