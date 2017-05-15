#include "Board.h"
#include "Move.h"
#include "Tests/TestRunner.h"
#include "Tests/MakeMoveTest.h"

int main()
{
    TestRunner runner;
    runner.RunTests<MakeMoveTest>();

    return 0;
}
