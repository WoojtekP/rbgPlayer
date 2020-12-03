#include "semisplit_node.hpp"
#include "reasoner.hpp"

semisplit_node::semisplit_node(const uint first_child_index, const uint child_count, const bool is_nodal, const node_status status)
    : node_base(first_child_index, child_count)
    , is_nodal(is_nodal)
    , status(status) {}

semisplit_node::semisplit_node(const bool is_nodal, const node_status status)
    : node_base()
    , is_nodal(is_nodal)
    , status(status) {}

bool semisplit_node::is_terminal() const {
    bool result = status == node_status::terminal;
    if (result) {
        assert(children_range.first == children_range.second);
    }
    return result;
}

semisplit_child::semisplit_child(const reasoner::action_representation action)
    : action(action) {}

const reasoner::action_representation semisplit_child::get_action() const {
    return action;
}

const reasoner::action_representation semisplit_child::get_edge() const {
    return action;
}
