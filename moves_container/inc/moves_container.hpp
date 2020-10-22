#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include <unordered_map>


#include "reasoner.hpp"


struct move_hash {
    std::size_t operator()(const reasoner::move& move) const noexcept;
};

class MovesContainer {
private:
    std::unordered_map<reasoner::move, std::pair<double, double>, move_hash> map;
public:
    int insert_or_update(const reasoner::move& move, const uint score, const uint, const int);
    double get_score_or_default_value(const reasoner::move& move, const int);
    void apply_decay_factor();
};

#endif
