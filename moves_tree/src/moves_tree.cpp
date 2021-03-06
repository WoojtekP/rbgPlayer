#include <algorithm>
#include <vector>

#include "moves_tree.hpp"
#include "reasoner.hpp"
#include "constants.hpp"


int MovesTree::get_index_node(const int c_node, const int index) {
    auto& cell_node = cell_nodes[c_node];
    switch (cell_node.status) {
        case moves_tree::node_status::empty: {
            cell_node.status = moves_tree::node_status::one_index;
            cell_node.index.value = index;
            cell_node.index.node = index_nodes.size();
            index_nodes.emplace_back();
            return cell_node.index.node;
        }
        case moves_tree::node_status::one_index: {
            assert(cell_node.index.node != -1);
            assert(cell_node.index.value >= 0 && cell_node.index.value <= reasoner::NUMBER_OF_MODIFIERS);
            if (index == cell_node.index.value) {
                return cell_node.index.node;
            }
            cell_node.status = moves_tree::node_status::expanded;
            const auto offset = index_nodes_indices.size();
            allocate_and_initialize(index_nodes_indices, reasoner::NUMBER_OF_MODIFIERS, -1);
            index_nodes_indices[offset + cell_node.index.value] = cell_node.index.node;
            cell_node.offset = offset;
            break;
        }
        case moves_tree::node_status::expanded: {
            assert(cell_node.offset % reasoner::NUMBER_OF_MODIFIERS == 0);
            if (index_nodes_indices[cell_node.offset + index] > 0) {
                return index_nodes_indices[cell_node.offset + index];
            }
        }
    }
    assert(index_nodes_indices[cell_node.offset + index] == -1);
    index_nodes_indices[cell_node.offset + index] = index_nodes.size();
    index_nodes.emplace_back();
    return index_nodes_indices[cell_node.offset + index];
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
    if (i_node >= static_cast<int>(index_nodes.size())) {
        return -1;
    }
    for (const auto& action : mr) {
        const auto c_node = index_nodes[i_node].cell[action.cell - 1];
        if (c_node == -1) {
            return -1;
        }
        const auto modifier_index = reasoner::action_to_modifier_index(action.index);
        const auto& cell_node = cell_nodes[c_node];
        switch (cell_node.status) {
            case moves_tree::node_status::empty: {
                return -1;
            }
            case moves_tree::node_status::one_index: {
                if (modifier_index == cell_node.index.value) {
                    i_node = cell_node.index.node;
                    assert(i_node != -1);
                    break;
                }
                return -1;
            }
            case moves_tree::node_status::expanded: {
                assert(cell_node.offset % reasoner::NUMBER_OF_MODIFIERS == 0);
                i_node = index_nodes_indices[cell_node.offset + modifier_index];
            }
        }
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
    allocate_space(states_scores, cell_node.size);
}

void MovesTree::extend(const int c_node) {
    auto& cell_node = cell_nodes[c_node];
    const auto old_fst = cell_node.fst;
    const auto old_lst = cell_node.lst;
    cell_node.fst = states_scores.size();
    cell_node.lst = cell_node.fst + (old_lst - old_fst);
    cell_node.size *= 2;
    allocate_space(states_scores, cell_node.size);
    std::copy(states_scores.begin() + old_fst, states_scores.begin() + old_lst, states_scores.begin() + cell_node.fst);
}

moves_tree::score MovesTree::get_score_or_default_value(const reasoner::move_representation& mr, const int context) {
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

moves_tree::score MovesTree::get_score_or_default_value(const reasoner::move& move, const int context) {
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
