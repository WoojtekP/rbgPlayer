#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include <unordered_map>

#include "moves_hashtable.hpp"
#include "reasoner.hpp"


class MovesContainer : public MovesHashtable {
private:
    // std::unordered_map<reasoner::move, std::pair<double, double>, move_hash> map;
public:
    MovesContainer();
    MovesContainer(const MovesContainer&) = delete;
    void operator=(const MovesContainer&) = delete;
    void operator=(const MovesContainer&&) = delete;

    int insert_or_update(const reasoner::move& move, const uint score, const uint, const int);
    double get_score_or_default_value(const reasoner::move& move, const int);
    void apply_decay_factor();
};

#endif
