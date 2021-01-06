#if STATS
#include <cmath>
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


#if STATS
namespace {
void print_move(const reasoner::move_representation& mr) {
    static constexpr uint cell_width = std::floor(std::log10(reasoner::BOARD_SIZE)) + 1;
    static constexpr uint index_width = std::floor(std::log10(reasoner::NUMBER_OF_MODIFIERS)) + 1;
    std::cout << "[";
    for (const auto action : mr) {
        std::cout << std::setw(cell_width) << action.cell << " " << std::setw(index_width) << action.index << " ";
    }
    std::cout << "]";
}
}  // namespace
#endif

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
    static std::vector<reasoner::move> moves;
    state.get_all_moves(cache, moves);
    for (const auto& move : moves) {
        children.emplace_back(move);
    }
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
        const auto child_index = move_chooser.get_unvisited_child_index(children, nodes[node_index], node_sim_count, current_player);
        state.apply_move(children[child_index].move);
        complete_turn(state);
        children_stack.emplace_back(node_index, child_index, current_player);
        auto new_node_index = create_node(state);
        ++state_count;
        state_count += play(state, move_chooser, cache, results);
        children[child_index].index = new_node_index;
    }
    #if RAVE > 0
    [[maybe_unused]] static reasoner::move mv;
    mv.mr.clear();
    for (const auto& [move, player] : move_chooser.get_path()) {
        // TODO: fix using if constexpt ()
        #if defined(ORTHODOX_SIMULATOR)
        // if constexpr (std::is_same<reasoner::move, simulation_move_type>::value) {
        moves_set.insert(move, player - 1);
        // }
        // else {
        #else
        if (move.index > 0) {
            mv.mr.push_back(move);
            if (reasoner::is_switch(move.index)) {
                moves_set.insert(mv, player - 1);
                mv.mr.clear();
            }
        }
        // }
        #endif
    }
    assert(mv.mr.empty());
    #endif
    for (const auto [node_index, child_index, player] : boost::adaptors::reverse(children_stack)) {
        assert(children[child_index].index != 0);
        assert(player != KEEPER);
        ++children[child_index].sim_count;
        children[child_index].total_score += results[player - 1];
        #if MAST > 0
        move_chooser.update_move(children[child_index].move, results, player);
        #endif
        #if RAVE > 0
        moves_set.update_amaf_scores(node_index, child_index, player - 1, results);
        #endif
    }
    ++root_sim_count;
    #if MAST > 0
    if constexpr (!TREE_ONLY) {
        move_chooser.update_all_moves(results);
    }
    #endif
    #if RAVE > 0
    moves_set.reset_moves();
    #endif
    move_chooser.clear_path();
    children_stack.clear();
    return state_count;
}

void Tree::reparent_along_move(const reasoner::move& move) {
    #if STATS
    ++turn_number;
    #endif
    root_state.apply_move(move);
    complete_turn(root_state);
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
    std::cout << "turn number " << turn_number << std::endl;
    #if RAVE > 0
    const double beta = std::sqrt(EQUIVALENCE_PARAMETER / static_cast<double>(3 * root_sim_count + EQUIVALENCE_PARAMETER));
    std::cout << "rave beta " << beta << std::endl;
    #endif
    std::cout << std::fixed << std::setprecision(2);
    const auto [fst, lst] = nodes.front().children_range;
    for (auto i = fst; i < lst; ++i) {
        std::cout << ((i == best_child) ? "* " : "  ");
        std::cout << "sim " << std::setw(6) << children[i].sim_count;
        std::cout << "   avg " << std::setw(6) << static_cast<double>(children[i].total_score) / children[i].sim_count << "   ";
        print_move(children[i].get_actions());
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
