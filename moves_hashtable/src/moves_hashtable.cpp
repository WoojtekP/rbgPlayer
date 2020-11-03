#include "moves_hashtable.hpp"

#include <unordered_map>

#include "constants.hpp"
#include "reasoner.hpp"


std::size_t move_hash::operator()(const reasoner::move& move) const noexcept {
    return MovesHashtable::hash(move);
}

MovesHashtable::MovesHashtable(const uint capacity_lvl)
    : capacity_level(capacity_lvl)
    , capacity(HASH_PRIMES[capacity_level])
    , hashtable(new uint[capacity]) {
    for (uint i = 0; i < capacity; i++) {
        hashtable[i] = 0;
    }
    buckets.emplace_back();
}

MovesHashtable::~MovesHashtable() {
    delete [] hashtable;
}

void MovesHashtable::grow_hashtable() {
    // std::cout << "growing hashtable from " << capacity << " to " << HASH_PRIMES[capacity_level+1] << ", size " << buckets.size() << "\n";
    delete[] hashtable;
    capacity = HASH_PRIMES[++capacity_level];
    hashtable = new uint[capacity];
    for (uint i = 0; i < capacity; i++) {
        hashtable[i] = 0;
    }
    for (uint i = 1; i < buckets.size(); i++) {
        buckets[i].next = 0;
        uint hashindex = hash(buckets[i].mv) % capacity;
        uint index = hashtable[hashindex];
        if (index == 0) {
            hashtable[hashindex] = i;
        }
        else {
            while (buckets[index].next) {
                index = buckets[index].next;
            }
            buckets[index].next = i;
        }
    }
}

long unsigned int MovesHashtable::hash(const reasoner::move& move) noexcept {
    long unsigned int hash = 0;
    for (const auto& action : move.mr) {
       hash = (hash * 13 + action.index) * 17 + action.cell;
    }
    return hash;
}
