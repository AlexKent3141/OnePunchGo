#include "Board.h"
#include "Move.h"
#include "Tests/TestRunner.h"
#include "Tests/MakeMoveTest.h"
#include "Tests/KoDetectionTest.h"
#include "Tests/TsumegoTest.h"

int main()
{
    TestRunner runner;
    runner.RunTests<MakeMoveTest>();
    runner.RunTests<KoDetectionTest>();
    runner.RunTests<TsumegoTest>();

    return 0;
}
