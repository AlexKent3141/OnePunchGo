#ifndef __TEST_RUNNER_H__
#define __TEST_RUNNER_H__

#include "TestBase.h"
#include <cctype>
#include <iostream>
#include <fstream>
#include <string>

// Runs the tests.
class TestRunner
{
public:
    // Execute the tests of the specified type.
    template<typename TestType>
    void RunTests() const
    {
        static_assert(std::is_base_of<TestBase, TestType>::value, "Not a test type.");
        auto test = TestType();

        test.OneTimeSetup();

        std::string testsPath = test.TestFileName();
        
        // Parse the tests file and process the test cases.
        // Need to adjust path as we will be executing from the directory above.
        std::ifstream file("Tests/" + testsPath);
        std::string line;
        bool pass = true;
        bool inTest = false;
        std::vector<std::string> lines;
        while (std::getline(file, line) && pass)
        {
            if (IsTestStart(line))
            {
                inTest = true;
                std::cout << line << std::endl;
            }
            else if (IsTestEnd(line))
            {
                inTest = false;
                pass = test.Run(lines);
                std::cout << (pass ? "PASS" : "FAIL") << std::endl;
                lines.clear();
            }
            else if (inTest)
            {
                lines.push_back(line);
            }
        }

        test.OneTimeCleanup();
    }

private:
    bool IsTestStart(const std::string& line) const
    {
        const std::string Begin = "Begin";
        return line.compare(0, Begin.size(), Begin) == 0;
    }

    bool IsTestEnd(const std::string& line) const
    {
        const std::string End = "End";
        return line.compare(0, End.size(), End) == 0;
    }
};

#endif // __TEST_RUNNER_H__

