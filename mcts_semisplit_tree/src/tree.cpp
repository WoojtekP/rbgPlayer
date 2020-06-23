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
    static std::vector<uint> children_indices;
    const auto [fst, lst] = nodes[node_index].children_range;
    const auto unvisited = (lst - fst) - nodes[node_index].sim_count + 1;
    assert(unvisited > 0);
    children_indices.resize(unvisited);
    uint j = 0;
    for (auto i = fst; i < lst; ++i) {
        if (children[i].index == 0) {
            children_indices[j] = i;
            j++;
        }
    }
    std::uniform_int_distribution<uint> dist(0, unvisited - 1);
    return children_indices[dist(random_numbers_generator)];
}

void Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS - 1);
    static reasoner::game_state state = root_state;
    static std::vector<std::pair<uint,int>> children_stack;  // child index, player
    // uint node_index = children_stack.empty() ? 0 : children[children_stack.back().first].index;
    uint node_index = 0;
    state = root_state;
    while (!nodes[node_index].is_terminal() && nodes[node_index].is_fully_expanded()) {
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
        for (const auto [index, player] : boost::adaptors::reverse(children_stack)) {
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
        auto state_copy = state;
        if (play(state_copy, cache, random_numbers_generator, results)) {
            complete_turn(state);
            auto new_node_index = create_node(state);
            nodes[new_node_index].sim_count = 1;
            children[child_index].index = new_node_index;
            children[child_index].sim_count = 1;
            children[child_index].total_score += results[current_player - 1];
            for (const auto [index, player] : boost::adaptors::reverse(children_stack)) {
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
            children_stack.clear();  // TODO remove it and use children_stack in next simulation
        }
    }
}

void Tree::apply_move(const reasoner::move& move) {
    reparent_along_move(move);
}