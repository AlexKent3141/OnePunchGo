#include "core/Args.h"
#include "patterns/PatternMatcher.h"
#include "TestRunner.h"
#include "MakeMoveTest.h"
#include "KoDetectionTest.h"
#include "PerformanceTest.h"
#include "PatternMatchTest.h"
#include "TsumegoTest.h"
#include "ExperimentTest.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    PatternMatcher::Load("pat3_v1.txt", 3);
    PatternMatcher::Load("pat5.txt", 5);

    auto args = Args::Parse(argc, argv);
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
        // Execute the unit tests.
        TestRunner runner;
        runner.RunTests<MakeMoveTest>();
        runner.RunTests<KoDetectionTest>();
        runner.RunTests<PatternMatchTest>();
        runner.RunTests<TsumegoTest>();
    }

    PatternMatcher::CleanUp();

    return 0;
}
