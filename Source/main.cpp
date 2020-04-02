#include "Args.h"
#include "CommsHandler.h"
#include "Patterns/PatternMatcher.h"
#include "Tests/TestRunner.h"
#include "Tests/MakeMoveTest.h"
#include "Tests/KoDetectionTest.h"
#include "Tests/PerformanceTest.h"
#include "Tests/PatternMatchTest.h"
#include "Tests/TsumegoTest.h"
#include "Tests/ExperimentTest.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    PatternMatcher::Load("pat3_v1.txt", 3);
    PatternMatcher::Load("pat5.txt", 5);

    auto args = Args::Parse(argc, argv);
    if (args->HasArg("-test"))
    {
        // Execute the unit tests.
        TestRunner runner;
        runner.RunTests<MakeMoveTest>();
        runner.RunTests<KoDetectionTest>();
        runner.RunTests<PatternMatchTest>();
        runner.RunTests<TsumegoTest>();
    }
    else if (args->HasArg("-experiment"))
    {
        // Execute the experimental tests.
        TestRunner runner;
        runner.RunTests<ExperimentTest>();
    }
    else if (args->HasArg("-perf"))
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

    PatternMatcher::CleanUp();

    return 0;
}
