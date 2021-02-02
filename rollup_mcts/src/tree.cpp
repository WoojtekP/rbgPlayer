#if STATS
#include <iostream>
#include <iomanip>
#endif

#include "tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "constants.hpp"


void Tree::choose_children_for_rolling_up(const uint node_index, std::vector<uint>& children_indices) {
    children_indices.clear();
    const auto [fst, lst] = nodes[node_index].children_range;
    for (auto i = fst; i < lst; ++i) {
        auto index = children[i].index;
        if (index > 0 && !nodes[index].is_nodal && is_node_fully_expanded(index)) {
            const auto [fst_child, lst_child] = nodes[index].children_range;
            const auto child_count = lst_child - fst_child;
            if (children[i].sim_count >= child_count * MIN_SIMULATIONS_FACTOR) {
                if constexpr (MIN_SCORE_DIFF == 0.0) {
                    children_indices.push_back(i);
                }
                else if (child_count == 1) {
                    children_indices.push_back(i);
                }
                else {
                    double min_score = static_cast<double>(children[fst_child].total_score) / children[fst_child].sim_count;
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
        auto mr_prefix = children[child_index].semimove.mr;
        if (mr_prefix.back().index <= 0) {
            mr_prefix.pop_back();
        }
        #ifndef NDEBUG
        for (const auto action : mr_prefix) {
            assert(action.index > 0);
        }
        #endif
        if (mr_prefix.empty()) {
            for (auto j = fst; j < lst; ++j) {
                children.push_back(std::move(children[j]));
            }
        }
        else {
            for (auto j = fst; j < lst; ++j) {
                children.push_back(std::move(children[j]));
                assert(children.back().semimove.mr.size() + mr_prefix.size() <= children.back().semimove.mr.max_size());
                children.back().semimove.mr.insert(children.back().semimove.mr.begin(), mr_prefix.begin(), mr_prefix.end());
            }
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
        const auto node_index = children[std::get<1>(children_stack[i])].index;
        choose_children_for_rolling_up(node_index, children_indices);
        if (!children_indices.empty()) {
            roll_up(node_index, children_indices);
        }
    }
    choose_children_for_rolling_up(0, children_indices);
    if (!children_indices.empty()) {
        roll_up(0, children_indices);
    }
    return state_count;
}

reasoner::move Tree::choose_best_move() {
    #if STATS
    std::cout << "turn number " << turn_number << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    #endif
    static std::vector<uint> children_indices;
    children_indices.clear();
    choose_best_greedy_move(children_indices);
    return get_move_from_saved_path_with_random_suffix(children_indices);
}
