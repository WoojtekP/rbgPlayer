#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include "reasoner.hpp"

class moves_container {
private:
    static constexpr uint size = reasoner::BOARD_SIZE * reasoner::NUMBER_OF_MODIFIERS;
    std::pair<double, double> map[moves_container::size];
    void insert_or_update(const reasoner::move_representation&, const uint, const uint);
    double get_score_or_default_value(const reasoner::move_representation&);
public:
    moves_container();
    template <typename T>
    void insert_or_update(const T& move, const uint score, [[maybe_unused]] const uint depth) {
        insert_or_update(move.mr, score, depth);
    }

    template <typename T>
    double get_score_or_default_value(const T& move) {
        return get_score_or_default_value(move.mr);
    }
    void apply_decay_factor();
};

#endif
