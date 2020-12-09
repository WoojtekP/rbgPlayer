#ifndef MOVESMAP
#define MOVESMAP

#include <unordered_map>

#include "constants.hpp"
#include "hashmap_entry.hpp"
#include "moves_hashtable.hpp"
#include "reasoner.hpp"


class MovesMap : public MovesHashtable<HashmapEntry> {
private:
    // std::unordered_map<reasoner::move, std::pair<double, double>, move_hash> map;
public:
    MovesMap();
    MovesMap(const MovesMap&) = delete;
    void operator=(const MovesMap&) = delete;
    void operator=(const MovesMap&&) = delete;

    template <typename T>
    int insert_or_update(const T& move, const uint score, const int context) {
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

    template <typename T>
    score get_score_or_default_value(const T& move, const int context) {
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

    void apply_decay_factor();
};

#endif
