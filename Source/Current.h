#include "Search/Search.h"
#include "Search/Selection/MCRave.h"
#include "Search/Playout/BiasedBestOf.h"

// This typedef defines the best search type found so far.
typedef Search<MCRave, BiasedBestOf<4>> CurrentSearch;
