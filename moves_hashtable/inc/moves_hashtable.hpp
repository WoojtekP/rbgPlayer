#ifndef MOVESHASHTABLE
#define MOVESHASHTABLE

#include <unordered_map>
#include <vector>

#include "hash_entry.hpp"
#include "reasoner.hpp"


constexpr uint HASH_PRIMES[] = {23U,53U,97U,193U,389U,769U,1543U,3079U,6151U,12289U,24593U,49157U,98317U,196613U,393241U,786433U,1572869U,3145739U,6291469U,12582917U,25165843U,50331653U,100663319U,201326611U,402653189U,805306457U,1610612741U,3221225827U};

struct move_hash {
    std::size_t operator()(const reasoner::move& move) const noexcept;
};

class MovesHashtable {
protected:
    uint capacity_level;
    uint capacity;
    uint *hashtable;
    std::vector<Entry> buckets;

    void grow_hashtable();
public:
    static long unsigned int hash(const reasoner::move &move) noexcept;

    MovesHashtable(const uint);
    MovesHashtable() = delete;
    MovesHashtable(const MovesHashtable&) = delete;
    void operator=(const MovesHashtable&) = delete;
    void operator=(const MovesHashtable&&) = delete;
    ~MovesHashtable();
};

#endif
