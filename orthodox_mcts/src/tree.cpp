#if STATS
#include <iostream>
#include <iomanip>
#endif

#include <type_traits>

#include <boost/range/adaptor/reversed.hpp>

#include "game_state.hpp"
#include "tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "constants.hpp"


Tree::Tree() {
    complete_turn(root_state);
    create_node(root_state);
    create_children(0, root_state);
}

uint Tree::create_node(GameState& state) {
    if (state.get_current_player() == KEEPER) {
        nodes.emplace_back(0, 0);
    }
    else {
        nodes.emplace_back();
    }
    return nodes.size() - 1;
}

void Tree::create_children(const uint node_index, GameState& state) {
    nodes[node_index].children_range.first = children.size();
    #ifdef SEMISPLIT_SIMULATOR
        static std::vector<reasoner::semimove> semimoves;
        state.get_all_semimoves(cache, semimoves, 1000);
        for (const auto& semimove : semimoves) {
            children.emplace_back(semimove.mr);
        }
    #else
        static std::vector<reasoner::move> moves;
        state.get_all_moves(cache, moves);
        for (const auto& move : moves) {
            children.emplace_back(move);
        }
    #endif
    nodes[node_index].children_range.second = children.size();
}

uint Tree::perform_simulation() {
    static simulation_result results;
    static GameState state = root_state;
    uint state_count = 0;
    uint node_index = 0;
    uint node_sim_count = root_sim_count;
    state = root_state;
    while (!nodes[node_index].is_terminal() && is_node_fully_expanded(node_index)) {
        const auto child_index = get_best_uct_child_index(node_index, node_sim_count);
        const auto current_player = state.get_current_player();
        state.apply_move(children[child_index].move);
        complete_turn(state);
        children_stack.emplace_back(node_index, child_index, current_player);
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
        const auto child_index = get_unvisited_child_index(children, nodes[node_index], node_sim_count, current_player);
        state.apply_move(children[child_index].move);
        complete_turn(state);
        children_stack.emplace_back(node_index, child_index, current_player);
        auto new_node_index = create_node(state);
        ++state_count;
        state_count += play(state, move_chooser, cache, results);
        children[child_index].index = new_node_index;
    }
    #if RAVE > 0
    [[maybe_unused]] reasoner::move_representation mr;
    for (const auto& [move, player] : move_chooser.get_path()) {
        if constexpr (std::is_same<reasoner::move, simulation_move_type>::value) {
            moves_tree[player - 1].insert_or_update(move);
        }
        else {
            mr.insert(mr.end(), move.mr.begin(), move.mr.end());
            if (!move.mr.empty() && reasoner::is_switch(move.mr.back().index)) {
                moves_tree[player - 1].insert_or_update(mr);
                mr.clear();
            }
        }
    }
    #endif
    #if MAST > 0
    uint path_len = children_stack.size();
    if constexpr (!TREE_ONLY) {
        path_len += move_chooser.get_path().size();
    }
    #endif
    for (const auto [node_index, child_index, player] : boost::adaptors::reverse(children_stack)) {
        assert(children[child_index].index != 0);
        assert(player != KEEPER);
        ++children[child_index].sim_count;
        children[child_index].total_score += results[player - 1];
        #if MAST > 0
        move_chooser.update_move(children[child_index].move, results, player, path_len);
        #endif
        #if RAVE > 0
        const auto [fst, lst] = nodes[node_index].children_range;
        for (auto i = fst; i < lst; ++i) {
            if (moves_tree[player - 1].find(children[i].move)) {
                children[i].amaf.score += results[player - 1];
                ++children[i].amaf.count;
            }
        }
        moves_tree[player - 1].insert_or_update(children[child_index].move);
        #endif
    }
    ++root_sim_count;
    #if MAST > 0
    if constexpr (!TREE_ONLY) {
        move_chooser.update_all_moves(results, path_len);
    }
    #endif
    #if RAVE > 0
    for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
        moves_tree[i - 1].reset();
    }
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
    #if MAST > 0
    move_chooser.complete_turn();
    #endif
}

reasoner::move Tree::choose_best_move() {
    assert(nodes.front().is_expanded());
    const auto best_child = get_top_ranked_child_index(0);
    #if STATS
    std::cout << "turn number " << turn_number / 2 + 1 << std::endl;
    #if RAVE > 0
    const double beta = std::sqrt(EQUIVALENCE_PARAMETER / static_cast<double>(3 * root_sim_count + EQUIVALENCE_PARAMETER));
    std::cout << "rave beta " << beta << std::endl;
    #endif
    std::cout << std::fixed << std::setprecision(2);
    const auto [fst, lst] = nodes.front().children_range;
    for (auto i = fst; i < lst; ++i) {
        char prefix = (i == best_child) ? '*' : ' ';
        std::cout << prefix << " sim " << std::setw(4) << children[i].sim_count;
        std::cout << "   avg " << std::setw(6) << static_cast<double>(children[i].total_score) / children[i].sim_count << "   [";
        for (const auto action : children[i].get_actions()) {
            std::cout << std::setw(3) << action.cell << " " << std::setw(3) << action.index << " ";
        }
        std::cout << "]";
        #if RAVE > 0
        std::cout << " amaf_avg " << (static_cast<double>(children[i].amaf.score) / children[i].amaf.count) << " amaf_count " << children[i].amaf.count;
        if (children[i].amaf.count > 0) {
            double priority = (1.0 - beta)*static_cast<double>(children[i].total_score) / children[i].sim_count + beta * (children[i].amaf.score / children[i].amaf.count);
            std::cout << " (avg_with_rave " << priority << ")";
        }   
        #endif
        std::cout << std::endl;
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
