#include "Search/Search.h"
#include "Search/Selection/MCRavePriors.h"
#include "Search/Playout/BiasedBestOf.h"

// This typedef defines the best search type found so far.
typedef Search<MCRavePriors, BiasedBestOf<4>> CurrentSearch;
