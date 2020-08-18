#include "constants.hpp"
#include "moves_container.hpp"
#include "reasoner.hpp"

moves_container::moves_container() {
    for (uint i = 0; i < moves_container::size1; ++i) {
        map1[i].weight = map1[i].sum = 0.0;
    }
    for (uint i = 0; i < moves_container::size2; ++i) {
        map2[i].weight = map2[i].sum = 0.0;
    }
}

void moves_container::insert_or_update(const reasoner::move_representation& mr, const uint score, [[maybe_unused]] const uint depth) {
    for (const auto& action : mr) {
        auto action_index = (action.cell - 1) * reasoner::NUMBER_OF_MODIFIERS + reasoner::action_to_modifier_index(action.index);
        if constexpr (WEIGHT_SCALING) {
            map1[action_index].weight += 1.0 / depth;
            map1[action_index].sum += static_cast<double>(score) / depth;
        }
        else {
            map1[action_index].weight += 1.0;
            map1[action_index].sum += static_cast<double>(score);
        }
    }
}

double moves_container::get_score_or_default_value(const reasoner::move& move) {
    double total_sum = 0;
    [[maybe_unused]] double weight_sum = 0;
    for (const auto& action : move.mr) {
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
    if constexpr (WEIGHTED_MEAN) {
        return total_sum / weight_sum;
    }
    else {
        return total_sum / move.mr.size();
    }
}

void moves_container::apply_decay_factor() {
    for (uint i = 0; i < moves_container::size1; ++i) {
        map1[i].weight *= DECAY_FACTOR;
        map1[i].sum *= DECAY_FACTOR;
    }
    for (uint i = 0; i < moves_container::size2; ++i) {
        map2[i].weight *= DECAY_FACTOR;
        map2[i].sum *= DECAY_FACTOR;
    }
}
