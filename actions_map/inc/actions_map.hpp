#ifndef ACTIONSMAP
#define ACTIONSMAP

#include "constants.hpp"
#include "reasoner.hpp"
#include "types.hpp"


class ActionsMap {
private:
    static constexpr uint size = reasoner::BOARD_SIZE * (reasoner::NUMBER_OF_MODIFIERS + reasoner::AUTOMATON_SIZE);
    score arr[ActionsMap::size];
    uint action_to_index(const reasoner::action_representation);
public:
    ActionsMap();
    int insert_or_update(const reasoner::action_representation, const double, const int = 0);
    int insert_or_update(const reasoner::move&, const double, const int = 0);
    score get_score_or_default_value(const reasoner::action_representation, const int = 0);
    score get_score_or_default_value(const reasoner::move&, const int = 0);
    void apply_decay_factor();
};

#endif
