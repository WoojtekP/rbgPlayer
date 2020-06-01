#include "moves_container.hpp"

void moves_container::insert_or_update(const reasoner::move& move, const uint& score) {
    for (const auto& action : move.mr) {
        auto action_index = action.cell * reasoner::NUMBER_OF_MODIFIERS + reasoner::action_to_modifier_index(action.index);
        map[action_index].first++;
        map[action_index].second += score;
    }
}

double moves_container::get_score_or_default_value(const reasoner::move& move) {
    uint size = 0;
    double total_score = 0.0;
    for (const auto& action : move.mr) {
        auto action_index = action.cell * reasoner::NUMBER_OF_MODIFIERS + reasoner::action_to_modifier_index(action.index);
        total_score += (map[action_index].first == 0) ? default_value : (static_cast<double>(map[action_index].second) / map[action_index].first);
        size++;
    }
    return total_score / size;
}