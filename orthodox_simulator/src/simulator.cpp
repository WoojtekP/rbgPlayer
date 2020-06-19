#include <random>

#include "simulator.hpp"
#include "reasoner.hpp"
#include "constants.hpp"
#if SIM_MOVE_JOIN == 1
#include "moves_generator.hpp"
#endif

void play(reasoner::game_state& state,
          reasoner::resettable_bitarray_stack& cache,
          std::mt19937& random_numbers_generator,
          simulation_result& results) {
    static std::vector<reasoner::move> move_list;
    while (true) {
        #if SIM_MOVE_JOIN == 1
        get_all_joint_moves(state, cache, move_list);
        #else
        state.get_all_moves(cache, move_list);
        #endif
        if(move_list.empty()) {
            break;
        }
        else {
            std::uniform_int_distribution<uint> dist(0, move_list.size() - 1);
            uint chosen_move = dist(random_numbers_generator);
            state.apply_move(move_list[chosen_move]);
        }
        while (state.get_current_player() == KEEPER) {
            if (not state.apply_any_move(cache)) {
                break;
            }
        }
    }
    for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
}
