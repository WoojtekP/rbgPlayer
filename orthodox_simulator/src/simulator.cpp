#include <random>

#include "game_state.hpp"
#include "simulator.hpp"
#include "reasoner.hpp"
#include "random_generator.hpp"
#include "constants.hpp"
#include "move_chooser.hpp"

#include <iostream>

uint play(GameState& state,
          MoveChooser<simulation_move_type>& move_chooser,
          reasoner::resettable_bitarray_stack& cache,
          simulation_result& results) {
    static std::vector<reasoner::move> moves;
    uint state_count = 0;
    while (true) {
        state.get_all_moves(cache, moves);
        if (moves.empty()) {
            break;
        }
        uint chosen_move = move_chooser.get_random_move(moves, state.get_current_player());
        #if RAVE > 0
        move_chooser.save_move(moves[chosen_move], state.get_current_player());
        #elif MAST > 0
        if constexpr (!TREE_ONLY) {
            move_chooser.save_move(moves[chosen_move], state.get_current_player());
        }
        #endif
        state.apply(moves[chosen_move]);
        ++state_count;
        while (state.get_current_player() == KEEPER) {
            if (!state.apply_any_move(cache)) {
                break;
            }
        }
    }
    for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
    return state_count;
}
