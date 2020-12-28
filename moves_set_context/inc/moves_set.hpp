#ifndef MOVESSET
#define MOVESSET

#include "hashset_context_entry.hpp"
#include "moves_hashset.hpp"
#include "move_hash_context.hpp"
#include "rave_moves_set.hpp"


typedef RaveMovesSet<MovesHashset<HashsetContextEntry, move_hash_context<HashsetContextEntry>>> MovesSet;

#endif
