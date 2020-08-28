#ifndef TRIE_NODE
#define TRIE_NODE

#include "constants.hpp"
#include "reasoner.hpp"
#include "types.hpp"


typedef double score_type;

struct score {
    score()=default;
    score(const score_type sum, const score_type weight) : sum(sum), weight(weight) {}
    score_type sum = 0;
    score_type weight = 0;
    double get_score() const {
        return weight == 0 ? EXPECTED_MAX_SCORE : static_cast<double>(sum) / static_cast<double>(weight);
    }
};

struct state_score {
    state_score()=default;
    state_score(const int state, const score_type score, const score_type weight) : state(state), total_score(score, weight) {}
    int state = -1;
    score total_score;
    bool operator<(const int& rhs) const {
        return state < rhs;
    }
    bool operator==(const int& rhs) const {
        return state == rhs;
    }
};

namespace moves_tree {
    enum node_status : char {
        empty,
        one_index,
        expanded,
    };
}

struct cell_node {
    cell_node()=default;
    union {
        struct {
            int value;
            int node;
        } index;
        int offset;
    };
    int fst = -1;
    int lst = -1;
    short int size = 0;
    moves_tree::node_status status = moves_tree::node_status::empty;
};

struct index_node {
    index_node() {
        for (int i = 0; i < reasoner::BOARD_SIZE; ++i) {
            cell[i] = -1;
        }
    }
    int cell[reasoner::BOARD_SIZE];
    score total_score;
};

#endif
