#ifndef MOVESHASHMAP
#define MOVESHASHMAP

#include <unordered_map>

#include "constants.hpp"
#include "moves_hashtable.hpp"
#include "reasoner.hpp"


template <typename E, typename H>
class MovesHashmap : public MovesHashtable<E, H> {
    using MovesHashtable<E, H>::buckets;
    using MovesHashtable<E, H>::capacity_level;
    using MovesHashtable<E, H>::capacity;
    using MovesHashtable<E, H>::hashtable;
    using MovesHashtable<E, H>::hash;
    using MovesHashtable<E, H>::grow_hashtable;
    // std::unordered_map<reasoner::move, std::pair<double, double>, move_hash> map;
public:
    MovesHashmap() : MovesHashtable<E, H>(HASHMAP_INITIAL_LEVEL) {}
    MovesHashmap(const MovesHashmap&) = delete;
    void operator=(const MovesHashmap&) = delete;
    void operator=(const MovesHashmap&&) = delete;

    template <typename T>
    int insert_or_update(const T& move, const uint score, const int context = 0, const double weight = 1.0) {
        uint hashindex = hash(move, context) % capacity;
        uint index = hashtable[hashindex];
        [[maybe_unused]] bool found = false;
        uint bucket_id = 0;
        if (index == 0) {
            bucket_id = hashtable[hashindex] = buckets.size();
            buckets.emplace_back(move, score, 1.0, context);
        }
        else {
            uint collisions = 0;
            while (true) {
                if (buckets[index].equals(move, context)) {
                    buckets[index].total_score.sum += score;
                    buckets[index].total_score.weight += weight;
                    bucket_id = index;
                    goto return_lb;  // TODO replace with 'return index'
                }
                if (buckets[index].next == 0) {
                    bucket_id = buckets[index].next = buckets.size();
                    buckets.emplace_back(move, score, weight, context);
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
        return bucket_id;
    }

    template <typename T>
    score get_score_or_default_value(const T& move, const int context = 0) {
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

    void apply_decay_factor() {
        for (uint i = 1; i < buckets.size(); i++) {
            buckets[i].total_score.sum *= DECAY_FACTOR;
            buckets[i].total_score.weight *= DECAY_FACTOR;
        }
        // std::cout << "load factor " << (double)buckets.size()/capacity << "\n";
    }

    int get_context(const reasoner::action_representation action, const int context = 0) {
        return insert_or_update(action, 0, context, 0.0);
    }
};

#endif
