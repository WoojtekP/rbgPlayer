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
        auto mr_prefix = children[child_index].move.mr;
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
                children.back().move.mr.insert(children.back().move.mr.begin(), mr_prefix.begin(), mr_prefix.end());
            }
        }
    }
    for (; i < nodes[node_index].children_range.second; ++i) {
        children.push_back(std::move(children[i]));
    }
    nodes[node_index].children_range.first = new_child_index;
    nodes[node_index].children_range.second = children.size();
}

reasoner::move Tree::get_move_from_saved_path_with_random_suffix(std::vector<uint>& children_indices) {
    static GameState state;
    state = root_state;
    reasoner::move move;
    for (const auto child_index : children_indices) {
        const auto& mr = children[child_index].move.mr;
        move.mr.insert(move.mr.end(), mr.begin(), mr.end());
        state.apply_move(move);
    }
    if (!state.is_nodal()) {
        #if STATS
        std::cout << "random continuation..." << std::endl << std::endl;
        #endif
        static std::vector<reasoner::action_representation> move_suffix;
        move_suffix.clear();
        random_walk_to_nodal(state, move_suffix);
        assert(!move_suffix.empty());
        for (const auto action : move_suffix) {
            if (action.index > 0) {
                move.mr.push_back(action);
            }
        }
    }
    return move;
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


void Tree::reparent_along_move(const reasoner::move& move) {
    #if STATS
    ++turn_number;
    #endif
    root_state.apply_move(move);
    complete_turn(root_state);
    uint root_index = 0;
    static std::vector<std::tuple<uint, uint, uint>> stack;
    stack.clear();
    const auto& mr = move.mr;
    const uint size = mr.size();
    uint i = 0;
    auto [fst, lst] = nodes.front().children_range;
    while (i < size) {
        if (!nodes[root_index].is_expanded()) {
            root_index = 0;
            break;
        }
        while (fst < lst) {
            if (children[fst].move.mr.size() == 1 && children[fst].get_action().index <= 0) {
                stack.emplace_back(root_index, fst, root_sim_count);
                root_index = children[fst].index;
                root_sim_count = children[fst].sim_count;
                break;
            }
            bool matched = true;
            uint j = 0;
            for (const auto action : children[fst].move.mr) {
                if (!(action == mr[i + j])) {
                    matched = false;
                    break;
                }
                ++j;
            }
            if (matched) {
                root_index = children[fst].index;
                root_sim_count = children[fst].sim_count;
                i += j;
                stack.clear();
                break;
            }
            ++fst;
        }
        if (fst == lst || root_index == 0) {
            if (!stack.empty()) {
                std::tie(root_index, fst, root_sim_count) = stack.back();
                stack.pop_back();
                ++fst;
                lst = nodes[root_index].children_range.second;
                continue;
            }
            else {
                root_index = 0;
                break;
            }
        }
        std::tie(fst, lst) = nodes[root_index].children_range;
    }
    if (root_index == 0) {
        root_sim_count = 0;
        nodes.clear();
        children.clear();
        const auto status = has_nodal_successor(root_state) ? node_status::nonterminal : node_status::terminal;
        create_node(root_state, status);
    }
    else {
        root_at_index(root_index);
        if (nodes.front().status == node_status::unknown) {
            if (has_nodal_successor(root_state)) {
                nodes.front().status = node_status::nonterminal;
            }
            else {
                nodes.front().status = node_status::terminal;
                nodes.front().children_range = {0, 0};
            }
        }
    }
    if (!nodes.front().is_expanded()) {
        create_children(0, root_state);
    }
    #if MAST > 0
    move_chooser.complete_turn();
    #endif
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
