#ifndef __UTILS_H__
#define __UTILS_H__

#include <vector>
#include <sstream>
#include <string>

// The Utils class implements the singleton pattern.
class Utils
{
public:
    // Split by character.
	std::vector<std::string> Split(
        const std::string& s,
        char delim) const
    {
        auto elems = std::vector<std::string>();
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim))
        {
            elems.push_back(item);
        }

        return elems;
    }

    // Split by characters.
	std::vector<std::string> Split(
        const std::string& s,
        const std::string& delims) const
    {
        auto elems = std::vector<std::string>();

        size_t prev = 0, pos;
        while ((pos = s.find_first_of(delims, prev)) != std::string::npos)
        {
            if (pos > prev)
                elems.push_back(s.substr(prev, pos-prev));
            prev = pos + 1;
        }

        if (prev < s.length())
            elems.push_back(s.substr(prev, std::string::npos));

        return elems;
    }

    // Convert the string to lowercase.
    std::string ToLower(
        const std::string& s) const
    {
        std::string lower;
        for (size_t i = 0; i < s.size(); i++)
            lower += tolower(s[i]);

        return lower;
    }
};

#endif // __UTILS_H__
