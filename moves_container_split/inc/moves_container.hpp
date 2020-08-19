#ifndef MOVESCONTAINER
#define MOVESCONTAINER

#include "reasoner.hpp"

struct score {
    double sum;
    double weight;
};

class moves_container {
private:
    static constexpr uint size1 = reasoner::BOARD_SIZE * reasoner::NUMBER_OF_MODIFIERS;
    static constexpr uint size2 = reasoner::BOARD_SIZE * reasoner::AUTOMATON_SIZE;
    score map1[moves_container::size1];
    score map2[moves_container::size2];
    void insert_or_update(const reasoner::move_representation&, const uint, const uint);
public:
    moves_container();
    template <typename T>
    int insert_or_update(const T& semimove, const uint score, const uint depth, const int) {
        insert_or_update(semimove.mr, score, depth);
        auto index = (semimove.cell - 1) * reasoner::AUTOMATON_SIZE + semimove.state;
        if constexpr (WEIGHT_SCALING) {
            map2[index].weight += 1.0 / depth;
            map2[index].sum += static_cast<double>(score) / depth;
        }
        else {
            map2[index].weight += 1.0;
            map2[index].sum += static_cast<double>(score);
        }
        return 0;
    }
    int insert_or_update(const reasoner::move& move, const uint score, const uint depth, const int) {
        insert_or_update(move.mr, score, depth);
        return 0;
    }

    template <typename T>
    double get_score_or_default_value(const T& semimove, const int) {
        double total_sum = 0;
        [[maybe_unused]] double weight_sum = 0;
        for (const auto& action : semimove.mr) {
            auto action_index = (action.cell - 1) * reasoner::NUMBER_OF_MODIFIERS + reasoner::action_to_modifier_index(action.index);
            if (map1[action_index].weight == 0) {
                return EXPECTED_MAX_SCORE;
            }
            if constexpr (WEIGHTED_MEAN) {
                total_sum += map1[action_index].sum;
                weight_sum += map1[action_index].weight;
            }
            else {
                total_sum += map1[action_index].sum / map1[action_index].weight;
            }
        }
        auto index = (semimove.cell - 1) * reasoner::AUTOMATON_SIZE + semimove.state;
        if (map2[index].weight == 0) {
            return EXPECTED_MAX_SCORE;
        }
        if constexpr (WEIGHTED_MEAN) {
            total_sum += map2[index].sum;
            weight_sum += map2[index].weight;
            return total_sum / weight_sum;
        }
        else {
            total_sum += map2[index].sum / map2[index].weight;
            return total_sum / (1 + semimove.mr.size());
        }
    }
    double get_score_or_default_value(const reasoner::move&, const int);
    void apply_decay_factor();
};

#endif
