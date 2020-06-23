#include "mcts_tree.hpp"
#include "node.hpp"
#include "constants.hpp"
#include "simulator.hpp"
#include <iostream>


MctsTree::MctsTree(const reasoner::game_state& initial_state) : 
    root_state(initial_state),
    random_numbers_generator(std::random_device{}()) {
    // no changes
    complete_turn(root_state);
    create_node(root_state);
    nodes.reserve(4 * MEBIBYTE / sizeof(Node));
    children.reserve(4 * MEBIBYTE / sizeof(Child));
}

uint MctsTree::create_node(reasoner::game_state& state) {
    // get_all_moves -> get_all_semimoves
    // function has_nodal_successor
    static std::vector<reasoner::semimove> semimoves;
    state.get_all_semimoves(cache, semimoves, SEMILENGTH);
    auto child_count = semimoves.size();
    uint new_child_index = children.size();
    auto state_copy = state;
    nodes.emplace_back(new_child_index, child_count, state.is_nodal(), has_nodal_successor(state_copy, cache, random_numbers_generator));
    for (const auto& semimove : semimoves) {
        auto ri = state.apply_semimove_with_revert(semimove);
        children.emplace_back(semimove, state.is_nodal());
        state.revert(ri);
    }
    return nodes.size() - 1;
}

void MctsTree::complete_turn(reasoner::game_state& state) {
    // no changes
    while (state.get_current_player() == KEEPER && state.apply_any_move(cache));
}

uint MctsTree::get_best_uct_child_index(const uint node_index) {
    // no chages
    static std::vector<uint> children_indices;
    const auto& [fst, lst] = nodes[node_index].children_range;
    children_indices.resize(1);
    children_indices[0] = fst;
    double const c_sqrt_logn = EXPLORATION_CONSTANT * std::sqrt(std::log(nodes[node_index].sim_count));
    double max_priority = children[fst].total_score / EXPECTED_MAX_SCORE / children[fst].sim_count +
                          c_sqrt_logn / std::sqrt(static_cast<double>(children[fst].sim_count));
    for (uint i = fst + 1; i < lst; ++i) {
        double priority = children[i].total_score / EXPECTED_MAX_SCORE / children[i].sim_count +
                          c_sqrt_logn / std::sqrt(static_cast<double>(children[i].sim_count));
        if (priority > max_priority) {
            max_priority = priority;
            children_indices.resize(1);
            children_indices[0] = i;
        }
        else if (priority == max_priority) {
            children_indices.push_back(i);
        }
    }
    std::uniform_int_distribution<uint> dist(0, children_indices.size() - 1);
    return children_indices[dist(random_numbers_generator)];
}

game_status_indication MctsTree::get_status(const int player_index) const {
    // no changes
    if (nodes.front().is_terminal()) {
        return end_game;
    }
    return root_state.get_current_player() == (player_index + 1) ? own_turn : opponent_turn;
}

reasoner::move MctsTree::choose_best_move() {
    reasoner::move move;
    uint node_index = 0;
    while (true) {
        const auto& [fst, lst] = nodes[node_index].children_range;
        auto max_sim = children[fst].sim_count;
        auto best_node = fst;
        for (auto i = fst + 1; i < lst; ++i) {
            if (children[i].sim_count > max_sim) {
                max_sim = children[i].sim_count;
                best_node = i;
            }
        }
        const auto& semimove = children[best_node].semimove.get_actions();
        move.mr.insert(move.mr.end(), semimove.begin(), semimove.end());
        if (children[best_node].is_nodal) {
            break;
        }
        node_index = children[best_node].index;
        assert(node_index != 0);
    }
    return move;
}

void MctsTree::reparent_along_move(const reasoner::move& move) {
    root_state.apply_move(move);
    complete_turn(root_state);
    uint index = 0;
    if constexpr (SEMILENGTH == 1) {
        for (const auto& action : move.mr) {
            const auto [fst, lst] = nodes[index].children_range;
            auto i = fst;
            while (i < lst) {
                if (children[i].semimove.get_actions().front() == action) {
                    index = children[i].index;
                    break;
                }
                ++i;
            }
            if (i == lst || index == 0) {
                break;
            }
        }
    }
    else {
        // not implemented
        assert(false);
    }
    if (index == 0) {
        nodes.clear();
        children.clear();
        create_node(root_state);
        return;
    }
    std::vector<Node> nodes_tmp;
    std::vector<Child> children_tmp;
    nodes_tmp.reserve(nodes.size());
    children_tmp.reserve(children.size());
    fix_tree(nodes_tmp, children_tmp, index);
    nodes = std::move(nodes_tmp);
    children = std::move(children_tmp);
}

uint MctsTree::fix_tree(std::vector<Node>& nodes_tmp, std::vector<Child>& children_tmp, const uint index) {
    // two loops
    auto new_index = nodes_tmp.size();
    nodes_tmp.push_back(nodes[index]);
    auto first_child_index = children_tmp.size();
    const auto& [fst, lst] = nodes[index].children_range;
    auto child_count = lst - fst;
    nodes_tmp[new_index].children_range =  std::make_pair(first_child_index, first_child_index + child_count);
    for (uint i = 0; i < child_count; ++i) {
        children_tmp.push_back(children[fst + i]);
    }
    for (uint i = 0; i < child_count; ++i) {
        if (children[fst + i].index == 0) {
            continue;
        }
        children_tmp[first_child_index + i].index = fix_tree(nodes_tmp, children_tmp, children[fst + i].index);
    }
    return new_index;
}
