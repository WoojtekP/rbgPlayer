#if STATS
#include <iostream>
#include <iomanip>
#endif

#include "tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "constants.hpp"


Tree::Tree(const reasoner::game_state& initial_state) : MctsTree(initial_state) {
    complete_turn(root_state);
    create_node(root_state);
    create_children(0, root_state);
}

uint Tree::create_node(reasoner::game_state& state) {
    if (state.get_current_player() == KEEPER) {
        nodes.emplace_back(0, 0);
    }
    else {
        nodes.emplace_back();
    }
    return nodes.size() - 1;
}

void Tree::create_children(const uint node_index, reasoner::game_state& state) {
    static std::vector<reasoner::move> move_list;
    state.get_all_moves(cache, move_list);
    nodes[node_index].children_range.first = children.size();
    for (const auto& move : move_list) {
        children.emplace_back(move);
    }
    nodes[node_index].children_range.second = children.size();
}

uint Tree::perform_simulation() {
    static simulation_result results;
    static reasoner::game_state state = root_state;
    uint state_count = 0;;
    uint node_index = 0;
    uint node_sim_count = root_sim_count;
    state = root_state;
    while (!nodes[node_index].is_terminal() && is_node_fully_expanded(node_index)) {
        const auto child_index = get_best_uct_child_index(node_index, node_sim_count);
        const auto current_player = state.get_current_player();
        state.apply_move(children[child_index].move);
        complete_turn(state);
        children_stack.emplace_back(child_index, current_player);
        node_index = children[child_index].index;
        node_sim_count = children[child_index].sim_count;
        assert(node_index != 0);
        ++state_count;
    }
    if (!nodes[node_index].is_expanded()) {
        create_children(node_index, state);
    }
    if (nodes[node_index].is_terminal()) {
        get_scores_from_state(state, results);
    }
    else {
        const auto current_player = state.get_current_player();
        const auto child_index = move_chooser.get_unvisited_child_index(children, nodes[node_index], node_sim_count, current_player);
        state.apply_move(children[child_index].move);
        complete_turn(state);
        auto new_node_index = create_node(state);
        ++state_count;
        state_count += play(state, move_chooser, cache, results);
        children[child_index].index = new_node_index;
        children[child_index].sim_count = 1;
        children[child_index].total_score += results[current_player - 1];
    }
    [[maybe_unused]] const uint path_len = children_stack.size() + move_chooser.get_path().size();  // TODO calculate only nodal-depth
    [[maybe_unused]] uint depth = 0;
    node_index = 0;
    for (const auto [child_index, player] : children_stack) {
        assert(children[child_index].index != 0);
        assert(player != KEEPER);
        children[child_index].sim_count++;
        children[child_index].total_score += results[player - 1];
        #if MAST > 0
        move_chooser.update_move(children[child_index].move, results, player, path_len);
        #endif
        #if RAVE > 0
        const auto [fst, lst] = nodes[children[child_index].index].children_range;
        for (auto i = fst; i < lst; ++i) {
            if (i == child_index) {
                continue;
            }
            bool found = false;
            for (uint j = depth; j < children_stack.size(); ++j) {
                const auto [new_child_index, new_player] = children_stack[j];
                if (new_player == player && children[new_child_index].move == children[i].move) {
                    ++children[i].amaf_count;
                    children[i].amaf_score += results[new_player - 1];
                    found = true;
                    break;
                }
            }
            if (!found) {
                for (const auto [move, new_player] : move_chooser.get_path()) {
                    if (new_player == player && children[i].move == move) {
                        ++children[i].amaf_count;
                        children[i].amaf_score += results[new_player - 1];
                        break;
                    }
                }
            }
        }
        ++depth;
        #endif
    }
    ++root_sim_count;
    #if MAST > 0
    move_chooser.update_all_moves(results, path_len);
    #endif
    move_chooser.clear_path();
    children_stack.clear();
    return state_count;
}

void Tree::reparent_along_move(const reasoner::move& move) {
    #if STATS
    auto current_player = root_state.get_current_player();
    #endif
    root_state.apply_move(move);
    complete_turn(root_state);
    #if STATS
    if (current_player != root_state.get_current_player())
        ++turn_number;
    #endif
    uint root_index = 0;
    const auto [fst, lst] = nodes.front().children_range;
    for (auto i = fst ; i < lst; ++i) {
        if (children[i].move == move) {
            root_index = children[i].index;
            root_sim_count = children[i].sim_count;
            break;
        }
    }
    if (root_index == 0) {
        root_sim_count = 0;
        nodes.clear();
        children.clear();
        create_node(root_state);
    }
    else {
        root_at_index(root_index);
    }
    if (!nodes.front().is_expanded()) {
        create_children(0, root_state);
    }
    children_stack.clear();
}

reasoner::move Tree::choose_best_move() {
    static std::vector<uint> children_indices;
    assert(nodes.front().is_expanded());
    const auto [fst, lst] = nodes.front().children_range;
    auto max_sim = children[fst].sim_count;
    children_indices.resize(1);
    children_indices[0] = fst;
    for (auto i = fst + 1; i < lst; ++i) {
        if (children[i].sim_count > max_sim) {
            max_sim = children[i].sim_count;
            children_indices.resize(1);
            children_indices[0] = i;
        }
        else if (children[i].sim_count == max_sim) {
            children_indices.push_back(i);
        }
    }
    auto best_child = children_indices.front();
    auto best_score = children[best_child].total_score;
    for (const auto child_index : children_indices) {
        if (children[child_index].total_score > best_score) {
            best_score = children[child_index].total_score;
            best_child = child_index;
        }
    }
    #if STATS
    std::cout << "turn number " << turn_number / 2 + 1 << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    for (auto i = fst; i < lst; ++i) {
        char prefix = (i == best_child) ? '*' : ' ';
        std::cout << prefix << " sim " << std::setw(4) << children[i].sim_count;
        std::cout << "   avg " << std::setw(6) << static_cast<double>(children[i].total_score) / children[i].sim_count << "   [";
        for (const auto action : children[i].get_actions()) {
            std::cout << std::setw(3) << action.cell << " " << std::setw(3) << action.index << " ";
        }
        std::cout << "]" << std::endl;
    }
    std::cout << std::endl;
    #endif
    return children[best_child].move;
}

game_status_indication Tree::get_status(const int player_index) {
    if (nodes.front().is_terminal()) {
        return end_game;
    }
    return root_state.get_current_player() == (player_index + 1) ? own_turn : opponent_turn;
}
