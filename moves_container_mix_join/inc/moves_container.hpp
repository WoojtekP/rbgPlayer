#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include <cmath>

#include "actions_map.hpp"
#include "constants.hpp"
#include "moves_hashmap.hpp"
#include "hashmap_entry.hpp"
#include "move_hash.hpp"


class MovesContainer {
private:
    MovesHashmap<HashmapEntry, move_hash<HashmapEntry>> moves;
    ActionsMap arr;
public:
    template <typename T>
    int insert_or_update(const T& move, const uint score, const int context = 0) {
        if constexpr (MASTMIX_THRESHOLD > 0) {
            arr.insert_or_update(move, static_cast<double>(score));
        }
        return moves.insert_or_update(move, static_cast<double>(score), context);
    }

    template <typename T>
    score get_score_or_default_value(const T& move, const int context = 0) {
        auto join_score = moves.get_score_or_default_value(move);
        if constexpr (MASTMIX_THRESHOLD == 0) {
            return join_score;
        }
        if (join_score.weight < MASTMIX_THRESHOLD)
            return arr.get_score_or_default_value(move);
        else
            return join_score;
        /*
        // Smooth mix
        double split_score = arr.get_score_or_default_value(move);
        double beta = std::sqrt(MASTMIX_THRESHOLD / (3.0 * join_score.weight  + MASTMIX_THRESHOLD)); // Normal
        return beta * split_score + (1.0 - beta) * join_score.get_score();
        */
    }

    void apply_decay_factor();
    int get_context(const reasoner::move&, const int);
};

#endif
