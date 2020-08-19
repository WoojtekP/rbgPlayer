#include <vector>

#include "moves_tree.hpp"
#include "reasoner.hpp"
#include "constants.hpp"


int MovesTree::get_index_node(const int c_node, const int index) {
    auto i_node = cell_nodes[c_node].index[index];
    if (i_node == -1) {
        i_node = cell_nodes[c_node].index[index] = index_nodes.size();
        index_nodes.emplace_back();
    }
    return i_node;
}

int MovesTree::get_cell_node(const int i_node, const int cell) {
    auto c_node = index_nodes[i_node].cell[cell];
    if (c_node == -1) {
        c_node = index_nodes[i_node].cell[cell] = cell_nodes.size();
        cell_nodes.emplace_back();
    }
    return c_node;
}

int MovesTree::get_index_node_by_move_representation(const reasoner::move_representation& mr, int i_node) {
    for (const auto& action : mr) {
        const auto c_node = get_cell_node(i_node, action.cell - 1);
        const auto modifier_index = reasoner::action_to_modifier_index(action.index);
        i_node = get_index_node(c_node, modifier_index);
    }
    return i_node;
}

void MovesTree::update_score_at_cell_node(const int c_node, const int state, const score_type score, const score_type weight) {
    auto& states_scores = cell_nodes[c_node].states_scores;
    auto it = std::lower_bound(states_scores.begin(), states_scores.end(), state);
    if (it == states_scores.end() || it->state != state) {
        states_scores.emplace(it, state, score, weight);
    }
    else {
        it->total_score.sum += score;
        it->total_score.weight += weight;
    }
}

void MovesTree::update_score_at_index_node(const int i_node, const score_type score, const score_type weight) {
    index_nodes[i_node].total_score.sum += score;
    index_nodes[i_node].total_score.weight += weight;
}

double MovesTree::get_score_or_default_value(const reasoner::move_representation& mr, const int context) {
    const auto i_node = get_index_node_by_move_representation(mr, context);
    if (index_nodes[i_node].total_score.weight == 0) {
        return EXPECTED_MAX_SCORE;
    }
    return index_nodes[i_node].total_score.get_score();
}

int MovesTree::insert_or_update(const reasoner::move_representation& mr, const score_type score, const score_type weight, const int context) {
    const auto i_node = get_index_node_by_move_representation(mr, context);
    update_score_at_index_node(i_node, score, weight);
    return i_node;
}

int MovesTree::insert_or_update(const reasoner::move& move, const score_type score, const score_type weight, const int context) {
    return insert_or_update(move.mr, score, weight, context);
}

double MovesTree::get_score_or_default_value(const reasoner::move& move, const int context) {
    return get_score_or_default_value(move.mr, context);
}

void MovesTree::apply_decay_factor() {
    for (auto& node : index_nodes) {
        node.total_score.sum *= DECAY_FACTOR;
        node.total_score.weight *= DECAY_FACTOR;
    }
    for (auto& node : cell_nodes) {
        for (auto state_score : node.states_scores) {
            state_score.total_score.sum *= DECAY_FACTOR;
            state_score.total_score.weight *= DECAY_FACTOR;
        }
    }
}

int MovesTree::get_context(const reasoner::move_representation& mr, const int context) {
    return get_index_node_by_move_representation(mr, context);
}
