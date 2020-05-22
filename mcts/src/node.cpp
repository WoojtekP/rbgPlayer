#include "node.hpp"
#include "reasoner.hpp"

Node::Node(const uint& first_child_index, const uint& child_count) 
    : children_range(first_child_index, first_child_index + child_count) {}

bool Node::is_terminal() const {
    return (children_range.second - children_range.first) == 0;
}

Child::Child(const reasoner::move& move)
    : move(move) {}

void Child::update_stats(const uint& current_player, simulation_result& results) {
    sim_count++;
    total_score += results[current_player];
}