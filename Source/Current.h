#include "Search/Search.h"
#include "Search/Selection/MCRave.h"
#include "Search/Playout/BestOf.h"
#include "Search/Playout/BiasedBestOf.h"

// This typedef defines the best search type found so far.
typedef Search<MCRave, BestOf<4>> CurrentSearch;
