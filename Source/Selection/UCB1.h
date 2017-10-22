#ifndef __UCB1_SELECTION_POLICY_H__
#define __UCB1_SELECTION_POLICY_H__

#include "UCB.h"

// The classical UCB1 version from the literature.
// In practice this doesn't seem to be the best exploration factor.
class UCB1 : public UCB<100>
{
};

#endif // __UCB1_SELECTION_POLICY_H__
