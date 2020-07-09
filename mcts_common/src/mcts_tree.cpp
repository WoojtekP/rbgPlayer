#include "mcts_tree.hpp"
#include "node.hpp"
#include "constants.hpp"
#include "random_generator.hpp"


MctsTree::MctsTree(const reasoner::game_state& initial_state) : root_state(initial_state) {
    nodes.reserve(4 * MEBIBYTE / sizeof(Node));
    children.reserve(4 * MEBIBYTE / sizeof(uint));
}

bool MctsTree::is_node_fully_expanded(const uint node_index) {
    // assuming number of children > 0
    return children[nodes[node_index].children_range.second - 1].index != 0;
}

void MctsTree::complete_turn(reasoner::game_state& state) {
    while (state.get_current_player() == KEEPER && state.apply_any_move(cache));
}

uint MctsTree::get_best_uct_child_index(const uint node_index) {
    // TODO RAVE
    static std::vector<uint> children_indices;
    const auto& [fst, lst] = nodes[node_index].children_range;
    children_indices.resize(1);
    children_indices[0] = fst;
    double const c_sqrt_logn = EXPLORATION_CONSTANT * std::sqrt(std::log(nodes[node_index].sim_count));
    double max_priority = children[fst].total_score / EXPECTED_MAX_SCORE / children[fst].sim_count +
                          c_sqrt_logn / std::sqrt(static_cast<double>(children[fst].sim_count));
    for (uint i = fst + 1; i < lst; ++i) {
        double priority = children[i].total_score / EXPECTED_MAX_SCORE / children[i].sim_count +
                          c_sqrt_logn / std::sqrt(static_cast<double>(children[i].sim_count));
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

uint MctsTree::get_unvisited_child_index(const uint node_index, [[maybe_unused]] const uint current_player) {
    auto [fst, lst] = nodes[node_index].children_range;
    auto lower = fst + nodes[node_index].sim_count;
    while (lower > fst && children[lower - 1].index == 0) {
        --lower;
    }
    RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
    uint chosen_child;
    #if MAST > 0
    static std::uniform_real_distribution<double> prob(0.0, 1.0);
    static std::mt19937 random_numbers_generator;
    static std::vector<uint> children_indices;
    if (prob(random_numbers_generator) < EPSILON) {
        children_indices.clear();
        children_indices.reserve(lst - lower);
        double best_score = 0.0;
        for (uint i = lower; i < lst; ++i) {
            assert(children[i].index == 0);
            double score = moves[current_player - 1].get_score_or_default_value(children[i].get_actions());
            if (score > best_score) {
                best_score = score;
                children_indices.resize(1);
                children_indices[0] = i;
            }
            else if (score == best_score) {
                children_indices.push_back(i);
            }
        }
        chosen_child = children_indices[rand_gen.uniform_choice(children_indices.size())];
    }
    else
    #endif
    chosen_child = lower + rand_gen.uniform_choice(lst - lower);
    if (chosen_child != lower) {
        std::swap(children[chosen_child], children[lower]);
    }
    return lower;
}

uint MctsTree::fix_tree(std::vector<Node>& nodes_tmp, std::vector<Child>& children_tmp, const uint index) {
    auto new_index = nodes_tmp.size();
    nodes_tmp.push_back(nodes[index]);
    auto first_child_index = children_tmp.size();
    const auto& [fst, lst] = nodes[index].children_range;
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

game_status_indication MctsTree::get_status(const int player_index) const {
    if (nodes.front().is_terminal()) {
        return end_game;
    }
    return root_state.get_current_player() == (player_index + 1) ? own_turn : opponent_turn;
}
