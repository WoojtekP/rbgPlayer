#include "node_base.hpp"
#include "reasoner.hpp"

node_base::node_base(const uint first_child_index, const uint child_count)
    : children_range(first_child_index, first_child_index + child_count) {}

bool node_base::is_expanded() const {
    return children_range.first <= children_range.second;
}
