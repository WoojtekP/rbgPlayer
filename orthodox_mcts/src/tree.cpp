#include "tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "constants.hpp"


Tree::Tree(const reasoner::game_state& initial_state) 
    : MctsTree(initial_state)
    #if MAST > 0
    , move_chooser(moves)
    #endif
{
    complete_turn(root_state);
    create_node(root_state);
}

uint Tree::create_node(reasoner::game_state& state) {
    static std::vector<reasoner::move> move_list;
    state.get_all_moves(cache, move_list);
    auto child_count = move_list.size();
    uint new_child_index = children.size();
    nodes.emplace_back(new_child_index, child_count);
    for (const auto& move : move_list) {
        children.emplace_back(move);
    }
    return nodes.size() - 1;
}

void Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS - 1);
    static reasoner::game_state state = root_state;
    state = root_state;
    uint node_index = 0;
    while (!nodes[node_index].is_terminal() && is_node_fully_expanded(node_index)) {
        const auto child_index = get_best_uct_child_index(node_index);
        const auto current_player = state.get_current_player();
        state.apply_move(children[child_index].move);
        complete_turn(state);
        children_stack.emplace_back(child_index, current_player);
        node_index = children[child_index].index;
        if (node_index == 0) {
            assert(false);
        }
    }
    if (nodes[node_index].is_terminal()) {
        for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
            results[i - 1] = state.get_player_score(i);
        }
    }
    else {
        const auto current_player = state.get_current_player();
        const auto child_index = get_unvisited_child_index(node_index, current_player);
        state.apply_move(children[child_index].move);
        complete_turn(state);
        auto new_node_index = create_node(state);
        play(state, move_chooser, cache, results);
        nodes[new_node_index].sim_count = 1;
        children[child_index].index = new_node_index;
        children[child_index].sim_count = 1;
        children[child_index].total_score += results[current_player - 1];
    }
    [[maybe_unused]] const uint depth = children_stack.size() + move_chooser.get_path().size();
    for (const auto [index, player] : children_stack) {
        nodes[children[index].index].sim_count++;
        children[index].sim_count++;
        children[index].total_score += results[player - 1];
        #if MAST > 0
        moves[player - 1].insert_or_update(children[index].get_actions(), results[player - 1], depth);
        #endif
    }
    #if MAST > 0
    for (const auto& [move, player] : move_chooser.get_path()) {
        moves[player - 1].insert_or_update(move_chooser.extract_actions(move), results[player - 1], depth);
    }
    move_chooser.clear_path();
    #endif
    nodes.front().sim_count++;
    children_stack.clear();
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
    root_at_index(root_index);
    children_stack.clear();
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