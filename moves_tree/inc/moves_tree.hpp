#ifndef MOVESTREE
#define MOVESTREE

#include <algorithm>
#include <vector>

#include "tree_nodes.hpp"
#include "constants.hpp"


class MovesTree {
private:
    std::vector<cell_node> cell_nodes;
    std::vector<index_node> index_nodes = {{}};
    std::vector<state_score> states_scores;
    std::vector<int> index_nodes_indices;

    int get_index_node(const int, const int);
    int get_cell_node(const int, const int);
    int get_index_node_by_move_representation(const reasoner::move_representation&, int);
    int get_index_node_by_move_representation_if_exists(const reasoner::move_representation&, int);
    void update_score_at_cell_node(const int, const int, const score_type, const score_type);
    void update_score_at_index_node(const int, const score_type, const score_type);
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
    int insert_or_update(const T& semimove, const score_type score, const score_type weight, const int context = 0) {
        auto i_node = get_index_node_by_move_representation(semimove.mr, context);
        auto c_node = get_cell_node(i_node, semimove.cell - 1);
        update_score_at_cell_node(c_node, semimove.state, score, weight);
        return i_node;
    }

    int insert_or_update(const reasoner::move&, const score_type, const score_type, const int = 0);
    int insert_or_update(const reasoner::move_representation&, const score_type, const score_type, const int);

    template <typename T>
    score get_score_or_default_value(const T& semimove, const int context = 0) {
        const auto i_node = get_index_node_by_move_representation_if_exists(semimove.mr, context);
        if (i_node == -1) {
            return {};
        }
        const auto c_node = index_nodes[i_node].cell[semimove.cell - 1];
        if (c_node == -1) {
            return {};
        }
        const auto& cell_node = cell_nodes[c_node];
        const auto end_it = states_scores.begin() + cell_node.lst;
        const auto it = std::find(states_scores.begin() + cell_node.fst, end_it, semimove.state);
        if (it == end_it) {
            return {};
        }
        return it->total_score;
    }

    score get_score_or_default_value(const reasoner::move&, const int = 0);
    score get_score_or_default_value(const reasoner::move_representation&, const int);
    void apply_decay_factor();
    int get_context(const reasoner::move_representation&, const int = 0);
};

#endif
