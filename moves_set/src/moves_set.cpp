#include "moves_set.hpp"

#include <unordered_set>

#include "constants.hpp"
#include "hash_entry.hpp"
#include "reasoner.hpp"


MovesSet::MovesSet() : MovesHashtable(HASHSET_INITIAL_LEVEL) {}

void MovesSet::insert(const reasoner::move& move) {
    uint hashindex = hash(move) % capacity;
    uint index = hashtable[hashindex];
    [[maybe_unused]] bool found = false;
    if (index == 0) {
        hashtable[hashindex] = buckets.size();
        buckets.emplace_back(move);
    }
    else {
        uint collisions = 0;
        while (true) {
            if (buckets[index].mv == move) {
                goto return_lb;  // TODO replace with 'return true'
            }
            if (buckets[index].next == 0) {
                buckets[index].next = buckets.size();
                buckets.emplace_back(move);
                break;
            }
            index = buckets[index].next;
            collisions++;
        }
    }
    if (buckets.size() * HASH_OVERLOAD_FACTOR >= capacity) {
        grow_hashtable();
    }
    return_lb:
    /*
    // Uncomment to test
    auto it = set.find(move);
    if (it == set.end()) {
        set.insert(move);
        if (found) {
            std::cout << "err found\n";
        }
    }
    else {
        if (!found) std::cout << "err not found\n";
    }*/
    return;
}

bool MovesSet::find(const reasoner::move& move) {
    uint hashindex = move_hash()(move) % capacity;
    uint index = hashtable[hashindex];
    while (index != 0) {
        if (buckets[index].mv == move) {
            return true;
        }
        index = buckets[index].next;
    }
    return false;
}

void MovesSet::reset() {
    for (uint i = 0; i < capacity; i++) {
        hashtable[i] = 0;
    }
    buckets.resize(1);
    // set.clear();
}
