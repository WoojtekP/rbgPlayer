#include"tree.hpp"
#include"constants.hpp"
#include"node.hpp"


tree::tree(const reasoner::game_state& initial_state) :
    root(initial_state),
    random_numbers_generator(std::random_device{}()) {}

node* tree::traverse(node* state) {
    if (state->is_leaf()) {
        return state;
    }
    if (state->is_fully_expanded()) {
        return traverse(state->get_best_uct());
    }
    return state->get_random_child(random_numbers_generator);
}

simulation_result tree::play(const node* state) {
    auto game_state = state->get_game_state();
    auto cache = state->get_cache();
    auto move_list = state->get_move_list();
    while (true) {
        if(move_list.empty()) {
            break;
        }
        else {
            std::uniform_int_distribution<int> dist(0, move_list.size()-1);
            uint chosen_move = dist(random_numbers_generator);
            game_state.apply_move(move_list[chosen_move]);
        }
        while (game_state.get_current_player() == KEEPER){
            if (not game_state.apply_any_move(cache)) {
                break;
            }
        }
        game_state.get_all_moves(cache, move_list);
    }
    for (int i = 0; i < reasoner::NUMBER_OF_VARIABLES; ++i) {
        if (game_state.get_variable_value(i) == 100) {
            return i;
        }
    }
    return -1;  // no winner
}

void tree::backpropagate(node* state, const simulation_result& winner) {
    state->increment_simulations_counter();
    if (state->get_current_player() == winner) {
        state->increment_wins_counter();
    }
    if (!state->is_root()) {
        backpropagate(state->get_parent(), winner);
    }
}

game_status_indication tree::get_status(int player_index) const {
    if (root.is_leaf()) {
        return end_game;
    }
    return root.get_current_player() == (player_index + 1) ? own_turn : opponent_turn;
}

int tree::get_simulations_count() {
    return root.get_simulations_count();
}

reasoner::move tree::choose_best_move() {
    return root.choose_best_move();
}

void tree::reparent_along_move(const reasoner::move& move) {
    root = root.get_node_by_move(move);
    root.set_root();
}

void tree::perform_simulation() {
    node* leaf = traverse(&root);
    auto winner = play(leaf);
    backpropagate(leaf, winner);
}
