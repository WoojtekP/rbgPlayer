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
    auto& states_turns = cell_nodes[c_node].states_turns;
    auto it = std::lower_bound(states_turns.begin(), states_turns.end(), state);
    if (it == states_turns.end() || it->state != state) {
        states_turns.emplace(it, state, turn);
    }
    else {
        it->turn = turn++;
    }
}

void RaveTree::update_at_index_node(const int i_node) {
    index_nodes[i_node].turn = turn++;
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
    turn = 1;
}
