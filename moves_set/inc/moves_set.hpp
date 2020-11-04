#ifndef MOVESSET
#define MOVESSET

#include <unordered_set>

#include "hashset_entry.hpp"
#include "moves_hashtable.hpp"
#include "reasoner.hpp"


class MovesSet : public MovesHashtable<HashsetEntry> {
private:
    // std::unordered_set<reasoner::move, move_hash> set;
public:
    MovesSet();
    MovesSet(const MovesSet&) = delete;
    void operator=(const MovesSet&) = delete;
    void operator=(const MovesSet&&) = delete;

    void insert(const reasoner::move& move);
    bool find(const reasoner::move& move);
    void reset();
};

#endif
