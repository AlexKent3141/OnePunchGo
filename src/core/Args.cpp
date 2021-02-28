#include "Args.h"

Args* Args::_instance = nullptr;

// Parse arguments which have integer values.
template<>
bool Args::TryParse<int>(const std::string& argName, int& val) const
{
    bool success = false;
    if (HasArg(argName))
    {
        const std::string& vals = _args.at(argName);
        val = stoi(vals);
        success = true;
    }

    return success;
}
