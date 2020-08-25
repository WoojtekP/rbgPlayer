#if STATS
#include <iostream>
#include <iomanip>
#endif

#include "semisplit_tree.hpp"
#include "semisplit_node.hpp"
#include "simulator.hpp"
#include "constants.hpp"


#if STATS
void SemisplitTree::print_node_stats(const SemisplitChild& child) {
    std::cout << "sim " << std::setw(4) << child.sim_count;
    std::cout << "   avg " << std::setw(6) << static_cast<double>(child.total_score) / child.sim_count << "   ";
    std::cout << "(" << std::setw(3) << child.semimove.cell << " " << std::setw(3) << child.semimove.state << ") ";
}

void SemisplitTree::print_actions(const reasoner::move_representation& mr) {
    for (const auto& action : mr) {
        std::cout << std::setw(3) << action.cell << " " << std::setw(3) << action.index << " ";
    }
}

void SemisplitTree::print_move(const reasoner::move_representation& mr) {
    std::cout << "[";
    print_actions(mr);
    std::cout << "]" << std::endl;
}
#endif

SemisplitTree::SemisplitTree(const reasoner::game_state& initial_state) : MctsTree(initial_state) {
    complete_turn(root_state);
    const auto status = has_nodal_successor(root_state) ? node_status::nonterminal : node_status::terminal;
    create_node(root_state, status);
    create_children(0, root_state);
}

uint SemisplitTree::create_node(reasoner::game_state& state, const node_status status) {
    if (status == node_status::terminal || state.get_current_player() == KEEPER) {
        assert(state.is_nodal());
        nodes.emplace_back(0, 0, true, node_status::terminal);
    }
    else {
        nodes.emplace_back(state.is_nodal(), status);
    }
    return nodes.size() - 1;
}

void SemisplitTree::create_children(const uint node_index, reasoner::game_state& state) {
    static std::vector<reasoner::semimove> semimoves;
    state.get_all_semimoves(cache, semimoves, SEMILENGTH);
    if (semimoves.empty() || nodes[node_index].status == node_status::terminal) {
        assert(state.is_nodal());
        nodes[node_index].status = node_status::terminal;
        nodes[node_index].children_range = {0, 0};
    }
    else {
        assert(state.get_current_player() != KEEPER);
        nodes[node_index].children_range.first = children.size();
        for (const auto& semimove : semimoves) {
            children.emplace_back(semimove);
        }
        nodes[node_index].children_range.second = children.size();
    }
}

bool SemisplitTree::has_nodal_successor(reasoner::game_state& state, uint semidepth) {
    static std::vector<reasoner::semimove> legal_semimoves[MAX_SEMIDEPTH];
    if (state.get_current_player() == KEEPER) {
        return false;
    }
    state.get_all_semimoves(cache, legal_semimoves[semidepth], SEMILENGTH);
    while (!legal_semimoves[semidepth].empty()) {
        auto ri = state.apply_semimove_with_revert(legal_semimoves[semidepth].back());
        if (state.is_nodal()) {
            state.revert(ri);
            return true;
        }
        if (has_nodal_successor(state, semidepth+1)) {
            state.revert(ri);
            return true;
        }
        legal_semimoves[semidepth].pop_back();
        state.revert(ri);
    }
    return false;
}

bool SemisplitTree::save_path_to_nodal_state(reasoner::game_state& state, std::vector<reasoner::semimove>& path, uint semidepth) {
    static std::vector<reasoner::semimove> legal_semimoves[MAX_SEMIDEPTH];
    if (state.is_nodal()) {
        move_chooser.reset_context();
        return true;
    }
    state.get_all_semimoves(cache, legal_semimoves[semidepth], SEMILENGTH);
    while (!legal_semimoves[semidepth].empty()) {
        const auto current_player = state.get_current_player();
        const auto chosen_semimove = move_chooser.get_random_move(legal_semimoves[semidepth], current_player);
        const auto ri = state.apply_semimove_with_revert(legal_semimoves[semidepth][chosen_semimove]);
        move_chooser.switch_context(legal_semimoves[semidepth][chosen_semimove], current_player);
        path.emplace_back(legal_semimoves[semidepth][chosen_semimove]);
        if (save_path_to_nodal_state(state, path, semidepth + 1)) {
            state.revert(ri);
            return true;
        }
        legal_semimoves[semidepth][chosen_semimove] = legal_semimoves[semidepth].back();
        legal_semimoves[semidepth].pop_back();
        path.pop_back();
        state.revert(ri);
        move_chooser.revert_context();
    }
    return false;
}

uint SemisplitTree::perform_simulation() {
    static simulation_result results;
    static reasoner::game_state state = root_state;
    static std::vector<reasoner::semimove> path;
    children_stack.clear();
    move_chooser.clear_path();
    path.clear();
    uint state_count = 0;
    uint node_index = 0;
    uint node_sim_count = root_sim_count;
    int current_player;
    state = root_state;
    while (!nodes[node_index].is_terminal() && is_node_fully_expanded(node_index)) {
        current_player = state.get_current_player();
        const auto child_index = get_best_uct_child_index(node_index, node_sim_count);
        assert(child_index >= nodes[node_index].children_range.first);
        assert(child_index < nodes[node_index].children_range.second);
        state.apply_semimove(children[child_index].semimove);
        move_chooser.switch_context(children[child_index].semimove, current_player);
        complete_turn(state);
        children_stack.emplace_back(child_index, current_player);
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
        auto child_index = get_unvisited_child_index(children, nodes[node_index], node_sim_count, current_player);
        auto ri = state.apply_semimove_with_revert(children[child_index].semimove);
        move_chooser.switch_context(children[child_index].semimove, current_player);
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
                state.apply_semimove(children[child_index].semimove);
                move_chooser.switch_context(children[child_index].semimove, current_player);
                complete_turn(state);
                children_stack.emplace_back(child_index, current_player);
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
            child_index = get_unvisited_child_index(children, nodes[node_index], node_sim_count, current_player);
            ri = state.apply_semimove_with_revert(children[child_index].semimove);
            move_chooser.switch_context(children[child_index].semimove, current_player);
        }
        complete_turn(state);
        children_stack.emplace_back(child_index, current_player);
        ++state_count;
        auto status = path.empty() ? node_status::unknown : node_status::nonterminal;
        auto new_node_index = create_node(state, status);
        children[child_index].index = new_node_index;
        if constexpr (IS_NODAL) {
            const uint size = path.size();
            for (uint i = 0; i < size; ++i) {
                const auto& semimove = path[i];
                if (!nodes[new_node_index].is_expanded()) {
                    create_children(new_node_index, state);
                }
                auto [fst, lst] = nodes[new_node_index].children_range;
                for (auto j = fst; j < lst; ++j) {
                    if (children[j].semimove == semimove) {
                        state.apply_semimove(semimove);
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
                        children_stack.emplace_back(fst, current_player);
                        break;
                    }
                }
            }
        }
        else {
            for (const auto semimove : path) {
                state.apply_semimove(semimove);
            }
            complete_turn(state);
        }
        assert(state.is_nodal());
        auto states_in_simulation = play(state, move_chooser, cache, results);
        if (states_in_simulation > 0) {
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
    [[maybe_unused]] const uint depth = children_stack.size() + move_chooser.get_path().size();  // TODO fix
    for (const auto [child_index, player] : children_stack) {
        assert(children[child_index].index != 0);
        assert(player != KEEPER);
        assert(nodes[children[child_index].index].status != node_status::unknown);
        children[child_index].sim_count++;
        children[child_index].total_score += results[player - 1];
        #if MAST > 0
        move_chooser.update_move(children[child_index].semimove, results, player, depth, nodes[children[child_index].index].is_nodal);
        #endif
    }
    ++root_sim_count;
    #if MAST > 0
    if constexpr (!IS_NODAL) {
        int i = 1;
        for (const auto& semimove : path) {
            move_chooser.update_move(semimove, results, current_player, depth, i == path.size());
            ++i;
        }
    }
    move_chooser.reset_context();
    move_chooser.update_all_moves(results, depth);
    #endif
    return state_count;
}

void SemisplitTree::reparent_along_move(const reasoner::move& move) {
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
            if (children[fst].get_actions().empty()) {
                stack.emplace_back(root_index, fst, root_sim_count);
                root_index = children[fst].index;
                root_sim_count = children[fst].sim_count;
                break;
            }
            bool matched = true;
            uint j = 0;
            for (const auto& action : children[fst].get_actions()) {
                if (!(action == mr[i + j])) {
                    matched = false;
                    break;
                }
                ++j;
            }
            if (matched) {
                root_index = children[fst].index;
                root_sim_count = children[fst].sim_count;
                i += j;
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
}

reasoner::move SemisplitTree::choose_best_greedy_move() {
    static std::vector<uint> children_indices;
    #if STATS
    uint level = 1;
    std::cout << "turn number " << turn_number / 2 + 1 << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    #endif
    children_indices.clear();
    reasoner::move move;
    uint node_index = 0;
    while (true) {
        const auto best_child = get_top_ranked_child_index(node_index);
        const auto& semimove = children[best_child].semimove.get_actions();
        move.mr.insert(move.mr.end(), semimove.begin(), semimove.end());
        #if STATS
        std::cout << "moves at level " << level << std::endl;
        for (auto i = fst; i < lst; ++i) {
            char prefix = (i == best_child) ? '*' : ' ';
            std::cout << prefix << " ";
            print_node_stats(children[i]);
            print_move(children[i].get_actions());
        }
        std::cout << std::endl;
        ++level;
        #endif
        if (nodes[children[best_child].index].is_nodal) {
            break;
        }
        node_index = children[best_child].index;
        assert(node_index != 0);
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
