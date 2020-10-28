#include "moves_container.hpp"

#include <unordered_map>

#include "constants.hpp"

#include <iostream>

void MovesContainer::grow_hashtable() {
    //std::cout << "growing hashtable from " << capacity << " to " << HASH_PRIMES[capacity_level+1] << ", size " << buckets.size() << "\n";
    delete[] hashtable;
    capacity = HASH_PRIMES[++capacity_level];
    hashtable = new uint[capacity];
    for (uint i = 0; i < capacity; i++) hashtable[i] = 0;
    for (uint i = 1; i < buckets.size(); i++) {
        buckets[i].next = 0;
        uint hashindex = hash(buckets[i].mv) % capacity;
        uint index = hashtable[hashindex];
        if (index == 0) {
            hashtable[hashindex] = i;
        } else {
            while (buckets[index].next) index = buckets[index].next;
            buckets[index].next = i;
        }
    }
}

std::size_t move_hash::operator()(const reasoner::move& move) const noexcept {
    return MovesContainer::hash(move);
}

long unsigned int MovesContainer::hash(const reasoner::move& move) noexcept {
    long unsigned int hash = 0;
    for (const auto& action : move.mr) {
       hash = (hash * 13 + action.index) * 17 + action.cell;
    }
    return hash;
}

int MovesContainer::insert_or_update(const reasoner::move& move, const uint score, const uint, const int) {
    uint hashindex = hash(move) % capacity;
    uint index = hashtable[hashindex];
    bool found = false;
    if (index == 0) {
        hashtable[hashindex] = buckets.size();
        buckets.emplace_back(move, score, 1.0);
    } else {
        uint collisions = 0;
        while (true) {
            if (buckets[index].mv == move) {
                buckets[index].scores += score;
                buckets[index].weight += 1.0;
                found = true;
                break;
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
    if (buckets.size()*HASH_OVERLOAD_FACTOR >= capacity) grow_hashtable();
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
    bool found;
    while (index != 0) {
        if (buckets[index].mv == move)
            return buckets[index].scores / buckets[index].weight;
        index = buckets[index].next;
    }
    return EXPECTED_MAX_SCORE;
}

void MovesContainer::apply_decay_factor() {
    for (uint i = 1; i < buckets.size(); i++) {
        buckets[i].scores *= DECAY_FACTOR;
        buckets[i].weight *= DECAY_FACTOR;
    }
    //std::cout << "load factor " << (double)buckets.size()/capacity << "\n";
}
