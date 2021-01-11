#include "node.hpp"
#include "reasoner.hpp"


child::child(const reasoner::action_representation action)
    : action(action) {}

const reasoner::action_representation child::get_action() const {
    return action;
}

const reasoner::action_representation child::get_edge() const {
    return action;
}
