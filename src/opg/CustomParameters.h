#ifndef __CUSTOM_PARAMETERS_H__
#define __CUSTOM_PARAMETERS_H__

#include <vector>

// This singleton class contains any defined custom parameters to use within the AI.
// This is useful when automatically tuning parameters.
class CustomParameters
{
public:
    static CustomParameters* GetInstance()
    {
        if (_instance == nullptr)
        {
            _instance = new CustomParameters();
        }

        return _instance;
    }

    void AddParameter(double param)
    {
        _parameters.push_back(param);
    }

    double GetParameter(int i) const
    {
        return _parameters[i];
    }

private:
    static CustomParameters* _instance;

    std::vector<double> _parameters;

    CustomParameters()
    {
    }
};

#endif // __CUSTOM_PARAMETERS_H__
