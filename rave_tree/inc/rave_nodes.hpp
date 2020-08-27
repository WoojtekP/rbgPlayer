#ifndef RAVE_NODE
#define RAVE_NODE

#include "reasoner.hpp"
#include "types.hpp"


struct state_turn {
    state_turn()=default;
    state_turn(const int state, const int turn) : state(state), turn(turn) {}
    int state = -1;
    int turn = -1;
    bool operator<(const int rhs) {
        return state < rhs;
    }
    bool operator==(const int& rhs) const {
        return state == rhs;
    }
};

struct cell_node {
    cell_node() {
        for (int i = 0; i < reasoner::NUMBER_OF_MODIFIERS; ++i) {
            index[i] = -1;
        }
    }
    int index[reasoner::NUMBER_OF_MODIFIERS];
    int fst = -1;
    int lst = -1;
    int size = 0;
};

struct index_node {
    index_node() {
        for (int i = 0; i < reasoner::BOARD_SIZE; ++i) {
            cell[i] = -1;
        }
    }
    int cell[reasoner::BOARD_SIZE];
    int turn = -1;
};

#endif
