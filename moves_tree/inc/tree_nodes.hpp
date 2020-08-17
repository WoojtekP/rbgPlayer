#ifndef TRIE_NODE
#define TRIE_NODE

#include "reasoner.hpp"
#include "types.hpp"


typedef double score_type;

struct score {
    score()=default;
    score(const score_type sum, const score_type weight) : sum(sum), weight(weight) {}
    score_type sum = 0;
    score_type weight = 0;
    double get_score() const {
        return static_cast<double>(sum) / static_cast<double>(weight);
    }
};

struct state_score {
    state_score()=default;
    state_score(const int state, const score_type score, const score_type weight) : state(state), total_score(score, weight) {}
    int state;
    score total_score;
    bool operator<(const int& rhs) const {
        return state < rhs;
    }
};

struct cell_node {
    cell_node() {
        for (int i = 0; i < reasoner::NUMBER_OF_MODIFIERS; ++i) {
            index[i] = -1;
        }
    }
    int index[reasoner::NUMBER_OF_MODIFIERS];
    std::vector<state_score> states_scores;
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
