#include "node.hpp"
#include "reasoner.hpp"

node::node(const uint first_child_index, const uint child_count)
    : node_base(first_child_index, child_count) {}

bool node::is_terminal() const {
    return children_range.second == children_range.first;
}

child::child(const reasoner::move& move)
    : move(move) {}

const reasoner::move_representation& child::get_actions() const {
    return move.mr;
}

const reasoner::move& child::get_edge() const {
    return move;
}
