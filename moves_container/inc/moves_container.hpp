#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include <unordered_map>
#include "reasoner.hpp"

constexpr uint HASH_PRIMES[] = {23U,53U,97U,193U,389U,769U,1543U,3079U,6151U,12289U,24593U,49157U,98317U,196613U,393241U,786433U,1572869U,3145739U,6291469U,12582917U,25165843U,50331653U,100663319U,201326611U,402653189U,805306457U,1610612741U,3221225827U};
constexpr uint HASH_INITIAL_LEVEL = 5;
constexpr uint HASH_OVERLOAD_FACTOR = 2;

struct move_hash {
    std::size_t operator()(const reasoner::move& move) const noexcept;
};

struct Entry {
    reasoner::move mv;
    double scores, weight;
    uint next;
    Entry(const reasoner::move _mv, double _scores, double _weight): mv(_mv), scores(_scores), weight(_weight), next(0) {}
};

class MovesContainer {
private:
    std::unordered_map<reasoner::move, std::pair<double, double>, move_hash> map;
    
    uint capacity_level, capacity;
    uint *hashtable;
    std::vector<Entry> buckets;
    
    bool insert_or_update(const reasoner::move &move, const double score, const double weight);
    void grow_hashtable();
    
   
public:
    static long unsigned int hash(const reasoner::move &move) noexcept;

    MovesContainer():
        capacity_level(HASH_INITIAL_LEVEL), capacity(HASH_PRIMES[capacity_level]),
        hashtable(new uint[capacity]) {
        for (uint i = 0; i < capacity; i++) hashtable[i] = 0;
        buckets.emplace_back(reasoner::move(), 0.0, 0.0);
    }

    MovesContainer(const MovesContainer&) = delete;
    void operator=(const MovesContainer&) = delete;
    void operator=(const MovesContainer&&) = delete;
    
    int insert_or_update(const reasoner::move& move, const uint score, const uint, const int);
    double get_score_or_default_value(const reasoner::move& move, const int);
    void apply_decay_factor();
};

#endif
