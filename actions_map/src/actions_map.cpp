#include "actions_map.hpp"

#include "constants.hpp"
#include "reasoner.hpp"


ActionsMap::ActionsMap() {
    for (uint i = 0; i < ActionsMap::size; ++i) {
        arr[i].sum = arr[i].weight = 0.0;
    }
}

uint ActionsMap::action_to_index(const reasoner::action_representation action) {
    return (action.cell - 1) * (reasoner::NUMBER_OF_MODIFIERS + reasoner::AUTOMATON_SIZE) + reasoner::AUTOMATON_SIZE + action.index - 1;
}

int ActionsMap::insert_or_update(const reasoner::action_representation action, const double score, const int) {
    const auto index = action_to_index(action);
    arr[index].weight += 1.0;
    arr[index].sum += score;
    return 0;
}

int ActionsMap::insert_or_update(const reasoner::move& move, const double score, const int) {
    for (const auto& action : move.mr) {
        insert_or_update(action, score);
    }
    return 0;
}

score ActionsMap::get_score_or_default_value(const reasoner::action_representation action, const int) {
    static const score default_score(EXPECTED_MAX_SCORE, 1.0);
    const auto index = action_to_index(action);
    return arr[index].weight == 0 ? default_score : arr[index];
}

score ActionsMap::get_score_or_default_value(const reasoner::move& move, const int) {
    double sum = 0;
    for (const auto& action : move.mr) {
        const auto index = action_to_index(action);
        if (arr[index].weight == 0) {
            return {EXPECTED_MAX_SCORE, 1.0};
        }
        sum += arr[index].sum / arr[index].weight;
    }
    return {sum, static_cast<double>(move.mr.size())};
}

void ActionsMap::apply_decay_factor() {
    for (uint i = 0; i < ActionsMap::size; ++i) {
        arr[i].weight *= DECAY_FACTOR;
        arr[i].sum *= DECAY_FACTOR;
    }
}
