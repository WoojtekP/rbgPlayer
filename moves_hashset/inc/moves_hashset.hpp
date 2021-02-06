#ifndef MOVESHASHSET
#define MOVESHASHSET

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
    using MovesHashtable<E, H>::find_or_insert;
    using MovesHashtable<E, H>::find_or_get_default;
public:
    MovesHashset() : MovesHashtable<E, H>(HASHSET_INITIAL_LEVEL) {
        buckets.front().inserted = false;
    }
    MovesHashset(const MovesHashset&) = delete;
    void operator=(const MovesHashset&) = delete;
    void operator=(const MovesHashset&&) = delete;

    int insert(const decltype(E::move)& move, const int context = 0) {
        const auto index = find_or_insert(move, context);
        buckets[index].inserted = true;
        return index;
    }

    template <typename T>
    bool find(const T& move, const int context = 0) {
        const auto index = find_or_get_default(move, context);
        return buckets[index].inserted;
    }

    // TODO: use enable_if
    #if RAVE >= 2
    bool find(const reasoner::move& move, const int context = 0) {
        int bucket_id = context;
        for (const auto action : move.mr) {
            bucket_id = find_or_get_default(action, bucket_id);
            if (bucket_id == 0) {
                return false;
            }
        }
        return buckets[bucket_id].inserted;
    }
    #endif

    void reset() {
        for (uint i = 0; i < capacity; i++) {
            hashtable[i] = 0;
        }
        buckets.resize(1);
    }
};

#endif
