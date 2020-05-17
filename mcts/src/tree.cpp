#include"tree.hpp"
#include"node.hpp"
#include"constants.hpp"

Tree::Tree(const reasoner::game_state& initial_state) : 
    root_state(initial_state),
    random_numbers_generator(std::random_device{}()) {
    complete_turn(root_state);
    create_node(root_state);
}

uint Tree::create_node(reasoner::game_state& state) {
    uint new_child_index = children.size();
    nodes.emplace_back(state, new_child_index);
    auto range = nodes.back().get_children();
    auto child_counter = range.second - range.first;
    children.insert(children.end(), child_counter, 0);
    return nodes.size() - 1;
}

void Tree::play(reasoner::game_state& state, simulation_result& results) {
    static std::vector<reasoner::move> move_list;
    while (true) {
        state.get_all_moves(Node::cache, move_list);
        if(move_list.empty()) {
            break;
        }
        else {
            std::uniform_int_distribution<> dist(0, move_list.size() - 1);
            uint chosen_move = dist(random_numbers_generator);
            state.apply_move(move_list[chosen_move]);
        }
        while (state.get_current_player() == KEEPER) {
            if (not state.apply_any_move(Node::cache)) {
                break;
            }
        }
    }
    for (int i = 1; i <= reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
}

void Tree::mcts(reasoner::game_state& state, uint node_index, simulation_result& results) {
    auto &node = nodes[node_index];
    auto current_player = state.get_current_player();
    uint child_index = 0;
    if (node.is_terminal()) {
        play(state, results);
    }
    else if (node.is_fully_expanded()) {
        auto [move, index] = node.get_best_uct_and_child_index(random_numbers_generator);
        child_index = index;
        state.apply_move(move);
        complete_turn(state);
        mcts(state, children[child_index], results);
    }
    else {
        auto [move, index] = node.get_random_move_and_child_index(random_numbers_generator);
        child_index = index;
        state.apply_move(move);
        complete_turn(state);
        children[child_index] = create_node(state);
        auto current_child_player = state.get_current_player();
        play(state, results);
        nodes[children[child_index]].update_stats(current_child_player, 0, results);
    }
    nodes[node_index].update_stats(current_player, child_index, results);
}

void Tree::complete_turn(reasoner::game_state& state) const {
    while (state.get_current_player() == KEEPER && state.apply_any_move(Node::cache));
}

game_status_indication Tree::get_status(int player_index) const {
    if (nodes[root_index].is_terminal()) {
        return end_game;
    }
    return root_state.get_current_player() == (player_index + 1) ? own_turn : opponent_turn;
}

uint Tree::get_simulation_counter() {
    return nodes[root_index].get_simulation_counter();
}

reasoner::move Tree::choose_best_move() {
    return nodes[root_index].choose_best_move();
}

void Tree::reparent_along_move(const reasoner::move& move) {
    // TODO przebudować drzewo usuwając niepotrzebne węzły a następnie
    // usunąć zmienną root_index (korzeń powinien być zawsze pierwszy w wektorze węzłów)
    root_state.apply_move(move);
    complete_turn(root_state);
    auto child_index = nodes[root_index].get_child_index_by_move(move);
    root_index = children[child_index];
    if (root_index == 0) {
        nodes.clear();
        children.clear();
        create_node(root_state);
        return;
    }
}

void Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS);
    reasoner::game_state root_state_copy = root_state;
    mcts(root_state_copy, root_index, results);
}
