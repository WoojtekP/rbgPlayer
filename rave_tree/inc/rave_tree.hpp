#ifndef RAVETREE
#define RAVETREE

#include <vector>

#include "rave_nodes.hpp"
#include "constants.hpp"


class RaveTree {
private:
    std::vector<cell_node> cell_nodes;
    std::vector<index_node> index_nodes = {{}};
    int turn = 1;

    int get_index_node(const int, const int);
    int get_cell_node(const int, const int);
    int get_index_node_by_move_representation(const reasoner::move_representation&, int);
    void update_at_cell_node(const int, const int);
    void update_at_index_node(const int);
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
        const auto i_node = get_index_node_by_move_representation(semimove.mr, context);
        const auto c_node = get_cell_node(i_node, semimove.cell - 1);
        auto& states_turns = cell_nodes[c_node].states_turns;
        auto it = std::lower_bound(states_turns.begin(), states_turns.end(), semimove.state);
        if (it == states_turns.end() || it->state != semimove.state) {
            return -1;
        }
        return it->turn;
    }

    int find(const reasoner::move&, const int = 0);
    int find(const reasoner::move_representation&, const int);
    void reset();
};

#endif
