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
        state.get_all_semimoves(cache, legal_semimoves[semidepth], SEMILENGTH);
        while (not legal_semimoves[semidepth].empty()) {
            const auto current_player = state.get_current_player();
            const auto chosen_semimove = move_chooser.get_random_move(legal_semimoves[semidepth], current_player);
            const auto ri = state.apply_semimove_with_revert(legal_semimoves[semidepth][chosen_semimove]);
            move_chooser.switch_context(legal_semimoves[semidepth][chosen_semimove], current_player);
            #if MAST > 0
            if constexpr (not TREE_ONLY)
                move_chooser.save_move(legal_semimoves[semidepth][chosen_semimove], current_player);
            #endif
            if (state.is_nodal())
                return true;
            if (apply_random_move_exhaustive(state, move_chooser, cache, semidepth+1))
                return true;
            state.revert(ri);
            move_chooser.revert_context();
            legal_semimoves[semidepth][chosen_semimove] = legal_semimoves[semidepth].back();
            legal_semimoves[semidepth].pop_back();
            #if MAST > 0
            if constexpr (not TREE_ONLY)
                move_chooser.revert_move();
            #endif
        }
        return false;
    }
}

uint play(reasoner::game_state& state,
          MoveChooser<move_type>& move_chooser,
          reasoner::resettable_bitarray_stack& cache,
          simulation_result& results) {
    uint state_count = 0;
    while(true) {
        if (not apply_random_move_exhaustive(state, move_chooser, cache, 0)) {
            break;
        }
        ++state_count;
        while(state.get_current_player() == KEEPER) {
            auto any_move = state.apply_any_move(cache);
            if (not any_move) {
                goto terminal;
            }
        }
    }
    terminal:
    for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
    return state_count;
}