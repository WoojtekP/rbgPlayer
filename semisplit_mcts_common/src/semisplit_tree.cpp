#if STATS
#include <cmath>
#include <iostream>
#include <iomanip>
#endif

#include <iostream>

#include <boost/range/adaptor/reversed.hpp>

#include "game_state.hpp"
#include "semisplit_tree.hpp"
#include "semisplit_node.hpp"
#include "simulator.hpp"
#include "constants.hpp"
#include "random_generator.hpp"


namespace {
    std::vector<reasoner::action_representation> legal_actions[MAX_SEMIDEPTH];

#if RAVE >= 2
    bool end_of_context(const reasoner::action_representation action) {
        return reasoner::is_switch(action.index);
    }
#endif
}  // namespace

#if STATS
void SemisplitTree::print_node_stats(const semisplit_child& child) {
    std::cout << "sim " << std::setw(6) << child.sim_count;
    std::cout << "   avg " << std::setw(6) << std::setprecision(2) << static_cast<double>(child.total_score) / child.sim_count << "   ";
}

void SemisplitTree::print_action(const reasoner::action_representation action) {
    static constexpr uint cell_width = std::floor(std::log10(reasoner::BOARD_SIZE)) + 1;
    static constexpr uint index_width = std::floor(std::log10(std::max(reasoner::AUTOMATON_SIZE, reasoner::NUMBER_OF_MODIFIERS))) + 2;
    std::cout << std::setw(cell_width) << action.cell << " " << std::setw(index_width) << action.index << " ";
}
#endif

SemisplitTree::SemisplitTree() {
    complete_turn(root_state);
    const auto status = has_nodal_successor(root_state) ? node_status::nonterminal : node_status::terminal;
    create_node(root_state, status);
    create_children(0, root_state);
}

uint SemisplitTree::create_node(GameState& state, const node_status status) {
    if (status == node_status::terminal || state.get_current_player() == KEEPER) {
        assert(state.is_nodal());
        nodes.emplace_back(0, 0, true, node_status::terminal);
    }
    else {
        nodes.emplace_back(state.is_nodal(), status);
    }
    return nodes.size() - 1;
}

void SemisplitTree::create_children(const uint node_index, GameState& state) {
    static std::vector<reasoner::action_representation> actions;
    state.get_all_actions(cache, actions);
    if (actions.empty() || nodes[node_index].status == node_status::terminal) {
        assert(state.is_nodal());
        nodes[node_index].status = node_status::terminal;
        nodes[node_index].children_range = {0, 0};
    }
    else {
        assert(state.get_current_player() != KEEPER);
        nodes[node_index].children_range.first = children.size();
        for (const auto& action : actions) {
            children.emplace_back(action);
        }
        nodes[node_index].children_range.second = children.size();
    }
}

bool SemisplitTree::has_nodal_successor(GameState& state, uint semidepth) {
    if (state.get_current_player() == KEEPER) {
        return false;
    }
    state.get_all_actions(cache, legal_actions[semidepth]);
    while (!legal_actions[semidepth].empty()) {
        auto ri = state.apply_action_with_revert(legal_actions[semidepth].back());
        if (state.is_nodal()) {
            state.revert(ri);
            return true;
        }
        if (has_nodal_successor(state, semidepth+1)) {
            state.revert(ri);
            return true;
        }
        legal_actions[semidepth].pop_back();
        state.revert(ri);
    }
    return false;
}

bool SemisplitTree::save_path_to_nodal_state(GameState& state, std::vector<reasoner::action_representation>& path, uint semidepth) {
    if (state.is_nodal()) {
        move_chooser.reset_context();
        return true;
    }
    state.get_all_actions(cache, legal_actions[semidepth]);
    while (!legal_actions[semidepth].empty()) {
        const auto current_player = state.get_current_player();
        const auto chosen_action = move_chooser.get_random_move(legal_actions[semidepth], current_player);
        const auto ri = state.apply_action_with_revert(legal_actions[semidepth][chosen_action]);
        move_chooser.switch_context(legal_actions[semidepth][chosen_action], current_player);
        path.emplace_back(legal_actions[semidepth][chosen_action]);
        if (save_path_to_nodal_state(state, path, semidepth + 1)) {
            state.revert(ri);
            return true;
        }
        legal_actions[semidepth][chosen_action] = legal_actions[semidepth].back();
        legal_actions[semidepth].pop_back();
        path.pop_back();
        state.revert(ri);
        move_chooser.revert_context();
    }
    return false;
}

bool SemisplitTree::random_walk_to_nodal(GameState& state, std::vector<reasoner::action_representation>& path, uint semidepth) {
    if (state.is_nodal()) {
        return true;
    }
    state.get_all_actions(cache, legal_actions[semidepth]);
    while (!legal_actions[semidepth].empty()) {
        const auto chosen_action = RBGRandomGenerator::get_instance().uniform_choice(legal_actions[semidepth].size());
        auto ri = state.apply_action_with_revert(legal_actions[semidepth][chosen_action]);
        path.push_back(legal_actions[semidepth][chosen_action]);
        if (random_walk_to_nodal(state, path, semidepth+1)) {
            return true;
        }
        legal_actions[semidepth][chosen_action] = legal_actions[semidepth].back();
        legal_actions[semidepth].pop_back();
        state.revert(ri);
        path.pop_back();
    }
    return false;
}

uint SemisplitTree::perform_simulation() {
    static simulation_result results;
    static GameState state = root_state;
    static std::vector<reasoner::action_representation> path;
    children_stack.clear();
    move_chooser.clear_path();
    path.clear();
    uint state_count = 0;
    uint node_index = 0;
    uint node_sim_count = root_sim_count;
    int current_player;
    state = root_state;
    #if RAVE > 0
    static std::vector<int> context_stack;
    context_stack.clear();
    int context = 0;
    #endif
    while (!nodes[node_index].is_terminal() && is_node_fully_expanded(node_index)) {
        current_player = state.get_current_player();
        const auto child_index = get_best_uct_child_index(node_index, node_sim_count);
        assert(child_index >= nodes[node_index].children_range.first);
        assert(child_index < nodes[node_index].children_range.second);
        state.apply_action(children[child_index].action);
        move_chooser.switch_context(children[child_index].action, current_player);
        complete_turn(state);
        children_stack.emplace_back(node_index, child_index, current_player);
        #if RAVE > 0
        context_stack.emplace_back(context);
        context = moves_set.get_context(children[child_index].action, current_player - 1, context);
        assert(!reasoner::is_switch(children[child_index].action.index) || context == 0);
        #endif
        node_index = children[child_index].index;
        node_sim_count = children[child_index].sim_count;
        assert(node_index != 0);
        if (state.is_nodal()) {
            move_chooser.reset_context();
            ++state_count;
        }
    }
    if (!nodes[node_index].is_expanded()) {
        create_children(node_index, state);
    }
    if (nodes[node_index].is_terminal()) {
        get_scores_from_state(state, results);
    }
    else {
        current_player = state.get_current_player();
        auto child_index = move_chooser.get_unvisited_child_index(children, nodes[node_index], node_sim_count, current_player);
        auto ri = state.apply_action_with_revert(children[child_index].action);
        move_chooser.switch_context(children[child_index].action, current_player);
        path.clear();
        while (!save_path_to_nodal_state(state, path)) {
            assert(path.empty());
            state.revert(ri);
            move_chooser.revert_context();
            auto& [fst, lst] = nodes[node_index].children_range;
            --lst;
            if (child_index != lst) {
                std::swap(children[child_index], children[lst]);
            }
            if (fst == lst) {
                assert(nodes[node_index].is_nodal);
                assert(nodes[node_index].status != node_status::nonterminal);
                nodes[node_index].status = node_status::terminal;
                nodes[node_index].children_range = {0, 0};
                get_scores_from_state(state, results);
                goto terminal;
            }
            assert(fst < lst);
            while (is_node_fully_expanded(node_index)) {
                child_index = get_best_uct_child_index(node_index, node_sim_count);
                state.apply_action(children[child_index].action);
                move_chooser.switch_context(children[child_index].action, current_player);
                complete_turn(state);
                children_stack.emplace_back(node_index, child_index, current_player);
                #if RAVE > 0
                context_stack.emplace_back(context);
                context = moves_set.get_context(children[child_index].action, current_player - 1, context);
                assert(!reasoner::is_switch(children[child_index].action.index) || context == 0);
                assert(!state.is_nodal() || reasoner::is_switch(children[child_index].action.index));
                #endif
                node_index = children[child_index].index;
                node_sim_count = children[child_index].sim_count;
                assert(node_index > 0);
                current_player = state.get_current_player();
                if (state.is_nodal()) {
                    move_chooser.reset_context();
                    ++state_count;
                }
                if (!nodes[node_index].is_expanded()) {
                    create_children(node_index, state);
                }
                if (nodes[node_index].is_terminal()) {
                    get_scores_from_state(state, results);
                    goto terminal;
                }
            }
            child_index = move_chooser.get_unvisited_child_index(children, nodes[node_index], node_sim_count, current_player);
            ri = state.apply_action_with_revert(children[child_index].action);
            move_chooser.switch_context(children[child_index].action, current_player);
        }
        complete_turn(state);
        children_stack.emplace_back(node_index, child_index, current_player);
        #if RAVE > 0
        context_stack.emplace_back(context);
        context = moves_set.get_context(children[child_index].action, current_player - 1, context);
        assert(!reasoner::is_switch(children[child_index].action.index) || context == 0);
        #endif
        ++state_count;
        auto status = path.empty() ? node_status::unknown : node_status::nonterminal;
        auto new_node_index = create_node(state, status);
        children[child_index].index = new_node_index;
        if constexpr (IS_NODAL) {
            const uint size = path.size();
            for (uint i = 0; i < size; ++i) {
                const auto& action = path[i];
                if (!nodes[new_node_index].is_expanded()) {
                    create_children(new_node_index, state);
                }
                auto [fst, lst] = nodes[new_node_index].children_range;
                for (auto j = fst; j < lst; ++j) {
                    if (children[j].action == action) {
                        state.apply_action(action);
                        if (i + 1 == size) {
                            complete_turn(state);
                            new_node_index = create_node(state, node_status::unknown);
                        }
                        else {
                            new_node_index = create_node(state, node_status::nonterminal);
                        }
                        children[j].index = new_node_index;
                        if (j != fst) {
                            std::swap(children[j], children[fst]);
                        }
                        children_stack.emplace_back(new_node_index, fst, current_player);
                        #if RAVE > 0
                        context_stack.emplace_back(context);
                        context = moves_set.get_context(children[fst].action, current_player - 1, context);
                        assert(!reasoner::is_switch(children[fst].action.index) || context == 0);
                        #endif
                        break;
                    }
                }
            }
        }
        else {
            for (const auto action : path) {
                state.apply_action(action);
            }
            complete_turn(state);
        }
        assert(state.is_nodal());
        auto states_in_simulation = play(state, move_chooser, cache, results);
        if (states_in_simulation > 0) {
            assert(nodes[new_node_index].status != node_status::terminal);
            nodes[new_node_index].status = node_status::nonterminal;
            state_count += states_in_simulation;
        }
        else {
            if constexpr (IS_NODAL) {
                nodes[new_node_index].status = node_status::terminal;
                nodes[new_node_index].children_range = {0, 0};
            }
            else {
                if (nodes[new_node_index].is_nodal) {
                    assert(path.empty());
                    assert(nodes[new_node_index].status != node_status::nonterminal);
                    nodes[new_node_index].status = node_status::terminal;
                    nodes[new_node_index].children_range = {0, 0};
                }
                else {
                    assert(!path.empty());
                    nodes[new_node_index].status = node_status::nonterminal;
                }
            }
        }
    }
    terminal:
    #if RAVE > 0
    if constexpr (!IS_NODAL) {
        for (const auto& action : path) {
            context = moves_set.insert(action, current_player - 1, context);
        }
    }
    assert(context == 0);
    for (const auto& [move, player] : move_chooser.get_path()) {
        context = moves_set.insert(move, player - 1, context);
    }
    assert(context == 0);
    #endif
    for (const auto [node_index, child_index, player] : boost::adaptors::reverse(children_stack)) {
        assert(children[child_index].index != 0);
        assert(player != KEEPER);
        assert(nodes[children[child_index].index].status != node_status::unknown);
        children[child_index].sim_count++;
        children[child_index].total_score += results[player - 1];
        #if RAVE > 0
        assert(!context_stack.empty());
        moves_set.update_amaf_scores(node_index, child_index, player - 1, results, context_stack.back());
        context_stack.pop_back();
        #endif
    }
    ++root_sim_count;
    #if MAST > 0
    for (const auto [node_index, child_index, player] : children_stack) {
        move_chooser.update_move(children[child_index].action, results, player);
    }
    if constexpr (!IS_NODAL && !TREE_ONLY) {
        for (const auto& action : path) {
            move_chooser.update_move(action, results, current_player);
        }
    }
    if constexpr (!TREE_ONLY) {
        move_chooser.update_all_moves(results);
    }
    move_chooser.reset_context();
    #endif
    #if RAVE > 0
    assert(context_stack.empty());
    moves_set.reset_moves();
    #endif
    return state_count;
}

void SemisplitTree::reparent_along_move(const reasoner::move& move) {
    #if STATS
    ++turn_number;
    #endif
    root_state.apply_move(move);
    complete_turn(root_state);
    uint root_index = 0;
    static std::vector<std::tuple<uint, uint, uint>> stack;
    stack.clear();
    const auto& mr = move.mr;
    const uint size = mr.size();
    uint i = 0;
    auto [fst, lst] = nodes.front().children_range;
    while (i < size) {
        if (!nodes[root_index].is_expanded()) {
            root_index = 0;
            break;
        }
        while (fst < lst) {
            if (children[fst].action.index <= 0) {
                stack.emplace_back(root_index, fst, root_sim_count);
                root_index = children[fst].index;
                root_sim_count = children[fst].sim_count;
                break;
            }
            else if (children[fst].action == mr[i]) {
                root_index = children[fst].index;
                root_sim_count = children[fst].sim_count;
                ++i;
                stack.clear();
                break;
            }
            ++fst;
        }
        if (fst == lst || root_index == 0) {
            if (!stack.empty()) {
                std::tie(root_index, fst, root_sim_count) = stack.back();
                stack.pop_back();
                ++fst;
                lst = nodes[root_index].children_range.second;
                continue;
            }
            else {
                root_index = 0;
                break;
            }
        }
        std::tie(fst, lst) = nodes[root_index].children_range;
    }
    if (root_index == 0) {
        root_sim_count = 0;
        nodes.clear();
        children.clear();
        const auto status = has_nodal_successor(root_state) ? node_status::nonterminal : node_status::terminal;
        create_node(root_state, status);
    }
    else {
        root_at_index(root_index);
        if (nodes.front().status == node_status::unknown) {
            if (has_nodal_successor(root_state)) {
                nodes.front().status = node_status::nonterminal;
            }
            else {
                nodes.front().status = node_status::terminal;
                nodes.front().children_range = {0, 0};
            }
        }
    }
    if (!nodes.front().is_expanded()) {
        create_children(0, root_state);
    }
    #if MAST > 0
    move_chooser.complete_turn();
    #endif
}

void SemisplitTree::choose_best_greedy_move(std::vector<uint>& children_indices) {
    [[maybe_unused]] uint level = 1;
    uint node_index = 0;
    if (!children_indices.empty()) {
        const uint last_child = children_indices.back();
        const uint last_node =  children[last_child].index;
        if (last_node == 0 || nodes[last_node].is_nodal || !nodes[last_node].is_expanded()) {
            return;
        }
        node_index = last_node;
    }
    do {
        assert(nodes[node_index].is_expanded());
        assert(node_index == 0 || !nodes[node_index].is_nodal);
        const auto best_child = get_top_ranked_child_index(node_index);
        children_indices.emplace_back(best_child);
        #if STATS
        std::cout << "moves at level " << level << std::endl;
        const auto [fst, lst] = nodes[node_index].children_range;
        for (auto i = fst; i < lst; ++i) {
            std::cout << ((i == best_child) ? "* " : "  ");
            print_node_stats(children[i]);
            std::cout << "(";
            print_action(children[i].action);
            std::cout << ")" << std::endl;
        }
        std::cout << std::endl;
        ++level;
        #endif
        node_index = children[best_child].index;
    } while (node_index > 0 && !nodes[node_index].is_nodal && nodes[node_index].is_expanded());
}

reasoner::move SemisplitTree::get_move_from_saved_path_with_random_suffix(std::vector<uint>& children_indices) {
    static GameState state;
    state = root_state;
    reasoner::move move;
    for (const auto child_index : children_indices) {
        const auto& action = children[child_index].action;
        if (action.index >= 0) {
            move.mr.emplace_back(action);
        }
        state.apply_action(action);
    }
    if (!state.is_nodal()) {
        #if STATS
        std::cout << "random continuation..." << std::endl << std::endl;
        #endif
        static std::vector<reasoner::action_representation> move_suffix;
        move_suffix.clear();
        random_walk_to_nodal(state, move_suffix);
        assert(!move_suffix.empty());
        for (const auto action : move_suffix) {
            if (action.index > 0) {
                move.mr.push_back(action);
            }
        }
    }
    return move;
}

game_status_indication SemisplitTree::get_status(const int player_index) {
    assert(root_state.is_nodal());
    assert(nodes.front().status != node_status::unknown);
    if (nodes.front().is_terminal()) {
        return end_game;
    }
    return root_state.get_current_player() == (player_index + 1) ? own_turn : opponent_turn;
}
