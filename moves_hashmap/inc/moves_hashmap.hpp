#ifndef MOVESHASHMAP
#define MOVESHASHMAP

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
    using MovesHashtable<E, H>::find_or_insert;
    using MovesHashtable<E, H>::find_or_get_default;

public:
    MovesHashmap() : MovesHashtable<E, H>(HASHMAP_INITIAL_LEVEL) {
        buckets.front().total_score = {EXPECTED_MAX_SCORE, 1.0};
    }
    MovesHashmap(const MovesHashmap&) = delete;
    void operator=(const MovesHashmap&) = delete;
    void operator=(const MovesHashmap&&) = delete;

    template <typename T>
    int insert_or_update(const T& move, const uint score, const int context = 0) {
        const auto index = find_or_insert(move, context);
        buckets[index].total_score.sum += score;
        buckets[index].total_score.weight += 1.0;
        #if MAST >= 2
        if (move.index > 0) {
            return reasoner::is_switch(move.index) ? 0 : index;
        }
        return context;
        #endif
        return 0;
    }

    #if MAST >= 2 && defined(ORTHODOX_TREE)
    int insert_or_update(const reasoner::move& move, const uint score, const int context = 0) {
        assert(context == 0);
        int bucket_id = context;
        for (const auto action : move.mr) {
            bucket_id = find_or_insert(action, bucket_id);
            buckets[bucket_id].total_score.sum += score;
            buckets[bucket_id].total_score.weight += 1.0;
        }
        return 0;
    }
    #endif

    template <typename T>
    score get_score_or_default_value(const T& move, const int context = 0) {
        static const score default_score(EXPECTED_MAX_SCORE, 1.0);
        const auto index = find_or_get_default(move, context);
        return buckets[index].total_score.weight == 0.0 ? default_score : buckets[index].total_score;
    }

    #if MAST >= 2 && defined(ORTHODOX_TREE)
    score get_score_or_default_value(const reasoner::move& move, const int context = 0) {
        static const score default_score(EXPECTED_MAX_SCORE, 1.0);
        assert(context == 0);
        int bucket_id = context;
        for (const auto action : move.mr) {
            bucket_id = find_or_get_default(action, bucket_id);
            if (bucket_id == 0) {
                return default_score;
            }
        }
        return buckets[bucket_id].total_score.weight == 0 ? default_score : buckets[bucket_id].total_score;
    }
    #endif

    void apply_decay_factor() {
        for (uint i = 1; i < buckets.size(); i++) {
            buckets[i].total_score.sum *= DECAY_FACTOR;
            buckets[i].total_score.weight *= DECAY_FACTOR;
        }
    }
};

#endif
