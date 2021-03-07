#include "CommsHandler.h"
#include "core/Args.h"
#include "lurien.h"
#include "patterns/PatternMatcher.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    LURIEN_INIT(std::make_unique<lurien::DefaultOutputReceiver>(std::cout))

    PatternMatcher::Load("pat3_v1.txt", 3);
    PatternMatcher::Load("pat5.txt", 5);

    Args::Parse(argc, argv);

    // Start listening for commands.
    CommsHandler handler;
    std::string message;
    while (std::getline(std::cin, message) && handler.Process(message));

    PatternMatcher::CleanUp();

    LURIEN_STOP

    return 0;
}
