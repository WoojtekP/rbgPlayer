#ifndef MOVESHASHSET
#define MOVESHASHSET

#include <unordered_set>

#include "hashset_entry.hpp"
#include "moves_hashtable.hpp"
#include "reasoner.hpp"


class MovesHashset : public MovesHashtable<HashsetEntry> {
private:
    // std::unordered_set<reasoner::move, move_hash> set;
public:
    MovesHashset();
    MovesHashset(const MovesHashset&) = delete;
    void operator=(const MovesHashset&) = delete;
    void operator=(const MovesHashset&&) = delete;

    void insert(const reasoner::move& move);
    bool find(const reasoner::move& move);
    void reset();
};

#endif
