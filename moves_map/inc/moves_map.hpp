#ifndef MOVESMAP
#define MOVESMAP

#include <unordered_map>

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

    int insert_or_update(const reasoner::move& move, const uint score, const int);
    double get_score_or_default_value(const reasoner::move& move, const int);
    void apply_decay_factor();
};

#endif
