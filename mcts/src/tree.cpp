#include"tree.hpp"
#include"node.hpp"
#include"constants.hpp"

Tree::Tree(const reasoner::game_state& initial_state) : 
    root_state(initial_state),
    random_numbers_generator(std::random_device{}()) {
    complete_turn(root_state);
    root_index = create_node(root_state);
}

int Tree::create_node(reasoner::game_state& state) {
    int new_child_index = children.size();
    nodes.emplace_back(state, new_child_index);
    auto range = nodes.back().get_children();
    auto child_counter = range.second - range.first;
    children.insert(children.end(), child_counter, -1);
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
            std::uniform_int_distribution<int> dist(0, move_list.size()-1);
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
        results[i-1] = state.get_player_score(i);
    }
}

void Tree::mcts(reasoner::game_state& state, int node_index, simulation_result& results) {
    auto &node = nodes[node_index];
    auto current_player = state.get_current_player();
    if (node.is_leaf()) {
        play(state, results);
    }
    else if (node.is_fully_expanded()) {
        auto child_index = get_best_uct_and_change_state(node, state);
        mcts(state, children[child_index], results);
    }
    else {
        auto child_index = get_random_child_and_change_state(node, state);
        auto current_child_player = state.get_current_player();
        play(state, results);
        nodes[children[child_index]].update_stats(current_child_player, results);
    }
    nodes[node_index].update_stats(current_player, results);
}

void Tree::complete_turn(reasoner::game_state& state) const {
    while (state.get_current_player() == KEEPER && state.apply_any_move(Node::cache));
}

int Tree::get_best_uct_and_change_state(const Node& node, reasoner::game_state& state) {
    static std::vector<int> best_children_indices;
    best_children_indices.clear();
    double logN = std::log(node.get_simulation_counter());
    auto [fst, lst] = node.get_children();
    best_children_indices.push_back(fst);
    double maxPriority = nodes[children[fst]].get_total_score() / nodes[children[fst]].get_simulation_counter() +
                         EXPLORATION_CONSTANT * std::sqrt(logN / nodes[children[fst]].get_simulation_counter());
    for (int i = fst+1; i < lst; ++i) {
        int node_index = children[i];
        double priority = nodes[node_index].get_total_score() / nodes[node_index].get_simulation_counter() +
                          EXPLORATION_CONSTANT * std::sqrt(logN / nodes[node_index].get_simulation_counter());
        if (priority > maxPriority) {
            maxPriority = priority;
            best_children_indices.resize(1);
            best_children_indices[0] = i;
        }
        else if (priority == maxPriority) {
            best_children_indices.push_back(i);
        }
    }
    int best_child_index = best_children_indices.front();
    if (best_children_indices.size() > 1) {
        std::uniform_int_distribution<int> dist(0, best_children_indices.size() - 1);
        best_child_index = best_children_indices[dist(random_numbers_generator)];
    }
    reasoner::move best_move = node.get_move_by_child_index(best_child_index);
    state.apply_move(best_move);
    complete_turn(state);
    return best_child_index;
}

int Tree::get_random_child_and_change_state(Node& node, reasoner::game_state& state) {
    auto [move, child_index] = node.get_random_move_and_child_index(random_numbers_generator);
    assert(children[child_index] == -1);
    state.apply_move(move);
    complete_turn(state);
    int node_index = create_node(state);
    children[child_index] = node_index;
    return child_index;
}

game_status_indication Tree::get_status(int player_index) const {
    if (nodes[root_index].is_leaf()) {
        return end_game;
    }
    return root_state.get_current_player() == (player_index + 1) ? own_turn : opponent_turn;
}

int Tree::get_simulation_counter() {
    return nodes[root_index].get_simulation_counter();
}

reasoner::move Tree::choose_best_move() {
    // TODO sprawdzać czy przechodzimy po rozwiniętych dzieciach
    auto [fst, lst] = nodes[root_index].get_children();
    int maxSimulations = nodes[children[fst]].get_simulation_counter();
    int best_child_index = fst;
    for (int i = fst+1; i < lst; ++i) {
        int simulations = nodes[children[i]].get_simulation_counter();
        if (simulations > maxSimulations) {
            maxSimulations = simulations;
            best_child_index = i;
        }
    }
    return nodes[root_index].get_move_by_child_index(best_child_index);
}

void Tree::reparent_along_move(const reasoner::move& move) {
    // TODO przebudować drzewo usuwając niepotrzebne węzły a następnie
    // usunąć zmienną root_index (korzeń powinien być zawsze pierwszy w wektorze węzłów)
    root_state.apply_move(move);
    complete_turn(root_state);
    auto child_index = nodes[root_index].get_child_index_by_move(move);
    root_index = children[child_index];
}

void Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS);
    reasoner::game_state root_state_copy = root_state;
    mcts(root_state_copy, root_index, results);
}
