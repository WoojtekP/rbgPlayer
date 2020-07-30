#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include <unordered_map>

#include "reasoner.hpp"

struct move_hash {
    std::size_t operator()(const reasoner::move_representation&) const noexcept;
};

class moves_container {
private:
    std::unordered_map<reasoner::move_representation, std::pair<double, double>, move_hash> map;
public:
    void insert_or_update(const reasoner::move_representation&, const uint, const uint);
    double get_score_or_default_value(const reasoner::move_representation&);
    void apply_decay_factor();
};

#endif
