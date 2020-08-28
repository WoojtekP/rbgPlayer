#include <algorithm>
#include <vector>

#include "rave_tree.hpp"
#include "reasoner.hpp"
#include "constants.hpp"


int RaveTree::get_index_node(const int c_node, const int index) {
    auto& cell_node = cell_nodes[c_node];
    switch (cell_node.status) {
        case empty: {
            cell_node.status = node_status::one_index;
            cell_node.index.value = index;
            cell_node.index.node = index_nodes.size();
            index_nodes.emplace_back();
            return cell_node.index.node;
        }
        case one_index: {
            assert(cell_node.index.node != -1);
            assert(cell_node.index.value >= 0 && cell_node.index.value <= reasoner::NUMBER_OF_MODIFIERS);
            if (index == cell_node.index.value) {
                return cell_node.index.node;
            }
            cell_node.status = node_status::expanded;
            const auto offset = index_nodes_indices.size();
            allocate_and_initialize(index_nodes_indices, reasoner::NUMBER_OF_MODIFIERS, -1);
            index_nodes_indices[offset + cell_node.index.value] = cell_node.index.node;
            cell_node.offset = offset;
            break;
        }
        case expanded: {
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

int RaveTree::get_cell_node(const int i_node, const int cell) {
    auto c_node = index_nodes[i_node].cell[cell];
    if (c_node == -1) {
        c_node = index_nodes[i_node].cell[cell] = cell_nodes.size();
        cell_nodes.emplace_back();
        init(c_node);
    }
    return c_node;
}

int RaveTree::get_index_node_by_move_representation(const reasoner::move_representation& mr, int i_node) {
    for (const auto& action : mr) {
        const auto c_node = get_cell_node(i_node, action.cell - 1);
        const auto modifier_index = reasoner::action_to_modifier_index(action.index);
        i_node = get_index_node(c_node, modifier_index);
    }
    return i_node;
}

int RaveTree::get_index_node_by_move_representation_if_exists(const reasoner::move_representation& mr, int i_node) {
    if (i_node >= (int)index_nodes.size()) {
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
            case node_status::empty: {
                return -1;
            }
            case node_status::one_index: {
                if (modifier_index == cell_node.index.value) {
                    i_node = cell_node.index.node;
                    assert(i_node != -1);
                    break;
                }
                return -1;
            }
            case node_status::expanded: {
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

void RaveTree::update_at_cell_node(const int c_node, const int state) {
    const auto& cell_node = cell_nodes[c_node];
    const auto end_it = states_turns.begin() + cell_node.lst;
    const auto it = std::find(states_turns.begin() + cell_node.fst, end_it, state);
    if (it == end_it) {
        if (cell_node.lst - cell_node.fst == cell_node.size) {
            extend(c_node);
        }
        auto& lst = cell_nodes[c_node].lst;
        states_turns[lst].turn = turn++;
        states_turns[lst].state = state;
        ++lst;
    }
    else {
        it->turn = turn++;
    }
}

void RaveTree::update_at_index_node(const int i_node) {
    index_nodes[i_node].turn = turn++;
}

void RaveTree::init(const int c_node) {
    auto& cell_node = cell_nodes[c_node];
    cell_node.fst = cell_node.lst = states_turns.size();
    cell_node.size = 1;
    allocate_space(states_turns, cell_node.size);
}

void RaveTree::extend(const int c_node) {
    auto& cell_node = cell_nodes[c_node];
    const auto old_fst = cell_node.fst;
    const auto old_lst = cell_node.lst;
    cell_node.fst = states_turns.size();
    cell_node.lst = cell_node.fst + (old_lst - old_fst);
    cell_node.size *= 2;
    allocate_space(states_turns, cell_node.size);
    std::copy(states_turns.begin() + old_fst, states_turns.begin() + old_lst, states_turns.begin() + cell_node.fst);
}

int RaveTree::find(const reasoner::move_representation& mr, const int context) {
    const auto i_node = get_index_node_by_move_representation_if_exists(mr, context);
    return (i_node == -1) ? -1 : index_nodes[i_node].turn;
}

int RaveTree::insert_or_update(const reasoner::move_representation& mr, const int context) {
    const auto i_node = get_index_node_by_move_representation(mr, context);
    update_at_index_node(i_node);
    return i_node;
}

int RaveTree::insert_or_update(const reasoner::move& move, const int context) {
    return insert_or_update(move.mr, context);
}

int RaveTree::find(const reasoner::move& move, const int context) {
    return find(move.mr, context);
}

void RaveTree::reset() {
    cell_nodes.clear();
    index_nodes.clear();
    index_nodes.emplace_back();
    states_turns.clear();
    index_nodes_indices.clear();
    turn = 1;
}

int RaveTree::get_context(const reasoner::move_representation& mr, const int context) {
    return get_index_node_by_move_representation(mr, context);
}
