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
    state.get_all_semimoves(cache, semimoves, SEMILENGTH);
    auto child_count = semimoves.size();
    uint new_child_index = children.size();
    auto state_copy = state;
    nodes.emplace_back(new_child_index, child_count, state.is_nodal(), has_nodal_successor(state_copy, cache));
    for (const auto& semimove : semimoves) {
        auto ri = state.apply_semimove_with_revert(semimove);
        children.emplace_back(semimove, state.is_nodal());
        state.revert(ri);
    }
    return nodes.size() - 1;
}

void Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS - 1);
    static reasoner::game_state state = root_state;
    static std::vector<std::pair<uint,int>> children_stack;  // child index, player
    if (reset_path) {
        children_stack.clear();
    }
    uint node_index = children_stack.empty() ? 0 : children[children_stack.back().first].index;
    state = root_state;
    for (const auto el : children_stack) {
        state.apply_semimove(children[el.first].semimove);
        complete_turn(state);
    }
    while (!nodes[node_index].is_terminal() && is_node_fully_expanded(node_index)) {
        const auto child_index = get_best_uct_child_index(node_index);
        const auto current_player = state.get_current_player();
        state.apply_semimove(children[child_index].semimove);
        complete_turn(state);
        children_stack.emplace_back(child_index, current_player);
        node_index = children[child_index].index;
        if (node_index == 0) {
            assert(false);
            break;
        }
    }
    if (nodes[node_index].is_terminal()) {
        for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
            results[i - 1] = state.get_player_score(i);
        }
        for (const auto [index, player] : children_stack) {
            nodes[children[index].index].sim_count++;
            children[index].sim_count++;
            children[index].total_score += results[player - 1];
        }
        nodes.front().sim_count++;
        children_stack.clear();
    }
    else {
        const auto current_player = state.get_current_player();
        const auto child_index = get_unvisited_child_index(node_index, current_player);
        state.apply_semimove(children[child_index].semimove);
        complete_turn(state);
        auto state_copy = state;
        if (play(state_copy, cache, results)) {
            auto new_node_index = create_node(state);
            nodes[new_node_index].sim_count = 1;
            children[child_index].index = new_node_index;
            children[child_index].sim_count = 1;
            children[child_index].total_score += results[current_player - 1];
            for (const auto [index, player] : children_stack) {
                nodes[children[index].index].sim_count++;
                children[index].sim_count++;
                children[index].total_score += results[player - 1];
            }
            nodes.front().sim_count++;
            children_stack.clear();
        }
        else {
            auto& end = nodes[node_index].children_range.second;
            if (child_index != end - 1) {
                std::swap(children[child_index], children[end - 1]);
            }
            --end;
        }
    }
    reset_path = false;
}

void Tree::reparent_along_move(const reasoner::move& move) {
    root_state.apply_move(move);
    complete_turn(root_state);
    uint index = 0;
    if constexpr (SEMILENGTH == 1) {
        for (const auto& action : move.mr) {
            const auto [fst, lst] = nodes[index].children_range;
            auto i = fst;
            while (i < lst) {
                if (children[i].semimove.get_actions().front() == action) {
                    index = children[i].index;
                    break;
                }
                ++i;
            }
            if (i == lst || index == 0) {
                break;
            }
        }
    }
    else {
        // not implemented
        assert(false);
    }
    if (index == 0) {
        nodes.clear();
        children.clear();
        create_node(root_state);
        return;
    }
    std::vector<Node> nodes_tmp;
    std::vector<Child> children_tmp;
    nodes_tmp.reserve(nodes.size());
    children_tmp.reserve(children.size());
    fix_tree(nodes_tmp, children_tmp, index);
    nodes = std::move(nodes_tmp);
    children = std::move(children_tmp);
    reset_path = true;
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
