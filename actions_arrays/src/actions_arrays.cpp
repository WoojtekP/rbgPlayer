#include "constants.hpp"
#include "actions_arrays.hpp"
#include "reasoner.hpp"

actions_arrays::actions_arrays() {
    for (uint i = 0; i < actions_arrays::size1; ++i) {
        arr1[i].weight = arr1[i].sum = 0.0;
    }
    for (uint i = 0; i < actions_arrays::size2; ++i) {
        arr2[i].weight = arr2[i].sum = 0.0;
    }
}

void actions_arrays::insert_or_update(const reasoner::move_representation& mr, const double score, const double weight) {
    for (const auto& action : mr) {
        auto action_index = (action.cell - 1) * reasoner::NUMBER_OF_MODIFIERS + reasoner::action_to_modifier_index(action.index);
        arr1[action_index].weight += weight;
        arr1[action_index].sum += score;
    }
}

int actions_arrays::insert_or_update(const reasoner::move& move, const double score, const double weight) {
    insert_or_update(move.mr, score, weight);
    return 0;
}

double actions_arrays::get_score_or_default_value(const reasoner::move& move) {
    double total_sum = 0;
    [[maybe_unused]] double weight_sum = 0;
    for (const auto& action : move.mr) {
        auto action_index = (action.cell - 1) * reasoner::NUMBER_OF_MODIFIERS + reasoner::action_to_modifier_index(action.index);
        if (arr1[action_index].weight == 0) {
            return EXPECTED_MAX_SCORE;
        }
        total_sum += arr1[action_index].sum / arr1[action_index].weight;
    }
    return total_sum / move.mr.size();
}

void actions_arrays::apply_decay_factor() {
    for (uint i = 0; i < actions_arrays::size1; ++i) {
        arr1[i].weight *= DECAY_FACTOR;
        arr1[i].sum *= DECAY_FACTOR;
    }
    for (uint i = 0; i < actions_arrays::size2; ++i) {
        arr2[i].weight *= DECAY_FACTOR;
        arr2[i].sum *= DECAY_FACTOR;
    }
}
