#include "moves_container.hpp"

#include <unordered_map>

#include "constants.hpp"
#include "hashmap_entry.hpp"


MovesMap::MovesMap() : MovesHashtable(HASHMAP_INITIAL_LEVEL) {}

void MovesMap::apply_decay_factor() {
    for (uint i = 1; i < buckets.size(); i++) {
        buckets[i].total_score.sum *= DECAY_FACTOR;
        buckets[i].total_score.weight *= DECAY_FACTOR;
    }
    // std::cout << "load factor " << (double)buckets.size()/capacity << "\n";
}
