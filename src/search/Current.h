#include "Search.h"
#include "Selection/MCRavePriors.h"
#include "Playout/BiasedBestOf.h"

// This typedef defines the best search type found so far.
typedef Search<MCRavePriors, BiasedBestOf<4>> CurrentSearch;
