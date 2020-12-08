#include "moves_container.hpp"

#include <unordered_map>

#include "constants.hpp"
#include "hashmap_entry.hpp"
#include "reasoner.hpp"


MovesMap::MovesMap() : MovesHashtable(HASHMAP_INITIAL_LEVEL) {}

int MovesMap::insert_or_update(const reasoner::move& move, const uint score, const int context) {
    uint hashindex = hash(move, context) % capacity;
    uint index = hashtable[hashindex];
    [[maybe_unused]] bool found = false;
    if (index == 0) {
        hashtable[hashindex] = buckets.size();
        buckets.emplace_back(move, score, 1.0, context);
    }
    else {
        uint collisions = 0;
        while (true) {
            if (buckets[index].equals(move, context)) {
                buckets[index].total_score.sum += score;
                buckets[index].total_score.weight += 1.0;
                goto return_lb;  // TODO replace with 'return 0'
            }
            if (buckets[index].next == 0) {
                buckets[index].next = buckets.size();
                buckets.emplace_back(move, score, 1.0, context);
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
    auto it = map.find(move);
    if (it == map.end()) {
        map.insert({move, {1.0, static_cast<double>(score)}});
        if (found) {
            std::cout << "err found\n";
        }
    }
    else {
        ++(it->second.first);
        it->second.second += static_cast<double>(score);
        if (!found) std::cout << "err not found\n";
    }*/
    return 0;
}

score MovesMap::get_score_or_default_value(const reasoner::move& move, const int context = 0) {
    uint hashindex = hash(move, context) % capacity;
    uint index = hashtable[hashindex];
    while (index != 0) {
        if (buckets[index].equals(move, context)) {
            return buckets[index].total_score;
        }
        index = buckets[index].next;
    }
    return {EXPECTED_MAX_SCORE, 1.0};
}

void MovesMap::apply_decay_factor() {
    for (uint i = 1; i < buckets.size(); i++) {
        buckets[i].total_score.sum *= DECAY_FACTOR;
        buckets[i].total_score.weight *= DECAY_FACTOR;
    }
    // std::cout << "load factor " << (double)buckets.size()/capacity << "\n";
}
