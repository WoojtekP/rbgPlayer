#include "actions_arrays.hpp"
#include "constants.hpp"
#include "reasoner.hpp"

ActionsArrays::ActionsArrays() {
    for (uint i = 0; i < ActionsArrays::size; ++i) {
        arr[i].sum = arr[i].weight = 0.0;
    }
}

uint ActionsArrays::action_to_index(const reasoner::action_representation action) {
    return (action.cell - 1) * (reasoner::NUMBER_OF_MODIFIERS + reasoner::AUTOMATON_SIZE) + reasoner::AUTOMATON_SIZE + action.index - 1;
}

int ActionsArrays::insert_or_update(const reasoner::action_representation action, const double score, const double weight) {
    auto index = action_to_index(action);
    arr[index].weight += weight;
    arr[index].sum += score;
    return 0;
}

int ActionsArrays::insert_or_update(const reasoner::move& move, const double score, const double weight) {
    for (const auto& action : move.mr) {
        auto index = action_to_index(action);
        arr[index].weight += weight;
        arr[index].sum += score;
    }
    return 0;
}

double ActionsArrays::get_score_or_default_value(const reasoner::action_representation action) {
    const auto index = action_to_index(action);
    return arr[index].weight == 0 ? EXPECTED_MAX_SCORE : arr[index].sum / arr[index].weight;
}

double ActionsArrays::get_score_or_default_value(const reasoner::move& move) {
    double sum = 0;
    for (const auto& action : move.mr) {
        auto index = action_to_index(action);
        if (arr[index].weight == 0) {
            return EXPECTED_MAX_SCORE;
        }
        sum += arr[index].sum / arr[index].weight;
    }
    return sum / move.mr.size();
}

void ActionsArrays::apply_decay_factor() {
    for (uint i = 0; i < ActionsArrays::size; ++i) {
        arr[i].weight *= DECAY_FACTOR;
        arr[i].sum *= DECAY_FACTOR;
    }
}
