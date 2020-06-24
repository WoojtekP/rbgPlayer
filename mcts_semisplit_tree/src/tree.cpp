#include <stack>

#include <boost/range/adaptor/reversed.hpp>

#include "tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "constants.hpp"

#include <iostream>

Tree::Tree(const reasoner::game_state& initial_state) : MctsTree(initial_state) {}

uint Tree::get_unvisited_child_index(const uint node_index) {
    // no changes
    auto [fst, lst] = nodes[node_index].children_range;
    auto lower = fst + nodes[node_index].sim_count;
    while (lower > fst && children[lower - 1].index == 0) {
        --lower;
    }
    std::uniform_int_distribution<uint> dist(lower, lst - 1);
    auto chosen_child = dist(random_numbers_generator);
    if (chosen_child != lower) {
        std::swap(children[chosen_child], children[lower]);
    }
    return lower;
}

void Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS - 1);
    static reasoner::game_state state = root_state;
    static std::vector<std::pair<uint,int>> children_stack;  // child index, player
    if (reset_path) {
        children_stack.clear();
    }
    uint node_index = children_stack.empty() ? 0 : children[children_stack.back().first].index;
    state = root_state;
    for (const auto el : children_stack) {
        state.apply_semimove(children[el.first].semimove);
        complete_turn(state);
    }
    while (!nodes[node_index].is_terminal() && is_node_fully_expanded(node_index)) {
        const auto child_index = get_best_uct_child_index(node_index);
        const auto current_player = state.get_current_player();
        state.apply_semimove(children[child_index].semimove);
        complete_turn(state);
        children_stack.emplace_back(child_index, current_player);
        node_index = children[child_index].index;
        if (node_index == 0) {
            assert(false);
            break;
        }
    }
    if (nodes[node_index].is_terminal()) {
        for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
            results[i - 1] = state.get_player_score(i);
        }
        for (const auto [index, player] : children_stack) {
            nodes[children[index].index].sim_count++;
            children[index].sim_count++;
            children[index].total_score += results[player - 1];
        }
        nodes.front().sim_count++;
        children_stack.clear();
    }
    else {
        const auto child_index = get_unvisited_child_index(node_index);
        const auto current_player = state.get_current_player();
        state.apply_semimove(children[child_index].semimove);
        complete_turn(state);
        auto state_copy = state;
        if (play(state_copy, cache, random_numbers_generator, results)) {
            auto new_node_index = create_node(state);
            nodes[new_node_index].sim_count = 1;
            children[child_index].index = new_node_index;
            children[child_index].sim_count = 1;
            children[child_index].total_score += results[current_player - 1];
            for (const auto [index, player] : children_stack) {
                nodes[children[index].index].sim_count++;
                children[index].sim_count++;
                children[index].total_score += results[player - 1];
            }
            nodes.front().sim_count++;
            children_stack.clear();
        }
        else {
            auto& end = nodes[node_index].children_range.second;
            if (child_index != end - 1) {
                std::swap(children[child_index], children[end - 1]);
            }
            --end;
        }
    }
    reset_path = false;
}

void Tree::apply_move(const reasoner::move& move) {
    reparent_along_move(move);
    reset_path = true;
}