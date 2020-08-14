#include "node.hpp"
#include "reasoner.hpp"

Node::Node(const uint first_child_index, const uint child_count, const bool is_nodal, const node_status status)
    : NodeBase(first_child_index, child_count)
    , is_nodal(is_nodal)
    , status(status) {}

Node::Node(const bool is_nodal, const node_status status)
    : NodeBase()
    , is_nodal(is_nodal)
    , status(status) {}

bool Node::is_terminal() const {
    bool result = status == node_status::terminal;
    if (result) {
        assert(children_range.first == children_range.second);
    }
    return result;
}

Child::Child(const reasoner::semimove& semimove)
    : semimove(semimove) {}

const reasoner::move_representation& Child::get_actions() const {
    return semimove.get_actions();
}
