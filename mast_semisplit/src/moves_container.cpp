#include "moves_container.hpp"

void moves_container::insert_or_update(const reasoner::move& move, const uint& score) {
    for (const auto& action : move.mr) {
        auto action_index = (action.cell - 1) * reasoner::NUMBER_OF_MODIFIERS + reasoner::action_to_modifier_index(action.index);
        map[action_index].first++;
        map[action_index].second += score;
    }
}

double moves_container::get_score_or_default_value(const reasoner::move& move) {
    uint weight_sum = 0;
    uint total_score = 0;
    for (const auto& action : move.mr) {
        auto action_index = (action.cell - 1) * reasoner::NUMBER_OF_MODIFIERS + reasoner::action_to_modifier_index(action.index);
        total_score += map[action_index].second;
        weight_sum += map[action_index].first;
    }
    return (weight_sum == 0) ? default_value : (static_cast<double>(total_score) / weight_sum);
}