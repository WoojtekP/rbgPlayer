#ifndef MOVESHASHSET
#define MOVESHASHSET

#include <unordered_set>

#include "constants.hpp"
#include "moves_hashtable.hpp"
#include "reasoner.hpp"


template <typename E, typename H>
class MovesHashset : public MovesHashtable<E, H> {
    using MovesHashtable<E, H>::buckets;
    using MovesHashtable<E, H>::capacity_level;
    using MovesHashtable<E, H>::capacity;
    using MovesHashtable<E, H>::hashtable;
    using MovesHashtable<E, H>::hash;
    using MovesHashtable<E, H>::grow_hashtable;
    // std::unordered_set<reasoner::move, move_hash> set;
public:
    MovesHashset() : MovesHashtable<E, H>(HASHSET_INITIAL_LEVEL) {}
    MovesHashset(const MovesHashset&) = delete;
    void operator=(const MovesHashset&) = delete;
    void operator=(const MovesHashset&&) = delete;

    template <typename T>
    int insert(const T& move, const bool inserted = true, const int context = 0) {
        uint hashindex = hash(move, context) % capacity;
        uint index = hashtable[hashindex];
        [[maybe_unused]] bool found = false;
        uint bucket_id = 0;
        if (index == 0) {
            bucket_id = hashtable[hashindex] = buckets.size();
            buckets.emplace_back(move, inserted, context);
        }
        else {
            uint collisions = 0;
            while (true) {
                if (buckets[index].equals(move, context)) {
                    bucket_id = index;
                    goto return_lb;  // TODO replace with 'return index'
                }
                if (buckets[index].next == 0) {
                    bucket_id = buckets[index].next = buckets.size();
                    buckets.emplace_back(move, inserted, context);
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
        return bucket_id;
    }

    template <typename T>
    bool find(const T& move, const int context = 0) {
        uint hashindex = hash(move, context) % capacity;
        uint index = hashtable[hashindex];
        while (index != 0) {
            if (buckets[index].equals(move, context)) {
                return buckets[index];
            }
            index = buckets[index].next;
        }
        return false;
    }

    void reset() {
        for (uint i = 0; i < capacity; i++) {
            hashtable[i] = 0;
        }
        buckets.resize(1);
        // set.clear();
    }
};

#endif
