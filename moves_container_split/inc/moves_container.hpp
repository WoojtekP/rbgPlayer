#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include "actions_arrays.hpp"


class MovesContainer {
private:
    ActionsArrays arr;
public:
    template <typename T>
    int insert_or_update(const T& move, const uint score, const int) {
        return arr.insert_or_update(move, static_cast<double>(score), 1.0);
    }

    template <typename T>
    double get_score_or_default_value(const T& move, const int) {
        return arr.get_score_or_default_value(move);
    }

    void apply_decay_factor();
};

#endif
