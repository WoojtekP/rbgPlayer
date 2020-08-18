#if STATS
#include <iostream>
#include <iomanip>
#endif

#include "tree.hpp"
#include "semisplit_tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "constants.hpp"


#if STATS
void Tree::print_rolledup_move(const std::vector<uint>& children_indices) {
    std::cout << "[";
    for (const auto child_index : children_indices) {
        print_actions(children[child_index].semimove.get_actions());
    }
    std::cout << "]" << std::endl;
}
#endif

Tree::Tree(const reasoner::game_state& initial_state) : SemisplitTree(initial_state) {}

void Tree::choose_best_rolledup_move(const uint node_index, std::vector<uint>& best_move_path) {
    static std::vector<uint> move_path;
    static uint max_sim = 0;
    static double max_score = 0;
    const auto [fst, lst] = nodes[node_index].children_range;
    for (auto i = fst; i < lst; ++i) {
        auto index = children[i].index;
        if (index > 0) {
            move_path.push_back(i);
            if (nodes[index].is_nodal) {
                #if STATS
                print_node_stats(children[i]);
                print_rolledup_move(move_path);
                #endif
                if (children[i].sim_count > max_sim) {
                    max_sim = children[i].sim_count;
                    max_score = static_cast<double>(children[i].total_score) / children[i].sim_count;
                    best_move_path = move_path;
                }
                else if (children[i].sim_count == max_sim) {
                    auto score = static_cast<double>(children[i].total_score) / children[i].sim_count;
                    if (score > max_score) {
                        max_score = score;
                        best_move_path = move_path;
                    }
                }
            }
            else {
                choose_best_rolledup_move(index, best_move_path);
            }
            move_path.pop_back();
        }
    }
    if (node_index == 0) {
        max_sim = 0;
        max_score = 0;
        move_path.clear();
    }
}

uint Tree::perform_simulation() {
    static simulation_result results;
    static reasoner::game_state state = root_state;
    static std::vector<reasoner::semimove> path;
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
        move_chooser.update_move(children[child_index].semimove, results, player, depth);
        #endif
    }
    ++root_sim_count;
    #if MAST > 0
    if constexpr (!IS_NODAL) {
        for (const auto& semimove : path) {
            move_chooser.update_move(semimove, results, current_player, depth);
        }
    }
    move_chooser.update_all_moves(results, depth);
    #endif
    children_stack.clear();
    move_chooser.clear_path();
    path.clear();
    return state_count;
}

reasoner::move Tree::choose_best_move() {
    if constexpr (GREEDY_CHOICE) {
        return choose_best_greedy_move();
    }
    static std::vector<uint> children_indices;
    children_indices.clear();
    #if STATS
    std::cout << std::setprecision(2);
    #endif
    choose_best_rolledup_move(0, children_indices);
    reasoner::move move;
    for (const auto child_index : children_indices) {
        const auto& semimove = children[child_index].semimove.get_actions();
        move.mr.insert(move.mr.end(), semimove.begin(), semimove.end());
    }
    #if STATS
    std::cout << std::endl << "chosen move:" << std::endl;
    print_node_stats(children[children_indices.back()]);
    print_rolledup_move(children_indices);
    std::cout << std::endl;
    #endif
    return move;
}
