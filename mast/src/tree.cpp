#include "tree.hpp"
#include "node.hpp"
#include "constants.hpp"

#include <iostream>

Tree::Tree(const reasoner::game_state& initial_state) : 
    root_state(initial_state),
    random_numbers_generator(std::random_device{}()) {
    complete_turn(root_state);
    create_node(root_state);
    nodes.reserve(4 * MEBIBYTE / sizeof(Node));
    children.reserve(4 * MEBIBYTE / sizeof(uint));
}

uint Tree::create_node(reasoner::game_state& state) {
    static std::vector<reasoner::move> move_list;
    state.get_all_moves(Node::cache, move_list);
    auto child_count = move_list.size();
    uint new_child_index = children.size();
    nodes.emplace_back(new_child_index, child_count);
    children.reserve(new_child_index + child_count);
    for (const auto& move : move_list) {
        children.emplace_back(move);
    }
    return nodes.size() - 1;
}

void Tree::play(reasoner::game_state& state, simulation_result& results) {
    static std::vector<reasoner::move> move_list;
    static std::vector<double> weights;
    while (true) {
        state.get_all_moves(Node::cache, move_list);
        if(move_list.empty()) {
            break;
        }
        else {
            const auto current_player = state.get_current_player();
            const auto size = move_list.size();
            weights.resize(size);
            for (uint i = 0; i < size; ++i) {
                weights[i] = std::exp(moves[current_player - 1].get_score_or_default_value(move_list[i]) / tau);
            }
            std::discrete_distribution<> dist(weights.begin(), weights.end());
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

void Tree::mcts(reasoner::game_state& state, uint node_index, simulation_result& results, uint& depth) {
    auto current_player = state.get_current_player();
    if (nodes[node_index].is_terminal()) {
        play(state, results);
    }
    else {
        depth++;
        uint child_index;
        if (nodes[node_index].is_fully_expanded()) {
            child_index = get_best_uct_child_index(node_index);
            state.apply_move(children[child_index].move);
            complete_turn(state);
            mcts(state, children[child_index].index, results, depth);
        }
        else {
            child_index = get_unvisited_child_index(node_index, current_player);
            state.apply_move(children[child_index].move);
            complete_turn(state);
            children[child_index].index = create_node(state);
            play(state, results);
        }
        children[child_index].sim_count++;
        children[child_index].total_score += results[current_player - 1];
        double weight = 1.0 / depth;
        double score = results[current_player - 1] / depth;
        moves[current_player - 1].insert_or_update(children[child_index].move, weight, score);
    }
    nodes[node_index].sim_count++;
}

void Tree::complete_turn(reasoner::game_state& state) const {
    while (state.get_current_player() == KEEPER && state.apply_any_move(Node::cache));
}

uint Tree::get_best_uct_child_index(const uint& node_index) {
    static std::vector<uint> best_children_indices;
    const auto& [fst, lst] = nodes[node_index].children_range;
    best_children_indices.resize(1);
    best_children_indices[0] = fst;
    double logN = std::log(nodes[node_index].sim_count);
    double max_priority = children[fst].total_score / EXPECTED_MAX_SCORE / children[fst].sim_count +
                         EXPLORATION_CONSTANT * std::sqrt(logN / children[fst].sim_count);
    for (uint i = fst + 1; i < lst; ++i) {
        double priority = children[i].total_score / EXPECTED_MAX_SCORE / children[i].sim_count +
                          EXPLORATION_CONSTANT * std::sqrt(logN / children[i].sim_count);
        if (priority > max_priority) {
            max_priority = priority;
            best_children_indices.resize(1);
            best_children_indices[0] = i;
        }
        else if (priority == max_priority) {
            best_children_indices.push_back(i);
        }
    }
    uint best_child_index = best_children_indices.front();
    if (best_children_indices.size() > 1) {
        std::uniform_int_distribution<> dist(0, best_children_indices.size() - 1);
        best_child_index = best_children_indices[dist(random_numbers_generator)];
    }
    return best_child_index;
}

uint Tree::get_unvisited_child_index(const uint& node_index, const uint& player_index) {
    static std::vector<uint> children_indices;
    static std::vector<double> weights;
    const auto& [fst, lst] = nodes[node_index].children_range;
    const auto unvisited = (lst - fst) - nodes[node_index].sim_count;
    children_indices.resize(unvisited);
    weights.resize(unvisited);
    uint j = 0;
    for (auto i = fst; i < lst; ++i) {
        if (children[i].index == 0) {
            children_indices[j] = i;
            weights[j] = std::exp(moves[player_index - 1].get_score_or_default_value(children[i].move) / tau);
            j++;
        }
    }
    assert(j == unvisited);
    uint child_index = children_indices.front();
    if (unvisited > 1) {
        std::discrete_distribution<> dist(weights.begin(), weights.end());
        child_index = children_indices[dist(random_numbers_generator)];
    }
    return child_index;
}

game_status_indication Tree::get_status(const uint& player_index) const {
    if (nodes.front().is_terminal()) {
        return end_game;
    }
    return (uint)root_state.get_current_player() == (player_index + 1) ? own_turn : opponent_turn;
}

reasoner::move Tree::choose_best_move() {
    const auto& [fst, lst] = nodes.front().children_range;
    auto max_sim = children[fst].sim_count;
    auto best_node = fst;
    for (auto i = fst + 1; i < lst; ++i) {
        if (children[i].sim_count > max_sim) {
            max_sim = children[i].sim_count;
            best_node = i;
        }
    }
    return children[best_node].move;
}

void Tree::reparent_along_move(const reasoner::move& move) {
    root_state.apply_move(move);
    complete_turn(root_state);
    uint root_index = 0;
    auto [fst, lst] = nodes.front().children_range;
    for ( ; fst < lst; ++fst) {
        if (children[fst].move == move) {
            root_index = children[fst].index;
            break;
        }
    }
    if (root_index == 0) {
        nodes.clear();
        children.clear();
        create_node(root_state);
        return;
    }
    static std::vector<Node> nodes_tmp;
    static std::vector<Child> children_tmp;
    nodes_tmp.reserve(nodes.size());
    children_tmp.reserve(children.size());
    fix_tree(nodes_tmp, children_tmp, root_index);
    nodes = nodes_tmp;
    children = children_tmp;
    nodes_tmp.clear();
    children_tmp.clear();
}

uint Tree::fix_tree(std::vector<Node>& nodes_tmp, std::vector<Child>& children_tmp, uint index) {
    auto new_index = nodes_tmp.size();
    nodes_tmp.push_back(nodes[index]);
    auto first_child_index = children_tmp.size();
    const auto& [fst, lst] = nodes[index].children_range;
    auto child_count = lst - fst;
    nodes_tmp[new_index].children_range =  std::make_pair(first_child_index, first_child_index + child_count);
    children_tmp.resize(first_child_index + child_count);
    for (uint i = 0; i < child_count; ++i) {
        children_tmp[first_child_index + i] = children[fst + i];
        if (children[fst + i].index == 0) {
            continue;
        }
        children_tmp[first_child_index + i].index = fix_tree(nodes_tmp, children_tmp, children[fst + i].index);
    }
    return new_index;
}

void Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS);
    reasoner::game_state root_state_copy = root_state;
    uint depth = 0;
    mcts(root_state_copy, 0, results, depth);
}
