#ifndef __ARGS_H__
#define __ARGS_H__

#include <cassert>
#include <map>
#include <string>

// This singleton class parses the command line arguments and allows them to be accessed
// elsewhere within the program.
class Args
{
public:
    // Parse the command line arguments.
    static Args* Parse(int argc, char* argv[])
    {
        if (_instance == nullptr)
        {
            _instance = new Args(argc, argv);
        }

        return _instance;
    }

    // Access the instance.
    static Args* Get()
    {
        return _instance;
    }

    // Check whether the argument is present.
    bool HasArg(const std::string& argName) const
    {
        return _args.find(argName) != _args.end();
    }

    // Parse the specified argument's value.
    template<typename T>
    bool TryParse(const std::string& argName, T& val) const;

private:
    static Args* _instance;

    // I am using the constraint that argument types must start with a '-'.
    const char ArgStart = '-';

    std::map<std::string, std::string> _args;

    // Construct the instance and parse the arguments.
    Args(int argc, char* argv[])
    {
        // Skip the first argument which is just the executable name.
        for (int i = 1; i < argc; i++)
        {
            auto arg = std::string(argv[i]);
            std::string val;
            assert(IsArgName(arg));

            // Is the next argument a value associated to this arg?
            if (i < argc - 1)
            {
                auto next = std::string(argv[i+1]);
                if (!IsArgName(next))
                {
                    val = next;
                    ++i;
                }
            }

            _args.insert({arg, val});
        }
    }

    bool IsArgName(const std::string& arg)
    {
        return arg.size() > 0 && arg[0] == ArgStart;
    }
};

#endif // __ARGS_H__
