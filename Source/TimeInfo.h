#ifndef __TIME_INFO_H__
#define __TIME_INFO_H__

#include <cassert>
#include <fstream>

// This class encapsulates all time management info.
class TimeInfo
{
public:
    TimeInfo() : _mainTime(0), _byoyomiTime(0), _byoyomiStones(0)
    {
    }

    TimeInfo(int mainTime, int byoyomiTime, int byoyomiStones) :
        _mainTime(mainTime), _byoyomiTime(byoyomiTime), _byoyomiStones(byoyomiStones)
    {
    }

    void TimeLeft(int timeLeft, int stonesLeft)
    {
        _timeLeft = timeLeft;
        _stonesLeft = stonesLeft;
    }

    // Calculate the amount of time that should be used for this move.
    int TimeForMove(int boardSize, int numMovesMade) const
    {
        double timeAvailable = 0;
        if (_mainTime == 0 && _byoyomiTime == 0)
        {
            timeAvailable = 5;
        }
        else if (_byoyomiStones == 0)
        {
            // Estimate the number of turns remaining.
            const int MinTurnsRemaining = 10;
            int turnsLeftInGame = std::max(MinTurnsRemaining, boardSize*boardSize/3 - numMovesMade);
            timeAvailable = _byoyomiTime + (double)_timeLeft / turnsLeftInGame;
        }
        else
        {
            const double Buffer = 0.2;
            int stones = _stonesLeft == 0 ? _byoyomiStones : _stonesLeft;
            timeAvailable = (_byoyomiTime - Buffer) / stones;
        }
        
        return 1000*timeAvailable;
    }

private:
    int _mainTime, _byoyomiTime, _byoyomiStones;
    int _timeLeft, _stonesLeft;
};

#endif // __TIME_INFO_H__
