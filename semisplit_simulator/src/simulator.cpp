#include "simulator.hpp"
#include "reasoner.hpp"
#include "constants.hpp"
#include "random_generator.hpp"
#include "move_chooser.hpp"


namespace {
    std::vector<reasoner::semimove> legal_semimoves[MAX_SEMIDEPTH];

    bool apply_random_move_exhaustive(reasoner::game_state& state,
                                      MoveChooser<move_type>& move_chooser,
                                      reasoner::resettable_bitarray_stack& cache,
                                      uint semidepth) {
        std::vector<reasoner::semimove>& semimoves = legal_semimoves[semidepth];
        state.get_all_semimoves(cache, semimoves, SEMILENGTH);
        semidepth++;
        while (not semimoves.empty()) {
            uint chosen_semimove = move_chooser.get_random_move(semimoves, state.get_current_player());
            reasoner::revert_information ri = state.apply_semimove_with_revert(semimoves[chosen_semimove]);
            semimoves[chosen_semimove] = semimoves.back();
            semimoves.pop_back();
            if (state.is_nodal())
                return true;
            if (apply_random_move_exhaustive(state, move_chooser, cache, semidepth))
                return true;
            state.revert(ri);
            move_chooser.revert_move();
        }
        return false;
    }

    bool apply_random_move_exhaustive(reasoner::game_state& state,
                                      reasoner::resettable_bitarray_stack& cache,
                                      uint semidepth) {
        static RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
        std::vector<reasoner::semimove>& semimoves = legal_semimoves[semidepth];
        state.get_all_semimoves(cache, semimoves, SEMILENGTH);
        semidepth++;
        while (not semimoves.empty()) {
            uint chosen_semimove = rand_gen.uniform_choice(semimoves.size());
            reasoner::revert_information ri = state.apply_semimove_with_revert(semimoves[chosen_semimove]);
            semimoves[chosen_semimove] = semimoves.back();
            semimoves.pop_back();
            if (state.is_nodal())
                return true;
            if (apply_random_move_exhaustive(state, cache, semidepth))
                return true;
            state.revert(ri);
        }
        return false;
    }
}

bool has_nodal_successor(reasoner::game_state& state,
                         reasoner::resettable_bitarray_stack& cache) {
    while (state.get_current_player() == KEEPER) {
        if (not state.apply_any_move(cache)) {
            return false;
        }
    }
    return apply_random_move_exhaustive(state, cache, 0);
}

bool play(reasoner::game_state& state,
          MoveChooser<move_type>& move_chooser,
          reasoner::resettable_bitarray_stack& cache,
          simulation_result& results) {
    while(true) {
        if (not apply_random_move_exhaustive(state, move_chooser, cache, 0)) {
            break;
        }
        while(state.get_current_player() == KEEPER) {
            auto any_move = state.apply_any_move(cache);
            if (not any_move) {
                break;
            }
        }
    }
    for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
    return state.is_nodal();
}