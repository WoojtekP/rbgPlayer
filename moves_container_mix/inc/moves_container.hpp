#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include <cmath>

#include "actions_map.hpp"
#include "constants.hpp"
#include "moves_tree.hpp"


class MovesContainer {
private:
    MovesTree moves;
    ActionsMap arr;
public:
    template <typename T>
    int insert_or_update(const T& move, const uint score, const int context = 0) {
        if constexpr (MAST_EQUIVALENCE_PARAMETER > 0) {
            arr.insert_or_update(move, static_cast<double>(score), 1.0);
        }
        return moves.insert_or_update(move, static_cast<double>(score), 1.0, context);
    }

    template <typename T>
    double get_score_or_default_value(const T& move, const int context = 0) {
        auto context_score = moves.get_score_or_default_value(move, context);
        if constexpr (MAST_EQUIVALENCE_PARAMETER == 0) {
            return context_score.get_score();
        }
        if (context_score.weight < MAST_EQUIVALENCE_PARAMETER)
            return arr.get_score_or_default_value(move);
        else
            return context_score.get_score();
        /*
        // Smooth mix
        double split_score = arr.get_score_or_default_value(move);
        double beta = std::sqrt(MAST_EQUIVALENCE_PARAMETER / (3.0 * context_score.weight  + MAST_EQUIVALENCE_PARAMETER)); // Normal
        return beta * split_score + (1.0 - beta) * context_score.get_score();
        */
    }

    void apply_decay_factor();
    int get_context(const reasoner::move_representation&, const int);
};

#endif
