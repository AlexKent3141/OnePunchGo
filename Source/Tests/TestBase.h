#ifndef __TEST_BASE_H__
#define __TEST_BASE_H__

#include <vector>
#include <string>

// Base class for test types.
class TestBase
{
public:
    // Do any required one time setup for this test type.
    virtual void OneTimeSetup() {}

    // Do any required one time cleanup for this test type.
    virtual void OneTimeCleanup() {}

    // Get the source of the test cases.
    virtual std::string TestFileName() const = 0;

    // Execute test based on the given lines.
    virtual bool Run(const std::vector<std::string>&) = 0;
};

#endif // __TEST_BASE_H__

