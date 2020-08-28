#ifndef RAVETREE
#define RAVETREE

#include <algorithm>
#include <vector>

#include "rave_nodes.hpp"
#include "constants.hpp"


class RaveTree {
private:
    std::vector<cell_node> cell_nodes;
    std::vector<index_node> index_nodes = {{}};
    std::vector<state_turn> states_turns;
    int turn = 1;

    int get_index_node(const int, const int);
    int get_cell_node(const int, const int);
    int get_index_node_by_move_representation(const reasoner::move_representation&, int);
    int get_index_node_by_move_representation_if_exists(const reasoner::move_representation&, int);
    void update_at_cell_node(const int, const int);
    void update_at_index_node(const int);
    void init(const int);
    void extend(const int);
    void allocate_space(const int);
public:
    template <typename T>
    int insert_or_update(const T& semimove, const int context = 0) {
        auto i_node = get_index_node_by_move_representation(semimove.mr, context);
        auto c_node = get_cell_node(i_node, semimove.cell - 1);
        update_at_cell_node(c_node, semimove.state);
        return i_node;
    }

    int insert_or_update(const reasoner::move&, const int = 0);
    int insert_or_update(const reasoner::move_representation&, const int);

    template <typename T>
    int find(const T& semimove, const int context = 0) {
        const auto i_node = get_index_node_by_move_representation_if_exists(semimove.mr, context);
        if (i_node == -1) {
            return -1;
        }
        const auto c_node = index_nodes[i_node].cell[semimove.cell - 1];
        if (c_node == -1) {
            return -1;
        }
        const auto& cell_node = cell_nodes[c_node];
        const auto end_it = states_turns.begin() + cell_node.lst;
        const auto it = std::find(states_turns.begin() + cell_node.fst, end_it, semimove.state);
        if (it == end_it) {
            return -1;
        }
        return it->turn;
    }

    int find(const reasoner::move&, const int = 0);
    int find(const reasoner::move_representation&, const int);
    void reset();
    int get_context(const reasoner::move_representation&, const int = 0);
};

#endif
