#ifndef RAVETREE
#define RAVETREE

#include <algorithm>
#include <vector>

#include "rave_nodes.hpp"
#include "constants.hpp"


class RaveTree {
private:
    std::vector<rave_tree::cell_node> cell_nodes;
    std::vector<rave_tree::index_node> index_nodes = {{}};
    std::vector<int> states;
    std::vector<int> index_nodes_indices;

    int get_index_node(const int, const int);
    int get_cell_node(const int, const int);
    int get_index_node_by_move_representation(const reasoner::move_representation&, int);
    int get_index_node_by_move_representation_if_exists(const reasoner::move_representation&, int);
    void update_at_cell_node(const int, const int);
    void update_at_index_node(const int);
    void init(const int);
    void extend(const int);
    template <typename T>
    void allocate_space(std::vector<T>& vec, const int size) {
        size_t new_size = vec.size() + size;
        if (new_size > vec.capacity()) {
            vec.reserve(2 * new_size);
        }
        vec.resize(new_size);
    }
    template <typename T>
    void allocate_and_initialize(std::vector<T>& vec, const int size, const T init) {
        size_t new_size = vec.size() + size;
        if (new_size > vec.capacity()) {
            vec.reserve(2 * new_size);
        }
        vec.resize(new_size, init);
    }

public:
    template <typename T>
    int insert_or_update(const T& semimove, const int context = 0) {
        auto i_node = get_index_node_by_move_representation(semimove.mr, context);
        auto c_node = get_cell_node(i_node, semimove.cell - 1);
        update_at_cell_node(c_node, semimove.state);
        return i_node;
    }

    int insert_or_update(const reasoner::move&, const int = 0);
    int insert_or_update(const reasoner::move_representation&, const int = 0);

    template <typename T>
    bool find(const T& semimove, const int context = 0) {
        const auto i_node = get_index_node_by_move_representation_if_exists(semimove.mr, context);
        if (i_node == -1) {
            return false;
        }
        const auto c_node = index_nodes[i_node].cell[semimove.cell - 1];
        if (c_node == -1) {
            return false;
        }
        const auto& cell_node = cell_nodes[c_node];
        const auto end_it = states.begin() + cell_node.lst;
        const auto it = std::find(states.begin() + cell_node.fst, end_it, semimove.state);
        return it != end_it;
    }

    bool find(const reasoner::move&, const int = 0);
    bool find(const reasoner::move_representation&, const int);
    void reset();
    int get_context(const reasoner::move_representation&, const int = 0);
};

#endif
