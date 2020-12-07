#ifndef ACTIONSARRAYS
#define ACTIONSARRAYS

#include "constants.hpp"
#include "reasoner.hpp"


struct score {
    double sum = 0;
    double weight = 0;
    score() = default;
    score(const double _sum, const double _weight) : sum(_sum), weight(_weight) {}
    inline double get_score() const {
        return sum / weight;
    }
};

class ActionsArrays {
private:
    static constexpr uint size = reasoner::BOARD_SIZE * (reasoner::NUMBER_OF_MODIFIERS + reasoner::AUTOMATON_SIZE);
    score arr[ActionsArrays::size];
    uint action_to_index(const reasoner::action_representation);
public:
    ActionsArrays();
    int insert_or_update(const reasoner::action_representation, const double, const int);
    int insert_or_update(const reasoner::move&, const double, const int);
    score get_score_or_default_value(const reasoner::action_representation, const int);
    score get_score_or_default_value(const reasoner::move&, const int);
    void apply_decay_factor();
};

#endif
