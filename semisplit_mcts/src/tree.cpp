#include "tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "constants.hpp"


Tree::Tree(const reasoner::game_state& initial_state) : MctsTree(initial_state) {
    complete_turn(root_state);
    create_node(root_state);
}

uint Tree::create_node(reasoner::game_state& state) {
    static std::vector<reasoner::semimove> semimoves;
    uint new_child_index = children.size();
    auto state_copy = state;
    bool is_nodal = state.is_nodal();
    bool has_nodal_succ = has_nodal_successor(state_copy, cache);
    if (is_nodal && !has_nodal_succ) {
        nodes.emplace_back(new_child_index, 0, is_nodal, has_nodal_succ);
    }
    else {
        state.get_all_semimoves(cache, semimoves, SEMILENGTH);
        auto child_count = semimoves.size();
        nodes.emplace_back(new_child_index, child_count, is_nodal, has_nodal_succ);
        for (const auto& semimove : semimoves) {
            auto ri = state.apply_semimove_with_revert(semimove);
            children.emplace_back(semimove, state.is_nodal());
            state.revert(ri);
        }
    }
    return nodes.size() - 1;
}

bool Tree::save_path_to_nodal_state(reasoner::game_state& state, std::vector<reasoner::semimove>& path, uint semidepth) {
    static std::vector<reasoner::semimove> legal_semimoves[20];
    if (state.is_nodal()) {
        return true;
    }
    state.get_all_semimoves(cache, legal_semimoves[semidepth], SEMILENGTH);
    while (!legal_semimoves[semidepth].empty()) {
        auto chosen_semimove = move_chooser.get_random_move(legal_semimoves[semidepth], state.get_current_player());
        auto ri = state.apply_semimove_with_revert(legal_semimoves[semidepth][chosen_semimove]);
        path.emplace_back(legal_semimoves[semidepth][chosen_semimove]);
        if (save_path_to_nodal_state(state, path, semidepth+1)) {
            state.revert(ri);
            return true;
        }
        legal_semimoves[semidepth][chosen_semimove] = legal_semimoves[semidepth].back();
        legal_semimoves[semidepth].pop_back();
        path.pop_back();
        state.revert(ri);
    }
    return false;
}

uint Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS - 1);
    static reasoner::game_state state = root_state;
    static std::vector<reasoner::semimove> path;
    uint state_count = 0;
    uint node_index = children_stack.empty() ? 0 : children[children_stack.back().first].index;
    state = root_state;
    for (const auto el : children_stack) {
        state.apply_semimove(children[el.first].semimove);
        complete_turn(state);
    }
    while (!nodes[node_index].is_terminal() && is_node_fully_expanded(node_index)) {
        const auto child_index = get_best_uct_child_index(node_index);
        const auto current_player = state.get_current_player();
        assert(child_index >= nodes[node_index].children_range.first);
        assert(child_index < nodes[node_index].children_range.second);
        state.apply_semimove(children[child_index].semimove);
        complete_turn(state);
        children_stack.emplace_back(child_index, current_player);
        node_index = children[child_index].index;
        assert(node_index != 0);
        if (state.is_nodal())
            ++state_count;
    }
    if (nodes[node_index].is_terminal()) {
        for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
            results[i - 1] = state.get_player_score(i);
        }
        [[maybe_unused]] uint depth = children_stack.size();
        for (const auto [index, player] : children_stack) {
            nodes[children[index].index].sim_count++;
            children[index].sim_count++;
            children[index].total_score += results[player - 1];
            #if MAST > 0
            move_chooser.update_move(children[index].get_actions(), results, player, depth);
            #endif
        }
        nodes.front().sim_count++;
        children_stack.clear();
    }
    else {
        const auto current_player = state.get_current_player();
        const auto child_index = move_chooser.get_unvisited_child_index(children, nodes[node_index], current_player);
        state.apply_semimove(children[child_index].semimove);
        complete_turn(state);
        path.clear();
        if (save_path_to_nodal_state(state, path, 0)) {
            children_stack.emplace_back(child_index, state.get_current_player());
            ++state_count;
            if constexpr (IS_NODAL) {
                auto new_node_index = create_node(state);
                children[child_index].index = new_node_index;
                for (const auto semimove : path) {
                    auto [fst, lst] = nodes[new_node_index].children_range;
                    for (auto i = fst; i < lst; ++i) {
                        if (children[i].semimove.get_actions() == semimove.get_actions()) {
                            state.apply_semimove(semimove);
                            complete_turn(state);
                            children_stack.emplace_back(i, state.get_current_player());
                            new_node_index = create_node(state);
                            children[i].index = new_node_index;
                            if (i != fst) {
                                std::swap(children[i], children[fst]);
                            }
                            break;
                        }
                    }
                }
            }
            else {
                children[child_index].index = create_node(state);
                for (const auto semimove : path) {
                    state.apply_semimove(semimove);
                }
            }
            state_count += play(state, move_chooser, cache, results);
            [[maybe_unused]] const uint depth = children_stack.size() + move_chooser.get_path().size();
            for (const auto [index, player] : children_stack) {
                nodes[children[index].index].sim_count++;
                children[index].sim_count++;
                children[index].total_score += results[player - 1];
                #if MAST > 0
                move_chooser.update_move(children[index].get_actions(), results, player, depth);
                #endif
            }
            nodes.front().sim_count++;
            #if MAST > 0
            for (const auto& semimove : path) {
                move_chooser.update_move(semimove.get_actions(), results, current_player, depth);
            }
            move_chooser.update_all_moves(simulation_result, depth);
            #endif
            children_stack.clear();
        }
        else {
            auto& end = nodes[node_index].children_range.second;
            --end;
            if (child_index != end) {
                std::swap(children[child_index], children[end]);
            }
            const auto [fst, lst] = nodes[node_index].children_range;
            assert(fst < lst);
        }
        move_chooser.clear_path();
    }
    return state_count;
}

void Tree::reparent_along_move(const reasoner::move& move) {
    root_state.apply_move(move);
    complete_turn(root_state);
    uint root_index = 0;
    if constexpr (SEMILENGTH == 1) {
        for (const auto& action : move.mr) {
            const auto [fst, lst] = nodes[root_index].children_range;
            auto i = fst;
            while (i < lst) {
                if (children[i].semimove.get_actions().front() == action) {
                    root_index = children[i].index;
                    break;
                }
                ++i;
            }
            if (i == lst || root_index == 0) {
                break;
            }
        }
    }
    else {
        // not implemented
        assert(false);
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
    reasoner::move move;
    uint node_index = 0;
    while (true) {
        const auto& [fst, lst] = nodes[node_index].children_range;
        auto max_sim = children[fst].sim_count;
        auto best_node = fst;
        for (auto i = fst + 1; i < lst; ++i) {
            if (children[i].sim_count > max_sim) {
                max_sim = children[i].sim_count;
                best_node = i;
            }
        }
        const auto& semimove = children[best_node].semimove.get_actions();
        move.mr.insert(move.mr.end(), semimove.begin(), semimove.end());
        if (children[best_node].is_nodal) {
            break;
        }
        node_index = children[best_node].index;
        assert(node_index != 0);
    }
    return move;
}

game_status_indication Tree::get_status(const int player_index) const {
    if (nodes.front().is_terminal()) {
        return end_game;
    }
    return root_state.get_current_player() == (player_index + 1) ? own_turn : opponent_turn;
}
