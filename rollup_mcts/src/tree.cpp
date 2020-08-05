#if STATS || defined(ROLLUP)
#include <algorithm>
#include <iostream>
#include <iomanip>
#endif

#include "tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "constants.hpp"


Tree::Tree(const reasoner::game_state& initial_state) : MctsTree(initial_state) {
    complete_turn(root_state);
    const auto status = has_nodal_successor(root_state) ? node_status::nonterminal : node_status::terminal;
    create_node(root_state, status);
}

uint Tree::create_node(reasoner::game_state& state, const node_status status) {
    static std::vector<reasoner::semimove> semimoves;
    uint new_child_index = children.size();
    if (status == node_status::terminal) {
        assert(state.is_nodal());
        nodes.emplace_back(new_child_index, 0, true, node_status::terminal);
    }
    else if (state.get_current_player() == KEEPER) {
        assert(!state.apply_any_move(cache));
        nodes.emplace_back(new_child_index, 0, true, node_status::terminal);
    }
    else {
        state.get_all_semimoves(cache, semimoves, SEMILENGTH);
        auto child_count = semimoves.size();
        if (child_count == 0) {
            assert(state.is_nodal());
            nodes.emplace_back(new_child_index, 0, true, node_status::terminal);
        }
        else {
            assert(state.get_current_player() != KEEPER);
            nodes.emplace_back(new_child_index, child_count, state.is_nodal(), status);
            for (const auto& semimove : semimoves) {
                children.emplace_back(semimove);
            }
        }
    }
    return nodes.size() - 1;
}

bool Tree::has_nodal_successor(reasoner::game_state& state, uint semidepth) {
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

bool Tree::save_path_to_nodal_state(reasoner::game_state& state, std::vector<reasoner::semimove>& path, uint semidepth) {
    static std::vector<reasoner::semimove> legal_semimoves[MAX_SEMIDEPTH];
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

void Tree::choose_children_for_rolling_up(const uint node_index, std::vector<uint>& children_indices) {
    children_indices.clear();
    const auto [fst, lst] = nodes[node_index].children_range;
    for (auto i = fst; i < lst; ++i) {
        auto index = children[i].index;
        if (index > 0 && !nodes[index].is_nodal && is_node_fully_expanded(index)) {
            children_indices.push_back(i);
        }
    }
}

void Tree::roll_up(const uint node_index, std::vector<uint>& children_indices) {
    uint new_child_index = children.size();
    uint i = nodes[node_index].children_range.first;
    #ifdef ROLLUP
    {
        std::cerr << "before rollup:\n";
        const auto [fst, lst] = nodes[node_index].children_range;
        for (auto i = fst; i < lst; ++i) {
            std::cerr << std::setw(3) << children[i].index << " ";
            std::cerr << std::setw(3) << children[i].sim_count << " ";
            std::cerr << std::setw(5) << children[i].total_score << " [";
            for (const auto& action : children[i].get_actions()) {
                std::cerr << std::setw(2) << action.cell << " " << std::setw(3) << action.index << " ";
            }
            std::cerr << "]  (" << std::setw(2) << children[i].semimove.cell << " " << std::setw(3) << children[i].semimove.state << ")";
            if (std::any_of(children_indices.begin(), children_indices.end(), [&i](const auto& index) { return index == i; })) {
                std::cerr << " *";
            }
            std::cerr << std::endl;
        }
    }
    #endif
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
        #ifdef ROLLUP
        std::cerr << "prefix: [";
        for (const auto& action : mr_prefix) {
            std::cerr << action.cell << " " << action.index << " ";
        }
        std::cerr << "]" << std::endl << "children:" << std::endl;
        #endif
        for (auto j = fst; j < lst; ++j) {
            #ifdef ROLLUP
            std::cerr << std::setw(3) << children[j].index << " ";
            std::cerr << std::setw(3) << children[j].sim_count << " ";
            std::cerr << std::setw(5) << children[j].total_score << " [";
            for (const auto& action : children[j].get_actions()) {
                std::cerr << std::setw(2) << action.cell << " " << std::setw(3) << action.index << ",";
            }
            std::cerr << "]  (" << std::setw(2) << children[j].semimove.cell << " " << std::setw(3) << children[j].semimove.state << ")" << std::endl;
            #endif
            const auto& semimove_suffix = children[j].semimove;
            auto mr_suffix = semimove_suffix.get_actions();
            const auto cell = semimove_suffix.cell;
            const auto state = semimove_suffix.state;
            mr_suffix.insert(mr_suffix.begin(), mr_prefix.begin(), mr_prefix.end());
            children.push_back(std::move(children[j]));
            children.back().semimove = reasoner::semimove(mr_suffix, cell, state);
        }
        #ifdef ROLLUP
        std::cerr << std::endl;
        #endif
    }
    for (; i < nodes[node_index].children_range.second; ++i) {
        children.push_back(std::move(children[i]));
    }
    nodes[node_index].children_range.first = new_child_index;
    nodes[node_index].children_range.second = children.size();
    #ifdef ROLLUP
    {
        std::cerr << "after rollup:\n";
        const auto [fst, lst] = nodes[node_index].children_range;
        for (auto i = fst; i < lst; ++i) {
            std::cerr << std::setw(3) << children[i].index << " ";
            std::cerr << std::setw(3) << children[i].sim_count << " ";
            std::cerr << std::setw(5) << children[i].total_score << " [";
            for (const auto& action : children[i].get_actions()) {
                std::cerr << std::setw(2) << action.cell << " " << std::setw(3) << action.index << " ";
            }
            std::cerr << "]  (" << std::setw(2) << children[i].semimove.cell << " " << std::setw(3) << children[i].semimove.state << ")" << std::endl;
        }
        std::cerr << std::endl;
    }
    #endif
}

uint Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS - 1);
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
    int current_player;
    state = root_state;
    while (!nodes[node_index].is_terminal() && is_node_fully_expanded(node_index)) {
        const auto child_index = get_best_uct_child_index(node_index);
        current_player = state.get_current_player();
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
    }
    else {
        current_player = state.get_current_player();
        auto child_index = move_chooser.get_unvisited_child_index(children, nodes[node_index], current_player);
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
                for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
                    results[i - 1] = state.get_player_score(i);
                }
                goto terminal;
            }
            assert(fst < lst);
            while (is_node_fully_expanded(node_index)) {
                child_index = get_best_uct_child_index(node_index);
                state.apply_semimove(children[child_index].semimove);
                complete_turn(state);
                children_stack.emplace_back(child_index, current_player);
                node_index = children[child_index].index;
                assert(node_index > 0);
                current_player = state.get_current_player();
                if (nodes[node_index].is_terminal()) {
                    for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
                        results[i - 1] = state.get_player_score(i);
                    }
                    goto terminal;
                }
            }
            child_index = move_chooser.get_unvisited_child_index(children, nodes[node_index], current_player);
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
                nodes[new_node_index].children_range.second = nodes[new_node_index].children_range.first;
            }
            else {
                if (nodes[new_node_index].is_nodal) {
                    assert(path.empty());
                    assert(nodes[new_node_index].status != node_status::nonterminal);
                    nodes[new_node_index].status = node_status::terminal;
                    nodes[new_node_index].children_range.second = nodes[new_node_index].children_range.first;
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
        nodes[children[child_index].index].sim_count++;
        children[child_index].sim_count++;
        children[child_index].total_score += results[player - 1];
        #if MAST > 0
        move_chooser.update_move(children[child_index].get_actions(), results, player, depth);
        #endif
    }
    nodes.front().sim_count++;
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
        choose_children_for_rolling_up(node_index, children_indices);
        if (!children_indices.empty()) {
            roll_up(node_index, children_indices);
        }
    }
    choose_children_for_rolling_up(0, children_indices);
    if (!children_indices.empty()) {
        roll_up(0, children_indices);
    }
    children_stack.clear();
    move_chooser.clear_path();
    path.clear();
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

    static std::vector<std::pair<uint, uint>> stack;
    stack.clear();
    const auto& mr = move.mr;
    const uint size = mr.size();
    uint i = 0;
    auto [fst, lst] = nodes[root_index].children_range;
    while (i < size) {
        while (fst < lst) {
            if (children[fst].get_actions().empty()) {
                stack.emplace_back(root_index, fst);
                root_index = children[fst].index;
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
                i += j;
                stack.clear();
                break;
            }
            ++fst;
        }
        if (fst == lst || root_index == 0) {
            if (!stack.empty()) {
                std::tie(root_index, fst) = stack.back();
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
        nodes.clear();
        children.clear();
        const auto status = has_nodal_successor(root_state) ? node_status::nonterminal : node_status::terminal;
        create_node(root_state, status);
        return;
    }
    root_at_index(root_index);
    children_stack.clear();
}

reasoner::move Tree::choose_best_move() {
    static std::vector<uint> children_indices;
    reasoner::move move;
    uint node_index = 0;
    #if STATS
    uint level = 1;
    std::cout << "turn number " << turn_number / 2 + 1 << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    #endif
    while (true) {
        const auto [fst, lst] = nodes[node_index].children_range;
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
        assert(max_sim > 0);
        auto best_child = children_indices.front();
        auto best_score = children[best_child].total_score;
        for (const auto child_index : children_indices) {
            if (children[child_index].total_score > best_score) {
                best_score = children[child_index].total_score;
                best_child = child_index;
            }
        }
        const auto& semimove = children[best_child].semimove.get_actions();
        move.mr.insert(move.mr.end(), semimove.begin(), semimove.end());
        #if STATS
        std::cout << "moves at level " << level << std::endl;
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

game_status_indication Tree::get_status(const int player_index) {
    assert(root_state.is_nodal());
    assert(nodes.front().status != node_status::unknown);
    if (nodes.front().is_terminal()) {
        return end_game;
    }
    return root_state.get_current_player() == (player_index + 1) ? own_turn : opponent_turn;
}
