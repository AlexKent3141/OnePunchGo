#include "Args.h"
#include "CommsHandler.h"
#include "Tests/TestRunner.h"
#include "Tests/MakeMoveTest.h"
#include "Tests/KoDetectionTest.h"
#include "Tests/PerformanceTest.h"
#include "Tests/TsumegoTest.h"
#include "Tests/ExperimentTest.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    auto args = Args::Parse(argc, argv);
    if (args->HasArg("-test"))
    {
        // Execute the unit tests.
        TestRunner runner;
        runner.RunTests<MakeMoveTest>();
        runner.RunTests<KoDetectionTest>();
        runner.RunTests<TsumegoTest>();
    }
    if (args->HasArg("-experiment"))
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

    return 0;
}
