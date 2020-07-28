#include "node.hpp"
#include "reasoner.hpp"

Node::Node(const uint first_child_index, const uint child_count, const bool is_nodal, const bool nodal_succ)
    : NodeBase(first_child_index, child_count)
    , is_nodal(is_nodal)
    , has_nodal_succ(nodal_succ) {}

bool Node::is_terminal() const {
    return is_nodal && !has_nodal_succ;
}

Child::Child(const reasoner::semimove& semimove, const bool is_nodal)
    : semimove(semimove)
    , is_nodal(is_nodal) {}

const reasoner::move_representation& Child::get_actions() const {
    return semimove.get_actions();
}
