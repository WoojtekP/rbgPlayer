#include "actions_set.hpp"
#include "constants.hpp"
#include "reasoner.hpp"


ActionsSet::ActionsSet() {
    actions_bitset.reset();
}

uint ActionsSet::action_to_index(const reasoner::action_representation action) {
    return (action.cell - 1) * (reasoner::NUMBER_OF_MODIFIERS + reasoner::AUTOMATON_SIZE) + reasoner::AUTOMATON_SIZE + action.index - 1;
}

int ActionsSet::insert(const reasoner::action_representation action, const int) {
    const auto index = action_to_index(action);
    actions_bitset.set(index);
    return 0;
}

int ActionsSet::insert(const reasoner::move& move, const int) {
    for (const auto& action : move.mr) {
        insert(action);
    }
    return 0;
}

bool ActionsSet::find(const reasoner::action_representation action, const int) {
    const auto index = action_to_index(action);
    return actions_bitset.test(index);
}

void ActionsSet::reset() {
    actions_bitset.reset();
}
