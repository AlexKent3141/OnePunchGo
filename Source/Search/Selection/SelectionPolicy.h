#ifndef __SELECTION_POLICY_H__
#define __SELECTION_POLICY_H__

#include "../Node.h"
#include "../../NeuralNet.h"
#include <cfloat>
#include <vector>
#include <functional>

// A selection policy defines a method which attempts to select the most promising child node.
class SelectionPolicy
{
public:
    virtual Node* Select(const Board& board, const std::vector<Node*>& children, NeuralNet* net) const
    {
        return children[0];
    }

    virtual ~SelectionPolicy() {}

protected:
    template<typename T>
    T* ArgMax(const std::vector<T*>& args, std::function<double(T* const)> score) const
    {
        T* best = nullptr;
        double bestVal = -DBL_MAX;
        for (T* arg : args)
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
