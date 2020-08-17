#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include "moves_tree.hpp"


class moves_container {
private:
    MovesTree moves;
public:
    template <typename T>
    int insert_or_update(const T& move, const uint score, [[maybe_unused]] const uint depth, const int context = 0) {
        double scaled_score = static_cast<double>(score);
        double scaled_weight = 1.0;
        if constexpr (WEIGHT_SCALING) {
            scaled_score /= static_cast<double>(depth);
            scaled_weight /= static_cast<double>(depth);
        }
        return moves.insert_or_update(move, scaled_score, scaled_weight, context);
    }

    template <typename T>
    double get_score_or_default_value(const T& move, const int context = 0) {
        return moves.get_score_or_default_value(move, context);
    }

    void apply_decay_factor();
};

#endif
