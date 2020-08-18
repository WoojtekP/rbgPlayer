#if STATS
#include <iostream>
#include <iomanip>
#endif

#include "tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "constants.hpp"


Tree::Tree(const reasoner::game_state& initial_state) : SemisplitTree(initial_state) {}

void Tree::choose_children_for_rolling_up(const uint node_index, const uint node_sim_count, std::vector<uint>& children_indices) {
    children_indices.clear();
    const auto [fst, lst] = nodes[node_index].children_range;
    for (auto i = fst; i < lst; ++i) {
        auto index = children[i].index;
        if (index > 0 && !nodes[index].is_nodal && is_node_fully_expanded(index)) {
            const auto [fst_child, lst_child] = nodes[index].children_range;
            const auto child_count = lst_child - fst_child;
            if (node_sim_count >= child_count * MIN_SIMULATIONS_FACTOR) {
                if constexpr (MIN_SCORE_DIFF == 0.0) {
                    children_indices.push_back(i);
                }
                else if (child_count == 1) {
                    children_indices.push_back(i);
                }
                else {
                    double min_score = static_cast<double>(children[fst_child].total_score) / children[fst_child].sim_count;;
                    double max_score = min_score;
                    for (auto i = fst_child + 1; i < lst_child; ++i) {
                        double score = static_cast<double>(children[i].total_score) / children[i].sim_count;
                        if (score > max_score) {
                            max_score = score;
                        }
                        else if (score < min_score) {
                            min_score = score;
                        }
                    }
                    if (max_score - min_score >= MIN_SCORE_DIFF) {
                        children_indices.push_back(i);
                    }
                }
            }
        }
    }
}

void Tree::roll_up(const uint node_index, std::vector<uint>& children_indices) {
    uint new_child_index = children.size();
    uint i = nodes[node_index].children_range.first;
    for (const auto child_index : children_indices) {
        assert(child_index >= nodes[node_index].children_range.first);
        assert(child_index < nodes[node_index].children_range.second);
        for (; i < child_index; ++i) {
            children.push_back(std::move(children[i]));
        }
        ++i;
        assert(children[child_index].index > 0);
        assert(is_node_fully_expanded(children[child_index].index));
        const auto [fst, lst] = nodes[children[child_index].index].children_range;
        const auto& semimove_prefix = children[child_index].semimove;
        const auto mr_prefix = semimove_prefix.get_actions();
        for (auto j = fst; j < lst; ++j) {
            const auto& semimove_suffix = children[j].semimove;
            auto mr_suffix = semimove_suffix.get_actions();
            const auto cell = semimove_suffix.cell;
            const auto state = semimove_suffix.state;
            mr_suffix.insert(mr_suffix.begin(), mr_prefix.begin(), mr_prefix.end());
            children.push_back(std::move(children[j]));
            children.back().semimove = reasoner::semimove(mr_suffix, cell, state);
        }
    }
    for (; i < nodes[node_index].children_range.second; ++i) {
        children.push_back(std::move(children[i]));
    }
    nodes[node_index].children_range.first = new_child_index;
    nodes[node_index].children_range.second = children.size();
}

uint Tree::perform_simulation() {
    static simulation_result results;
    static reasoner::game_state state = root_state;
    static std::vector<reasoner::semimove> path;
    static std::vector<uint> children_indices;
    for (const auto& node : nodes) {
        assert(node.status != node_status::unknown);
        if (node.status == node_status::terminal) {
            assert(node.is_nodal);
            assert(node.children_range.first == node.children_range.second);
        }
    }
    uint state_count = 0;
    uint node_index = 0;
    uint node_sim_count = root_sim_count;
    int current_player;
    state = root_state;
    while (!nodes[node_index].is_terminal() && is_node_fully_expanded(node_index)) {
        const auto child_index = get_best_uct_child_index(node_index, node_sim_count);
        current_player = state.get_current_player();
        assert(child_index >= nodes[node_index].children_range.first);
        assert(child_index < nodes[node_index].children_range.second);
        state.apply_semimove(children[child_index].semimove);
        complete_turn(state);
        children_stack.emplace_back(child_index, current_player);
        node_index = children[child_index].index;
        node_sim_count = children[child_index].sim_count;
        assert(node_index != 0);
        if (state.is_nodal()) {
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
        auto ri = state.apply_semimove_with_revert(children[child_index].semimove);
        path.clear();
        while (!save_path_to_nodal_state(state, path)) {
            assert(path.empty());
            state.revert(ri);
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
                complete_turn(state);
                children_stack.emplace_back(child_index, current_player);
                node_index = children[child_index].index;
                node_sim_count = children[child_index].sim_count;
                assert(node_index > 0);
                current_player = state.get_current_player();
                if (!nodes[node_index].is_expanded()) {
                    create_children(node_index, state);
                }
                if (nodes[node_index].is_terminal()) {
                    get_scores_from_state(state, results);
                    goto terminal;
                }
            }
            child_index = move_chooser.get_unvisited_child_index(children, nodes[node_index], node_sim_count, current_player);
            ri = state.apply_semimove_with_revert(children[child_index].semimove);
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
        children[child_index].sim_count++;
        children[child_index].total_score += results[player - 1];
        #if MAST > 0
        move_chooser.update_move(children[child_index].get_actions(), results, player, depth);
        #endif
    }
    ++root_sim_count;
    #if MAST > 0
    if constexpr (!IS_NODAL) {
        for (const auto& semimove : path) {
            move_chooser.update_move(semimove.get_actions(), results, current_player, depth);
        }
    }
    move_chooser.update_all_moves(results, depth);
    #endif
    int size = children_stack.size();
    for (int i = size - 2; i >= 0; --i) {
        node_index = children[children_stack[i].first].index;
        node_sim_count = children[children_stack[i].first].sim_count;
        choose_children_for_rolling_up(node_index, node_sim_count, children_indices);
        if (!children_indices.empty()) {
            roll_up(node_index, children_indices);
        }
    }
    choose_children_for_rolling_up(0, root_sim_count, children_indices);
    if (!children_indices.empty()) {
        roll_up(0, children_indices);
    }
    children_stack.clear();
    move_chooser.clear_path();
    path.clear();
    return state_count;
}

reasoner::move Tree::choose_best_move() {
    return choose_best_greedy_move();
}
