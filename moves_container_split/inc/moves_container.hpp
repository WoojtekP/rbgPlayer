#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include "actions_arrays.hpp"


class MovesContainer {
private:
    ActionsArrays arr;
public:
    template <typename T>
    int insert_or_update(const T& move, const uint score, [[maybe_unused]] const uint depth, const int) {
        double scaled_score = static_cast<double>(score);
        double scaled_weight = 1.0;
        if constexpr (WEIGHT_SCALING) {
            scaled_score /= static_cast<double>(depth);
            scaled_weight /= static_cast<double>(depth);
        }
        return arr.insert_or_update(move, scaled_score, scaled_weight);
    }

    template <typename T>
    double get_score_or_default_value(const T& move, const int) {
        return arr.get_score_or_default_value(move);
    }

    void apply_decay_factor();
};

#endif
