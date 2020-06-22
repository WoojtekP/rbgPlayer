#include "tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "constants.hpp"

Tree::Tree(const reasoner::game_state& initial_state) : MctsTree(initial_state) {}

uint Tree::get_unvisited_child_index(const uint node_index) {
    static std::vector<uint> children_indices;
    const auto& [fst, lst] = nodes[node_index].children_range;
    const auto unvisited = (lst - fst) - nodes[node_index].sim_count + 1;
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

void Tree::mcts(reasoner::game_state& state, const uint node_index, simulation_result& results) {
    if (nodes[node_index].is_terminal()) {
        for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
            results[i - 1] = state.get_player_score(i);
        }
    }
    else {
        uint current_player = state.get_current_player();
        uint child_index;
        if (nodes[node_index].is_fully_expanded()) {
            child_index = get_best_uct_child_index(node_index);
            state.apply_move(children[child_index].move);
            complete_turn(state);
            mcts(state, children[child_index].index, results);
        }
        else {
            child_index = get_unvisited_child_index(node_index);
            state.apply_move(children[child_index].move);
            complete_turn(state);
            auto new_node_index = create_node(state);
            children[child_index].index = new_node_index;
            nodes[new_node_index].sim_count = 1;
            play(state, cache, random_numbers_generator, results);
        }
        children[child_index].sim_count++;
        children[child_index].total_score += results[current_player - 1];
    }
    nodes[node_index].sim_count++;
}

void Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS - 1);
    reasoner::game_state root_state_copy = root_state;
    mcts(root_state_copy, 0, results);
}

void Tree::apply_move(const reasoner::move& move) {
    reparent_along_move(move);
}