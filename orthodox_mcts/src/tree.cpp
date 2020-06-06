#include "tree.hpp"
#include "node.hpp"
#include "constants.hpp"

Tree::Tree(const reasoner::game_state& initial_state) : MctsTree(initial_state) {}

uint Tree::get_unvisited_child_index(const uint& node_index, const uint&) {
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
    assert(j == unvisited);
    uint child_index = children_indices.front();
    if (unvisited > 1) {
        std::uniform_int_distribution<> dist(0, unvisited - 1);
        child_index = children_indices[dist(random_numbers_generator)];
    }
    return child_index;
}

void Tree::play(reasoner::game_state& state, simulation_result& results) {
    static std::vector<reasoner::move> move_list;
    while (true) {
        state.get_all_moves(cache, move_list);
        if(move_list.empty()) {
            break;
        }
        else {
            std::uniform_int_distribution<> dist(0, move_list.size() - 1);
            uint chosen_move = dist(random_numbers_generator);
            state.apply_move(move_list[chosen_move]);
        }
        while (state.get_current_player() == KEEPER) {
            if (not state.apply_any_move(cache)) {
                break;
            }
        }
    }
    for (int i = 1; i <= reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
}

void Tree::mcts(reasoner::game_state& state, uint node_index, simulation_result& results) {
    if (nodes[node_index].is_terminal()) {
        play(state, results);
    }
    else {
        uint current_player = state.get_current_player();
        uint child_index;
        if (nodes[node_index].is_fully_expanded()) {
            child_index = get_best_uct_child_index(node_index);
            state.apply_move(children[child_index].move);
            complete_turn(state);
            Tree::mcts(state, children[child_index].index, results);
        }
        else {
            child_index = get_unvisited_child_index(node_index, current_player);
            state.apply_move(children[child_index].move);
            complete_turn(state);
            children[child_index].index = create_node(state);
            Tree::play(state, results);
        }
        children[child_index].sim_count++;
        children[child_index].total_score += results[current_player - 1];
    }
    nodes[node_index].sim_count++;
}
