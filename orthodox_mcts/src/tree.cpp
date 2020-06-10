#include "tree.hpp"
#include "node.hpp"
#include "constants.hpp"

Tree::Tree(const reasoner::game_state& initial_state) : MctsTree(initial_state) {}

uint Tree::get_unvisited_child_index(const uint& node_index) {
    static std::vector<uint> children_indices;
    const auto& [fst, lst] = nodes[node_index].children_range;
    const auto unvisited = (lst - fst) - nodes[node_index].sim_count;
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

void Tree::play(reasoner::game_state& state, simulation_result& results) {
    static std::vector<reasoner::move> move_list;
    while (true) {
        state.get_all_moves(cache, move_list);
        if(move_list.empty()) {
            break;
        }
        else {
            std::uniform_int_distribution<uint> dist(0, move_list.size() - 1);
            uint chosen_move = dist(random_numbers_generator);
            state.apply_move(move_list[chosen_move]);
        }
        while (state.get_current_player() == KEEPER) {
            if (not state.apply_any_move(cache)) {
                break;
            }
        }
    }
    for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
}

void Tree::mcts(reasoner::game_state& state, const uint& node_index, simulation_result& results) {
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
            children[child_index].index = create_node(state);
            play(state, results);
        }
        children[child_index].sim_count++;
        children[child_index].total_score += results[current_player - 1];
    }
    nodes[node_index].sim_count++;
}

void Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS - 1);
    reasoner::game_state root_state_copy = root_state;
    static const uint root_index = 0;
    mcts(root_state_copy, root_index, results);
}

void Tree::apply_move(const reasoner::move& move) {
    reparent_along_move(move);
}