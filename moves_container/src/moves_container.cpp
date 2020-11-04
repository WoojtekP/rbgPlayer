#include "moves_container.hpp"

#include <unordered_map>

#include "constants.hpp"
#include "hashmap_entry.hpp"
#include "reasoner.hpp"


MovesContainer::MovesContainer() : MovesHashtable(HASHMAP_INITIAL_LEVEL) {}

int MovesContainer::insert_or_update(const reasoner::move& move, const uint score, const uint, const int) {
    uint hashindex = hash(move) % capacity;
    uint index = hashtable[hashindex];
    [[maybe_unused]] bool found = false;
    if (index == 0) {
        hashtable[hashindex] = buckets.size();
        buckets.emplace_back(move, score, 1.0);
    }
    else {
        uint collisions = 0;
        while (true) {
            if (buckets[index].mv == move) {
                buckets[index].scores += score;
                buckets[index].weight += 1.0;
                goto return_lb;  // TODO replace with 'return 0'
            }
            if (buckets[index].next == 0) {
                buckets[index].next = buckets.size();
                buckets.emplace_back(move, score, 1.0);
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

double MovesContainer::get_score_or_default_value(const reasoner::move& move, const int) {
    uint hashindex = move_hash()(move) % capacity;
    uint index = hashtable[hashindex];
    while (index != 0) {
        if (buckets[index].mv == move) {
            return buckets[index].scores / buckets[index].weight;
        }
        index = buckets[index].next;
    }
    return EXPECTED_MAX_SCORE;
}

void MovesContainer::apply_decay_factor() {
    for (uint i = 1; i < buckets.size(); i++) {
        buckets[i].scores *= DECAY_FACTOR;
        buckets[i].weight *= DECAY_FACTOR;
    }
    // std::cout << "load factor " << (double)buckets.size()/capacity << "\n";
}
