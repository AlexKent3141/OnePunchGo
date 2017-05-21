#ifndef __SELECTION_POLICY_H__
#define __SELECTION_POLICY_H__

#include "Node.h"
#include <vector>

// A selection policy defines a method which attempts to select the most promising child node.
class SelectionPolicy
{
public:
    virtual Node* Select(const std::vector<Node*>& children) const
    {
        return children[0];
    }

    virtual ~SelectionPolicy() {}
};

#endif // __SELECTION_POLICY_H__
