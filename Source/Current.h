#include "Search/Search.h"
#include "Search/Selection/UCBPriors.h"
#include "Search/Playout/BiasedBestOf.h"

// This typedef defines the best search type found so far.
typedef Search<UCBPriors, BiasedBestOf<4>> CurrentSearch;
