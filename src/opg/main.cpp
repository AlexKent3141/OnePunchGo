#include "CommsHandler.h"
#include "core/Args.h"
#include "patterns/PatternMatcher.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    PatternMatcher::Load("pat3_v1.txt", 3);
    PatternMatcher::Load("pat5.txt", 5);

    Args::Parse(argc, argv);

    // Start listening for commands.
    CommsHandler handler;
    std::string message;
    while (std::getline(std::cin, message) && handler.Process(message));

    PatternMatcher::CleanUp();

    return 0;
}
