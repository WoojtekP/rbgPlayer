#ifndef MOVESHASHTABLE
#define MOVESHASHTABLE

#include <unordered_map>
#include <vector>

#include "reasoner.hpp"


namespace {
constexpr uint HASH_PRIMES[] = {23U,53U,97U,193U,389U,769U,1543U,3079U,6151U,12289U,24593U,49157U,98317U,196613U,393241U,786433U,1572869U,3145739U,6291469U,12582917U,25165843U,50331653U,100663319U,201326611U,402653189U,805306457U,1610612741U,3221225827U};
}  // namespace


template <typename E, typename H>
class MovesHashtable {
protected:
    std::vector<E> buckets;
    uint capacity_level;
    uint capacity;
    uint *hashtable;
    H hash;

    void grow_hashtable() {
        // std::cout << "growing hashtable from " << capacity << " to " << HASH_PRIMES[capacity_level+1] << ", size " << buckets.size() << "\n";
        delete[] hashtable;
        capacity = HASH_PRIMES[++capacity_level];
        hashtable = new uint[capacity];
        for (uint i = 0; i < capacity; i++) {
            hashtable[i] = 0;
        }
        for (uint i = 1; i < buckets.size(); i++) {
            buckets[i].next = 0;
            uint hashindex = hash(buckets[i]) % capacity;
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

    uint find_or_insert(const decltype(E::move)& move, const int context = 0) {
        uint hashindex = hash(move, context) % capacity;
        uint index = hashtable[hashindex];
        if (index == 0) {
            hashtable[hashindex] = buckets.size();
            buckets.emplace_back(move, context);
        }
        else {
            while (true) {
                if (buckets[index].equals(move, context)) {
                    return index;
                }
                if (buckets[index].next == 0) {
                    buckets[index].next = buckets.size();
                    buckets.emplace_back(move, context);
                    break;
                }
                index = buckets[index].next;
            }
        }
        if (buckets.size() * HASH_OVERLOAD_FACTOR >= capacity) {
            grow_hashtable();
        }
        return buckets.size() - 1;
    }

    uint find_or_get_default(const decltype(E::move)& move, const int context = 0) {
        uint hashindex = hash(move, context) % capacity;
        uint index = hashtable[hashindex];
        while (index != 0) {
            if (buckets[index].equals(move, context)) {
                return index;
            }
            index = buckets[index].next;
        }
        return 0;
    }

public:
    MovesHashtable(const uint capacity_lvl)
        : capacity_level(capacity_lvl)
        , capacity(HASH_PRIMES[capacity_level])
        , hashtable(new uint[capacity]) {
        for (uint i = 0; i < capacity; i++) {
            hashtable[i] = 0;
        }
        buckets.emplace_back();
    }
    MovesHashtable() = delete;
    MovesHashtable(const MovesHashtable&) = delete;
    void operator=(const MovesHashtable&) = delete;
    void operator=(const MovesHashtable&&) = delete;
    ~MovesHashtable() {
        delete [] hashtable;
    }

    uint get_context(const reasoner::action_representation action, const int context) {
        if (action.index > 0) {
            return reasoner::is_switch(action.index) ? 0 : find_or_insert(action, context);
        }
        return context;
    }
};

#endif
