#include "node_base.hpp"
#include "reasoner.hpp"

NodeBase::NodeBase(const uint first_child_index, const uint child_count)
    : children_range(first_child_index, first_child_index + child_count) {}

bool NodeBase::is_expanded() const {
    return children_range.first <= children_range.second;
}
