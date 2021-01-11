#include "node.hpp"
#include "reasoner.hpp"


child::child(const reasoner::move& move)
    : move(move) {}

child::child(const reasoner::action_representation action) {
    move.mr.push_back(action);
}

const reasoner::action_representation child::get_action() const {
    assert(move.mr.size() == 1);
    return move.mr.back();
}

const reasoner::move& child::get_edge() const {
    return move;
}
