#include <random>

#include "simulator.hpp"
#include "reasoner.hpp"
#include "random_generator.hpp"
#include "constants.hpp"
#include "move_chooser.hpp"

#include <iostream>

uint play(reasoner::game_state& state,
          MoveChooser<simulation_move_type>& move_chooser,
          reasoner::resettable_bitarray_stack& cache,
          simulation_result& results) {
    static std::vector<reasoner::move> moves;
    uint state_count = 0;
    while (true) {
        #ifdef SEMISPLIT_TREE
        static std::vector<reasoner::semimove> semimoves;
        state.get_all_semimoves(cache, semimoves, 1000);
        moves.clear();
        for (const auto& semimove : semimoves) {
            moves.emplace_back(semimove.mr);
        }
        #else
            state.get_all_moves(cache, moves);
        #endif
        if(moves.empty()) {
            break;
        }
        uint chosen_move = move_chooser.get_random_move(moves, state.get_current_player());
        #if RAVE > 0
        move_chooser.save_move(moves[chosen_move], state.get_current_player());
        #elif MAST > 0
        if constexpr (not TREE_ONLY) {
            move_chooser.save_move(moves[chosen_move], state.get_current_player());
        }
        #endif
        state.apply_move(moves[chosen_move]);
        ++state_count;
        while (state.get_current_player() == KEEPER) {
            if (not state.apply_any_move(cache)) {
                break;
            }
        }
    }
    for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
    return state_count;
}
