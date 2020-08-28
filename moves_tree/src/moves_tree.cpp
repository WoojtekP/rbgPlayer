#include <algorithm>
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

int MovesTree::get_index_node_by_move_representation_if_exists(const reasoner::move_representation& mr, int i_node) {
    if (i_node >= (int)index_nodes.size()) {
        return -1;
    }
    for (const auto& action : mr) {
        const auto c_node = index_nodes[i_node].cell[action.cell - 1];
        if (c_node == -1) {
            return -1;
        }
        const auto modifier_index = reasoner::action_to_modifier_index(action.index);
        i_node = cell_nodes[c_node].index[modifier_index];
        if (i_node == -1) {
            return -1;
        }
    }
    return i_node;
}

void MovesTree::update_score_at_cell_node(const int c_node, const int state, const score_type score, const score_type weight) {
    const auto& cell_node = cell_nodes[c_node];
    if (cell_node.size == 0) {
        init(c_node);
        auto& lst = cell_nodes[c_node].lst;
        states_scores[lst].total_score.sum = score;
        states_scores[lst].total_score.weight = weight;
        states_scores[lst].state = state;
        ++lst;
    }
    else {
        const auto end_it = states_scores.begin() + cell_node.lst;
        const auto it = std::find(states_scores.begin() + cell_node.fst, end_it, state);
        if (it == end_it) {
            if (cell_node.lst - cell_node.fst == cell_node.size) {
                extend(c_node);
            }
            auto& lst = cell_nodes[c_node].lst;
            states_scores[lst].total_score.sum = score;
            states_scores[lst].total_score.weight = weight;
            states_scores[lst].state = state;
            ++lst;
        }
        else {
            it->total_score.sum += score;
            it->total_score.weight += weight;
        }
    }
}

void MovesTree::update_score_at_index_node(const int i_node, const score_type score, const score_type weight) {
    index_nodes[i_node].total_score.sum += score;
    index_nodes[i_node].total_score.weight += weight;
}

void MovesTree::init(const int c_node) {
    auto& cell_node = cell_nodes[c_node];
    cell_node.fst = cell_node.lst = states_scores.size();
    cell_node.size = 1;
    allocate_space(cell_node.size);
}

void MovesTree::extend(const int c_node) {
    auto& cell_node = cell_nodes[c_node];
    const auto old_fst = cell_node.fst;
    const auto old_lst = cell_node.lst;
    cell_node.fst = states_scores.size();
    cell_node.lst = cell_node.fst + (old_lst - old_fst);
    cell_node.size *= 2;
    allocate_space(cell_node.size);
    std::copy(states_scores.begin() + old_fst, states_scores.begin() + old_lst, states_scores.begin() + cell_node.fst);
}

void MovesTree::allocate_space(const int size) {
    size_t new_size = states_scores.size() + size;
    if (new_size > states_scores.capacity()) {
        states_scores.reserve(2 * new_size);
    }
    states_scores.resize(new_size);
}

score MovesTree::get_score_or_default_value(const reasoner::move_representation& mr, const int context) {
    const auto i_node = get_index_node_by_move_representation_if_exists(mr, context);
    if (i_node == -1) {
        return {};
    }
    return index_nodes[i_node].total_score;
}

int MovesTree::insert_or_update(const reasoner::move_representation& mr, const score_type score, const score_type weight, const int context) {
    const auto i_node = get_index_node_by_move_representation(mr, context);
    update_score_at_index_node(i_node, score, weight);
    return i_node;
}

int MovesTree::insert_or_update(const reasoner::move& move, const score_type score, const score_type weight, const int context) {
    return insert_or_update(move.mr, score, weight, context);
}

score MovesTree::get_score_or_default_value(const reasoner::move& move, const int context) {
    return get_score_or_default_value(move.mr, context);
}

void MovesTree::apply_decay_factor() {
    for (auto& node : index_nodes) {
        node.total_score.sum *= DECAY_FACTOR;
        node.total_score.weight *= DECAY_FACTOR;
    }
    for (auto state_score : states_scores) {
        state_score.total_score.sum *= DECAY_FACTOR;
        state_score.total_score.weight *= DECAY_FACTOR;
    }
}

int MovesTree::get_context(const reasoner::move_representation& mr, const int context) {
    return get_index_node_by_move_representation(mr, context);
}
