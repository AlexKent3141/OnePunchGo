#include "CommsHandler.h"
#include "Tests/TestRunner.h"
#include "Tests/MakeMoveTest.h"
#include "Tests/KoDetectionTest.h"
#include "Tests/PerformanceTest.h"
#include "Tests/PatternMatchTest.h"
#include "Tests/TsumegoTest.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc == 2 && std::string(argv[1]) == "test")
    {
        // Execute the unit tests.
        TestRunner runner;
        runner.RunTests<MakeMoveTest>();
        runner.RunTests<KoDetectionTest>();
        runner.RunTests<PatternMatchTest>();
        runner.RunTests<TsumegoTest>();
    }
    else if (argc == 2 && std::string(argv[1]) == "perf")
    {
        // Execute the performance tests.
        TestRunner runner;
        runner.RunTests<PerformanceTest>();
    }
    else
    {
        // Start listening for commands.
        CommsHandler handler;
        std::string message;
        while (std::getline(std::cin, message) && handler.Process(message));
    }

    return 0;
}
