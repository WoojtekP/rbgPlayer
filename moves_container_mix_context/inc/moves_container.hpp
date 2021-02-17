#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include <cmath>

#include "actions_map.hpp"
#include "constants.hpp"
#include "moves_hashmap.hpp"
#include "hashmap_context_entry.hpp"
#include "move_hash_context.hpp"


class MovesContainer {
private:
    MovesHashmap<HashmapContextEntry, move_hash_context<HashmapContextEntry>> moves;
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
        auto context_score = moves.get_score_or_default_value(move, context);
        if constexpr (MASTMIX_THRESHOLD == 0) {
            return context_score;
        }
        if (context_score.weight < MASTMIX_THRESHOLD)
            return arr.get_score_or_default_value(move);
        else
            return context_score;
        /*
        // Smooth mix
        double split_score = arr.get_score_or_default_value(move);
        double beta = std::sqrt(MASTMIX_THRESHOLD / (3.0 * context_score.weight  + MASTMIX_THRESHOLD)); // Normal
        return beta * split_score + (1.0 - beta) * context_score.get_score();
        */
    }

    void apply_decay_factor();
    int get_context(const reasoner::action_representation, const int);
};

#endif
