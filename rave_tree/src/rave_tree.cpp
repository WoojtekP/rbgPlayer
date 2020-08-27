#include <algorithm>
#include <vector>

#include "rave_tree.hpp"
#include "reasoner.hpp"
#include "constants.hpp"


int RaveTree::get_index_node(const int c_node, const int index) {
    auto i_node = cell_nodes[c_node].index[index];
    if (i_node == -1) {
        i_node = cell_nodes[c_node].index[index] = index_nodes.size();
        index_nodes.emplace_back();
    }
    return i_node;
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
    allocate_space(cell_node.size);
}

void RaveTree::extend(const int c_node) {
    auto& cell_node = cell_nodes[c_node];
    const auto old_fst = cell_node.fst;
    const auto old_lst = cell_node.lst;
    cell_node.fst = states_turns.size();
    cell_node.lst = cell_node.fst + (old_lst - old_fst);
    cell_node.size *= 2;
    allocate_space(cell_node.size);
    std::copy(states_turns.begin() + old_fst, states_turns.begin() + old_lst, states_turns.begin() + cell_node.fst);
}

void RaveTree::allocate_space(const int size) {
    size_t new_size = states_turns.size() + size;
    if (new_size > states_turns.capacity()) {
        states_turns.reserve(2 * new_size);
    }
    states_turns.resize(new_size);
}

int RaveTree::find(const reasoner::move_representation& mr, const int context) {
    const auto i_node = get_index_node_by_move_representation(mr, context);
    return index_nodes[i_node].turn;
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
    turn = 1;
}

int RaveTree::get_context(const reasoner::move_representation& mr, const int context) {
    return get_index_node_by_move_representation(mr, context);
}
