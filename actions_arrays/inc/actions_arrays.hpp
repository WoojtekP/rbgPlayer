#ifndef ACTIONSARRAYS
#define ACTIONSARRAYS

#include "constants.hpp"
#include "reasoner.hpp"


class ActionsArrays {
private:
    struct score {
        double sum;
        double weight;
    };
    static constexpr uint size1 = reasoner::BOARD_SIZE * reasoner::NUMBER_OF_MODIFIERS;
    static constexpr uint size2 = reasoner::BOARD_SIZE * reasoner::AUTOMATON_SIZE;
    score arr1[ActionsArrays::size1];
    score arr2[ActionsArrays::size2];
    void insert_or_update(const reasoner::move_representation&, const double, const double);

public:
    ActionsArrays();
    template <typename T>
    int insert_or_update(const T& semimove, const double score, const double weight) {
        if constexpr (!ONLY_STATES) {
            insert_or_update(semimove.mr, score, weight);
        }
        auto index = (semimove.cell - 1) * reasoner::AUTOMATON_SIZE + semimove.state;
        arr2[index].weight += weight;
        arr2[index].sum += score;
        return 0;
    }
    int insert_or_update(const reasoner::move&, const double, const double);

    template <typename T>
    double get_score_or_default_value(const T& semimove) {
        const auto index = (semimove.cell - 1) * reasoner::AUTOMATON_SIZE + semimove.state;
        if constexpr (ONLY_STATES) {
            return arr2[index].sum / arr2[index].weight;
        }
        double total_sum = 0;
        [[maybe_unused]] double weight_sum = 0;
        for (const auto& action : semimove.mr) {
            auto action_index = (action.cell - 1) * reasoner::NUMBER_OF_MODIFIERS + reasoner::action_to_modifier_index(action.index);
            if (arr1[action_index].weight == 0) {
                return EXPECTED_MAX_SCORE;
            }
            total_sum += arr1[action_index].sum / arr1[action_index].weight;
        }
        if (arr2[index].weight == 0) {
            return EXPECTED_MAX_SCORE;
        }
        total_sum += arr2[index].sum / arr2[index].weight;
        return total_sum / (1 + semimove.mr.size());
    }
    double get_score_or_default_value(const reasoner::move&);
    void apply_decay_factor();
};

#endif
