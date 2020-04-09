#ifndef __SELECTION_POLICY_H__
#define __SELECTION_POLICY_H__

#include "../Node.h"
#include <cfloat>
#include <vector>
#include <functional>
#include <memory>

// A selection policy defines a method which attempts to select the most promising child node.
class SelectionPolicy
{
public:
    virtual std::shared_ptr<Node> Select(
        const std::vector<std::shared_ptr<Node>>& children) const
    {
        return children[0];
    }

    virtual ~SelectionPolicy() {}

protected:
    template<typename T>
    std::shared_ptr<T> ArgMax(
        const std::vector<std::shared_ptr<T>>& args,
        std::function<double(std::shared_ptr<T> const)> score) const
    {
        std::shared_ptr<T> best = nullptr;
        double bestVal = -DBL_MAX;
        for (std::shared_ptr<T> arg : args)
        {
            double val = score(arg);
            if (val > bestVal)
            {
                bestVal = val;
                best = arg;
            }
        }

        return best;
    }
};

#endif // __SELECTION_POLICY_H__
