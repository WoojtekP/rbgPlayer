#include <random>

#include "simulator.hpp"
#include "reasoner.hpp"
#include "random_generator.hpp"
#include "constants.hpp"

void play(reasoner::game_state& state,
          reasoner::resettable_bitarray_stack& cache,
          simulation_result& results) {
    static std::vector<reasoner::move> move_list;
    static RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
    while (true) {
        state.get_all_moves(cache, move_list);
        if(move_list.empty()) {
            break;
        }
        else {
            uint chosen_move = rand_gen.uniform_choice(move_list.size());
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
