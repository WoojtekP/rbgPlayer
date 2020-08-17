#include "semisplit_node.hpp"
#include "reasoner.hpp"

SemisplitNode::SemisplitNode(const uint first_child_index, const uint child_count, const bool is_nodal, const node_status status)
    : NodeBase(first_child_index, child_count)
    , is_nodal(is_nodal)
    , status(status) {}

SemisplitNode::SemisplitNode(const bool is_nodal, const node_status status)
    : NodeBase()
    , is_nodal(is_nodal)
    , status(status) {}

bool SemisplitNode::is_terminal() const {
    bool result = status == node_status::terminal;
    if (result) {
        assert(children_range.first == children_range.second);
    }
    return result;
}

SemisplitChild::SemisplitChild(const reasoner::semimove& semimove)
    : semimove(semimove) {}

const reasoner::move_representation& SemisplitChild::get_actions() const {
    return semimove.get_actions();
}
