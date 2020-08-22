#include "mcts_tree.hpp"
#include "node.hpp"
#include "constants.hpp"
#include "random_generator.hpp"
#include "move_chooser.hpp"


MctsTree::MctsTree(const reasoner::game_state& initial_state) : root_state(initial_state) {
    nodes.reserve(4 * MEBIBYTE / sizeof(Node));
    children.reserve(4 * MEBIBYTE / sizeof(Child));
}

bool MctsTree::is_node_fully_expanded(const uint node_index) {
    // assuming number of children > 0
    if (!nodes[node_index].is_expanded()) {
        return false;
    }
    assert(nodes[node_index].children_range.second > nodes[node_index].children_range.first);
    bool result = children[nodes[node_index].children_range.second - 1].index != 0;
    if (result) {
        for (auto i = nodes[node_index].children_range.first; i < nodes[node_index].children_range.second; ++i) {
            assert(children[i].index != 0);
        }
    }
    return result;
}

void MctsTree::complete_turn(reasoner::game_state& state) {
    while (state.get_current_player() == KEEPER && state.apply_any_move(cache));
}

void MctsTree::get_scores_from_state(reasoner::game_state& state, simulation_result& results) {
    for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
}

uint MctsTree::get_unvisited_child_index(std::vector<Child>& children, const Node& node, const uint node_sim_count, const int) {
    auto [fst, lst] = node.children_range;
    assert(fst < lst);
    auto lower = std::min(fst + node_sim_count, lst - 1);
    while (lower > fst && children[lower - 1].index == 0) {
        --lower;
    }
    assert(children[lst - 1].index == 0);
    RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
    uint chosen_child = lower + rand_gen.uniform_choice(lst - lower);
    if (chosen_child != lower) {
        std::swap(children[chosen_child], children[lower]);
    }
    return lower;
}

uint MctsTree::get_best_uct_child_index(const uint node_index, const uint node_sim_count) {
    static std::vector<uint> children_indices;
    children_indices.clear();
    const double c_sqrt_logn = EXPLORATION_CONSTANT * std::sqrt(std::log(node_sim_count));
    double max_priority = 0.0;
    #if RAVE > 0
    const double beta = std::sqrt(EQUIVALENCE_PARAMETER / static_cast<double>(3 * node_sim_count + EQUIVALENCE_PARAMETER));
    #endif
    const auto [fst, lst] = nodes[node_index].children_range;
    for (uint i = fst; i < lst; ++i) {
        double priority = children[i].total_score / EXPECTED_MAX_SCORE / children[i].sim_count;
        #if RAVE > 0
        if (children[i].amaf_count > 0) {
            priority *= 1.0 - beta;
            priority += beta * children[i].amaf_score / EXPECTED_MAX_SCORE / children[i].amaf_count;
        }
        #endif
        priority += c_sqrt_logn / std::sqrt(static_cast<double>(children[i].sim_count));
        if (priority > max_priority) {
            max_priority = priority;
            children_indices.resize(1);
            children_indices[0] = i;
        }
        else if (priority == max_priority) {
            children_indices.push_back(i);
        }
    }
    RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
    return children_indices[rand_gen.uniform_choice(children_indices.size())];
}

void MctsTree::root_at_index(const uint root_index) {
    static std::vector<Node> nodes_tmp;
    static std::vector<Child> children_tmp;
    nodes_tmp.clear();
    children_tmp.clear();
    nodes_tmp.reserve(nodes.size());
    children_tmp.reserve(children.size());
    fix_tree(nodes_tmp, children_tmp, root_index);
    std::swap(nodes, nodes_tmp);
    std::swap(children, children_tmp);
    #if MAST > 0
    move_chooser.complete_turn();
    #endif
}

uint MctsTree::fix_tree(std::vector<Node>& nodes_tmp, std::vector<Child>& children_tmp, const uint index) {
    auto new_index = nodes_tmp.size();
    nodes_tmp.push_back(nodes[index]);
    if (!nodes[index].is_expanded()) {
        return new_index;
    }
    auto first_child_index = children_tmp.size();
    const auto [fst, lst] = nodes[index].children_range;
    auto child_count = lst - fst;
    nodes_tmp[new_index].children_range = std::make_pair(first_child_index, first_child_index + child_count);
    for (uint i = 0; i < child_count; ++i) {
        children_tmp.push_back(children[fst + i]);
    }
    for (uint i = 0; i < child_count; ++i) {
        if (children[fst + i].index == 0) {
            continue;
        }
        children_tmp[first_child_index + i].index = fix_tree(nodes_tmp, children_tmp, children[fst + i].index);
    }
    return new_index;
}
