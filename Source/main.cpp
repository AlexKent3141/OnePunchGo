#include "Board.h"
#include "Move.h"
#include "Tests/TestRunner.h"
#include "Tests/MakeMoveTest.h"
#include "Tests/KoDetectionTest.h"

int main()
{
    TestRunner runner;
    runner.RunTests<MakeMoveTest>();
    runner.RunTests<KoDetectionTest>();

    return 0;
}
