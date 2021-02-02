#include "game_state.hpp"
#include "simulator.hpp"
#include "reasoner.hpp"
#include "constants.hpp"
#include "random_generator.hpp"
#include "move_chooser.hpp"


namespace {
std::vector<simulation_move_type> legal_actions[MAX_SEMIDEPTH];

bool apply_random_move_exhaustive(GameState& state,
                                  MoveChooser<simulation_move_type>& move_chooser,
                                  reasoner::resettable_bitarray_stack& cache,
                                  uint semidepth) {
    state.get_all_semimoves(cache, legal_actions[semidepth]);
    while (!legal_actions[semidepth].empty()) {
        const auto current_player = state.get_current_player();
        const auto chosen_action = move_chooser.get_random_move(legal_actions[semidepth], current_player);
        const auto ri = state.apply_with_revert(legal_actions[semidepth][chosen_action]);
        move_chooser.switch_context(legal_actions[semidepth][chosen_action], current_player);
        #if RAVE > 0
        move_chooser.save_move(legal_actions[semidepth][chosen_action], current_player);
        #elif MAST > 0
        if constexpr (!TREE_ONLY) {
            move_chooser.save_move(legal_actions[semidepth][chosen_action], current_player);
        }
        #endif
        if (state.is_nodal())
            return true;
        if (apply_random_move_exhaustive(state, move_chooser, cache, semidepth+1))
            return true;
        state.revert(ri);
        move_chooser.revert_context();
        legal_actions[semidepth][chosen_action] = legal_actions[semidepth].back();
        legal_actions[semidepth].pop_back();
        #if RAVE > 0
        move_chooser.revert_move();
        #elif MAST > 0
        if constexpr (!TREE_ONLY) {
            move_chooser.revert_move();
        }
        #endif
    }
    return false;
}
}  // namespace

uint play(GameState& state,
          MoveChooser<simulation_move_type>& move_chooser,
          reasoner::resettable_bitarray_stack& cache,
          simulation_result& results) {
    uint state_count = 0;
    if (state.get_current_player() == KEEPER) {
        goto lb_terminal;
    }
    while (true) {
        if (!apply_random_move_exhaustive(state, move_chooser, cache, 0)) {
            break;
        }
        assert(move_chooser.get_context() == 0);
        ++state_count;
        while (state.get_current_player() == KEEPER) {
            if (!state.apply_any_move(cache)) {
                goto lb_terminal;
            }
        }
    }
    lb_terminal:
    for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
    return state_count;
}