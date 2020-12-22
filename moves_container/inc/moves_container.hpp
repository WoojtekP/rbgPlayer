#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include "moves_hashmap.hpp"
#include "hashmap_entry.hpp"
#include "move_hash.hpp"


typedef MovesHashmap<HashmapEntry, move_hash<HashmapEntry>> MovesContainer;


#endif
