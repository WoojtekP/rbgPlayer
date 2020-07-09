#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include "reasoner.hpp"

class moves_container {
private:
    static constexpr uint size = reasoner::BOARD_SIZE * reasoner::NUMBER_OF_MODIFIERS;
    std::pair<double, double> map[moves_container::size];
public:
    moves_container();
    void insert_or_update(const reasoner::move_representation&, const uint, const uint);
    double get_score_or_default_value(const reasoner::move_representation&);
    void apply_decay_factor();
};

#endif
