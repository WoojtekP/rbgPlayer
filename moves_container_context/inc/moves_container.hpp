#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include "moves_hashmap.hpp"
#include "hashmap_context_entry.hpp"
#include "move_hash_context.hpp"


typedef MovesHashmap<HashmapContextEntry, move_hash_context<HashmapContextEntry>> MovesContainer;


#endif
