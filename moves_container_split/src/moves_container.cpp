#include "constants.hpp"
#include "moves_container.hpp"


moves_container::moves_container() {
    for (uint i = 0; i < moves_container::size; ++i) {
        map[i].first = map[i].second = 0.0;
    }
}

void moves_container::insert_or_update(const reasoner::move_representation& move, const uint score,[[maybe_unused]] const uint depth) {
    for (const auto& action : move) {
        auto action_index = (action.cell - 1) * reasoner::NUMBER_OF_MODIFIERS + reasoner::action_to_modifier_index(action.index);
        if constexpr (WEIGHT_SCALING == 1) {
            map[action_index].first += 1.0 / depth;
            map[action_index].second += static_cast<double>(score) / depth;
        }
        else {
            map[action_index].first += 1.0;
            map[action_index].second += static_cast<double>(score);
        }
    }
}

double moves_container::get_score_or_default_value(const reasoner::move_representation& move) {
    double weight_sum = 0;
    double total_score = 0;
    for (const auto& action : move) {
        auto action_index = (action.cell - 1) * reasoner::NUMBER_OF_MODIFIERS + reasoner::action_to_modifier_index(action.index);
        if (map[action_index].first == 0) {
            return EXPECTED_MAX_SCORE;
        }
        total_score += map[action_index].second;
        weight_sum += map[action_index].first;
    }
    return total_score / weight_sum;
}

void moves_container::apply_decay_factor() {
    for (uint i = 0; i < moves_container::size; ++i) {
        map[i].first *= DECAY_FACTOR;
        map[i].second *= DECAY_FACTOR;
    }
}
