#ifndef __COMMS_HANDLER_H__
#define __COMMS_HANDLER_H__

#include "core/MoveHistory.h"
#include "TimeInfo.h"
#include <string>

// This class processes incoming messages.
class CommsHandler
{
public:
    std::vector<std::string> _knownCommands = 
    {
        "protocol_version",
        "name",
        "version",
        "known_command",
        "list_commands",
        "quit",
        "boardsize",
        "clear_board",
        "komi",
        "play",
        "genmove",
        "undo",
        "time_settings",
        "time_left",
        "opg_parameters"
    };

    bool Process(const std::string&);

private:
    const int NoID = -1;
    MoveHistory _history;
    unsigned int _boardSize;
    TimeInfo _timeInfos[2];

    std::string PreProcess(const std::string&) const;

    bool IsWhitespaceLine(const std::string&) const;
    bool IsWhitespace(char) const;
    bool IsControl(char) const;
    bool IsHT(char) const;
    bool IsLF(char) const;
    bool IsHash(char) const;
    bool IsInteger(const std::string&) const;

    void SuccessResponse(int, const std::string&) const;
    void FailureResponse(int, const std::string&) const;
    void Response(const std::string&, int, const std::string&) const;

    void Log(const std::string&) const;
};

#endif // __COMMS_HANDLER_H__
