#if STATS
#include <iostream>
#include <iomanip>
#endif

#include "tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "constants.hpp"


Tree::Tree(const reasoner::game_state& initial_state) : SemisplitTree(initial_state) {}

void Tree::choose_children_for_rolling_up(const uint node_index, const uint node_sim_count, std::vector<uint>& children_indices) {
    children_indices.clear();
    const auto [fst, lst] = nodes[node_index].children_range;
    for (auto i = fst; i < lst; ++i) {
        auto index = children[i].index;
        if (index > 0 && !nodes[index].is_nodal && is_node_fully_expanded(index)) {
            const auto [fst_child, lst_child] = nodes[index].children_range;
            const auto child_count = lst_child - fst_child;
            if (node_sim_count >= child_count * MIN_SIMULATIONS_FACTOR) {
                if constexpr (MIN_SCORE_DIFF == 0.0) {
                    children_indices.push_back(i);
                }
                else if (child_count == 1) {
                    children_indices.push_back(i);
                }
                else {
                    double min_score = static_cast<double>(children[fst_child].total_score) / children[fst_child].sim_count;;
                    double max_score = min_score;
                    for (auto i = fst_child + 1; i < lst_child; ++i) {
                        double score = static_cast<double>(children[i].total_score) / children[i].sim_count;
                        if (score > max_score) {
                            max_score = score;
                        }
                        else if (score < min_score) {
                            min_score = score;
                        }
                    }
                    if (max_score - min_score >= MIN_SCORE_DIFF) {
                        children_indices.push_back(i);
                    }
                }
            }
        }
    }
}

void Tree::roll_up(const uint node_index, std::vector<uint>& children_indices) {
    uint new_child_index = children.size();
    uint i = nodes[node_index].children_range.first;
    for (const auto child_index : children_indices) {
        assert(child_index >= nodes[node_index].children_range.first);
        assert(child_index < nodes[node_index].children_range.second);
        for (; i < child_index; ++i) {
            children.push_back(std::move(children[i]));
        }
        ++i;
        assert(children[child_index].index > 0);
        assert(is_node_fully_expanded(children[child_index].index));
        const auto [fst, lst] = nodes[children[child_index].index].children_range;
        const auto& semimove_prefix = children[child_index].semimove;
        const auto mr_prefix = semimove_prefix.get_actions();
        for (auto j = fst; j < lst; ++j) {
            const auto& semimove_suffix = children[j].semimove;
            auto mr_suffix = semimove_suffix.get_actions();
            const auto cell = semimove_suffix.cell;
            const auto state = semimove_suffix.state;
            mr_suffix.insert(mr_suffix.begin(), mr_prefix.begin(), mr_prefix.end());
            children.push_back(std::move(children[j]));
            children.back().semimove = reasoner::semimove(mr_suffix, cell, state);
        }
    }
    for (; i < nodes[node_index].children_range.second; ++i) {
        children.push_back(std::move(children[i]));
    }
    nodes[node_index].children_range.first = new_child_index;
    nodes[node_index].children_range.second = children.size();
}

uint Tree::perform_simulation() {
    uint state_count = SemisplitTree::perform_simulation();
    static std::vector<uint> children_indices;
    int size = children_stack.size();
    for (int i = size - 2; i >= 0; --i) {
        const auto node_index = children[children_stack[i].first].index;
        const auto node_sim_count = children[children_stack[i].first].sim_count;
        choose_children_for_rolling_up(node_index, node_sim_count, children_indices);
        if (!children_indices.empty()) {
            roll_up(node_index, children_indices);
        }
    }
    choose_children_for_rolling_up(0, root_sim_count, children_indices);
    if (!children_indices.empty()) {
        roll_up(0, children_indices);
    }
    return state_count;
}

reasoner::move Tree::choose_best_move() {
    return choose_best_greedy_move();
}
