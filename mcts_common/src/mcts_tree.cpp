#include "mcts_tree.hpp"
#include "game_state.hpp"
#include "node.hpp"
#include "constants.hpp"
#include "random_generator.hpp"
#include "move_chooser.hpp"

MctsTree::MctsTree()
#if RAVE
    : moves_set(nodes, children)
#endif
{
    nodes.reserve(4 * MEBIBYTE / sizeof(node));
    children.reserve(4 * MEBIBYTE / sizeof(child));
}

bool MctsTree::is_node_fully_expanded(const uint node_index) {
    // assuming number of children > 0
    if (!nodes[node_index].is_expanded()) {
        return false;
    }
    assert(nodes[node_index].children_range.second > nodes[node_index].children_range.first);
    bool result = children[nodes[node_index].children_range.second - 1].index != 0;
    #ifndef NDEBUG
    if (result) {
        for (auto i = nodes[node_index].children_range.first; i < nodes[node_index].children_range.second; ++i) {
            assert(children[i].index != 0);
        }
    }
    #endif
    return result;
}

void MctsTree::complete_turn(GameState& state) {
    while (state.get_current_player() == KEEPER && state.apply_any_move(cache));
}

void MctsTree::get_scores_from_state(GameState& state, simulation_result& results) {
    for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
}

uint MctsTree::get_unvisited_child_index(std::vector<child>& children, const node& node, const uint node_sim_count, const int) {
    auto [fst, lst] = node.children_range;
    assert(fst < lst);
    auto lower = std::min(fst + node_sim_count, lst - 1);
    while (lower > fst && children[lower - 1].index == 0) {
        --lower;
    }
    #ifndef NDEBUG
    for (auto i = fst; i < lower; ++i) {
        assert(children[i].index > 0);
    }
    for (auto i = lower; i < lst; ++i) {
        assert(children[i].index == 0);
    }
    #endif
    assert(children[lst - 1].index == 0);
    RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
    uint chosen_child = lower + rand_gen.uniform_choice(lst - lower);
    if (chosen_child != lower) {
        std::swap(children[chosen_child], children[lower]);
    }
    return lower;
}

#if RAVE
void MctsTree::get_amaf_scores(std::vector<std::tuple<amaf_score, uint, bool>>& amaf_scores, uint node_pos, uint found_counter) {
    if (node_pos > 0 && children[std::get<1>(children_stack[node_pos - 1])].sim_count <= REF) {
        get_amaf_scores(amaf_scores, node_pos - 1, found_counter);
        return;
    }
    const uint node_index = (node_pos == 0) ? 0 : children[std::get<1>(children_stack[node_pos - 1])].index;
    const auto [fst, lst] = nodes[node_index].children_range;
    for (uint i = fst; i < lst; ++i) {
        for (auto& [score, index, found] : amaf_scores) {
            if (found) {
                continue;
            }
            if (children[index].get_edge() == children[i].get_edge()) {
                score = children[i].amaf;
                found = true;
                ++found_counter;
                if (found_counter == amaf_scores.size()) {
                    return;
                }
            }
        }
    }
    if (node_pos > 0) {
        get_amaf_scores(amaf_scores, node_pos - 1, found_counter);
    }
}
#endif

uint MctsTree::get_best_uct_child_index(const uint node_index, const uint node_sim_count) {
    static std::vector<uint> children_indices;
    children_indices.clear();
    const double c_sqrt_logn = EXPLORATION_CONSTANT * std::sqrt(std::log(node_sim_count));
    double max_priority = 0.0;
    const auto [fst, lst] = nodes[node_index].children_range;
    #if RAVE > 0
    static std::vector<std::tuple<amaf_score, uint, bool>> amaf_scores;
    amaf_scores.clear();
    if (node_sim_count <= REF && root_sim_count > REF) {
        assert(node_index != 0);
        assert(!children_stack.empty());
        amaf_scores.reserve(lst - fst);
        for (uint i = fst; i < lst; ++i) {
            amaf_scores.emplace_back(children[i].amaf, i, false);
        }
        get_amaf_scores(amaf_scores, children_stack.size() - 1, 0);
    }
    const double beta = std::sqrt(EQUIVALENCE_PARAMETER / (3.0 * node_sim_count + EQUIVALENCE_PARAMETER));
    #endif
    for (uint i = fst; i < lst; ++i) {
        assert(children[i].sim_count > 0);
        double priority = static_cast<double>(children[i].total_score) / EXPECTED_MAX_SCORE / children[i].sim_count;
        #if RAVE > 0
            constexpr bool rave = REF == 0;
            const auto& amaf = (rave || amaf_scores.empty()) ? children[i].amaf : std::get<0>(amaf_scores[i - fst]);
            double amaf_score;
            #if RAVE == 3
            if (amaf.count < RAVEMIX_THRESHOLD)
                amaf_score = static_cast<double>(amaf.score_base) / EXPECTED_MAX_SCORE / amaf.count_base; else
            #endif
            amaf_score = static_cast<double>(amaf.score) / EXPECTED_MAX_SCORE / amaf.count;
            priority = priority * (1.0 - beta) + amaf_score * beta;
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

uint MctsTree::get_top_ranked_child_index(const uint node_index) {
    static std::vector<uint> children_indices;
    children_indices.clear();
    double max_score = -1.0;
    uint max_sim = 0;
    const auto [fst, lst] = nodes[node_index].children_range;
    assert(fst < lst);
    for (auto i = fst; i < lst; ++i) {
        if (children[i].sim_count == 0)
            continue;
        double score = static_cast<double>(children[i].total_score) / children[i].sim_count;
        // if (children[i].sim_count > max_sim || (children[i].sim_count == max_sim && score > max_score)) {
        if (score > max_score || (score == max_score && children[i].sim_count > max_sim)) {
            max_score = score;
            max_sim = children[i].sim_count;
            children_indices.resize(1);
            children_indices[0] = i;
        }
        else if (score == max_score && children[i].sim_count == max_sim) {
            children_indices.push_back(i);
        }
    }
    assert(children_indices.size() > 0);
    return children_indices[RBGRandomGenerator::get_instance().uniform_choice(children_indices.size())];
}

void MctsTree::root_at_index(const uint root_index) {
    static std::vector<node> nodes_tmp;
    static std::vector<child> children_tmp;
    nodes_tmp.clear();
    children_tmp.clear();
    nodes_tmp.reserve(nodes.size());
    children_tmp.reserve(children.size());
    fix_tree(nodes_tmp, children_tmp, root_index);
    std::swap(nodes, nodes_tmp);
    std::swap(children, children_tmp);
}

uint MctsTree::fix_tree(std::vector<node>& nodes_tmp, std::vector<child>& children_tmp, const uint index) {
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
