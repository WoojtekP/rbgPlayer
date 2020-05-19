#include"tree.hpp"
#include"node.hpp"
#include"constants.hpp"

Tree::Tree(const reasoner::game_state& initial_state) : 
    root_state(initial_state),
    random_numbers_generator(std::random_device{}()) {
    complete_turn(root_state);
    create_node(root_state);
    nodes.reserve(4 * MEBIBYTE / sizeof(Node));
    children.reserve(4 * MEBIBYTE / sizeof(uint));
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
    if (nodes.front().is_terminal()) {
        return end_game;
    }
    return root_state.get_current_player() == (player_index + 1) ? own_turn : opponent_turn;
}

reasoner::move Tree::choose_best_move() {
    return nodes.front().choose_best_move();
}

void Tree::reparent_along_move(const reasoner::move& move) {
    root_state.apply_move(move);
    complete_turn(root_state);
    auto root_index = children[nodes.front().get_child_index_by_move(move)];
    if (root_index == 0) {
        nodes.resize(0);
        children.resize(0);
        create_node(root_state);
        return;
    }
    static std::vector<Node> nodes_tmp;
    static std::vector<uint> children_tmp;
    nodes_tmp.reserve(nodes.size());
    children_tmp.reserve(children.size());
    fix_tree(nodes_tmp, children_tmp, root_index);
    nodes = nodes_tmp;
    children = children_tmp;
    nodes_tmp.resize(0);
    children_tmp.resize(0);
}

uint Tree::fix_tree(std::vector<Node>& nodes_tmp, std::vector<uint>& children_tmp, uint index) {
    auto new_index = nodes_tmp.size();
    nodes_tmp.push_back(nodes[index]);
    auto first_child_index = children_tmp.size();
    nodes_tmp[new_index].set_children(first_child_index);
    auto [fst, lst] = nodes[index].get_children();
    children_tmp.insert(children_tmp.end(), lst - fst, 0);
    for (uint i = 0; i < lst - fst; ++i) {
        if (children[fst + i] == 0) {
            break;
        }
        children_tmp[first_child_index + i] = fix_tree(nodes_tmp, children_tmp, children[fst + i]);
    }
    return new_index;
}

void Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS);
    reasoner::game_state root_state_copy = root_state;
    mcts(root_state_copy, 0, results);
}
