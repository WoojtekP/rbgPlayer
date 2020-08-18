#ifndef MOVESTREE
#define MOVESTREE

#include <vector>

#include "tree_nodes.hpp"
#include "constants.hpp"


class MovesTree {
private:
    std::vector<cell_node> cell_nodes;
    std::vector<index_node> index_nodes = {{}};

    int get_index_node(const int, const int);
    int get_cell_node(const int, const int);
    int get_index_node_by_move_representation(const reasoner::move_representation&, int);
    void update_score_at_cell_node(const int, const int, const score_type, const score_type);
    void update_score_at_index_node(const int, const score_type, const score_type);
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
    double get_score_or_default_value(const T& semimove, const int context = 0) {
        const auto i_node = get_index_node_by_move_representation(semimove.mr, context);
        const auto c_node = get_cell_node(i_node, semimove.cell - 1);
        auto& states_scores = cell_nodes[c_node].states_scores;
        auto it = std::lower_bound(states_scores.begin(), states_scores.end(), semimove.state);
        if (it == states_scores.end() || it->state != semimove.state || it->total_score.weight == 0) {
            return EXPECTED_MAX_SCORE;
        }
        return it->total_score.get_score();
    }

    double get_score_or_default_value(const reasoner::move&, const int = 0);
    double get_score_or_default_value(const reasoner::move_representation&, const int);
    void apply_decay_factor();
};

#endif
