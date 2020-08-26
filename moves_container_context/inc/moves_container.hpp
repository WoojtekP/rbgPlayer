#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include <cmath>

#include "actions_arrays.hpp"
#include "constants.hpp"
#include "moves_tree.hpp"


class moves_container {
private:
    MovesTree moves;
    actions_arrays arr;
public:
    template <typename T>
    int insert_or_update(const T& move, const uint score, [[maybe_unused]] const uint depth, const int context = 0) {
        double scaled_score = static_cast<double>(score);
        double scaled_weight = 1.0;
        if constexpr (WEIGHT_SCALING) {
            scaled_score /= static_cast<double>(depth);
            scaled_weight /= static_cast<double>(depth);
        }
        if constexpr (MAST_EQUIVALENCE_PARAMETER > 0) {
            arr.insert_or_update(move, scaled_score, scaled_weight);
        }
        return moves.insert_or_update(move, scaled_score, scaled_weight, context);
    }

    template <typename T>
    double get_score_or_default_value(const T& move, const int context = 0) {
        auto context_score = moves.get_score_or_default_value(move, context);
        if constexpr (MAST_EQUIVALENCE_PARAMETER == 0) {
            return context_score.get_score();
        }
        double split_score = arr.get_score_or_default_value(move);
        double beta = std::sqrt(MAST_EQUIVALENCE_PARAMETER / (3.0 * context_score.weight  + MAST_EQUIVALENCE_PARAMETER));
        return beta * split_score + (1.0 - beta) * context_score.get_score();
    }

    void apply_decay_factor();
    int get_context(const reasoner::move_representation&, const int);
};

#endif
