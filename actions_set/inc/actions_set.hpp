#ifndef ACTIONSSET
#define ACTIONSSET

#include <bitset>

#include "constants.hpp"
#include "reasoner.hpp"


class ActionsSet {
private:
    static constexpr uint size = reasoner::BOARD_SIZE * (reasoner::NUMBER_OF_MODIFIERS + reasoner::AUTOMATON_SIZE);
    std::bitset<size> actions_bitset;
    uint action_to_index(const reasoner::action_representation);
public:
    ActionsSet();
    int insert(const reasoner::action_representation, const double, const int);
    bool find(const reasoner::action_representation, const int);
    void reset();
};

#endif
