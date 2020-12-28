#ifndef MOVESSET
#define MOVESSET

#include "hashset_entry.hpp"
#include "move_hash.hpp"
#include "moves_hashset.hpp"
#include "rave_moves_set.hpp"


typedef RaveMovesSet<MovesHashset<HashsetEntry, move_hash<HashsetEntry>>> MovesSet;

#endif
