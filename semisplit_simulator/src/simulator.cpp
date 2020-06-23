#include <random>

#include "simulator.hpp"
#include "reasoner.hpp"
#include "constants.hpp"


namespace {
    std::vector<reasoner::semimove> legal_semimoves[MAX_SEMIDEPTH];
    
    reasoner::revert_information apply_random_semimove_from_given(reasoner::game_state &state,
                                                                std::mt19937& random_numbers_generator,
                                                                std::vector<reasoner::semimove> &semimoves) {
        std::uniform_int_distribution<uint> dist(0, semimoves.size() - 1);
        uint chosen_semimove = dist(random_numbers_generator);
        reasoner::revert_information ri = state.apply_semimove_with_revert(semimoves[chosen_semimove]);
        semimoves[chosen_semimove] = semimoves.back();
        semimoves.pop_back();
        return ri;
    }

    std::vector<reasoner::semimove>& fill_semimoves_table(reasoner::game_state &state,
                                                          reasoner::resettable_bitarray_stack& cache,
                                                          uint semidepth){
        std::vector<reasoner::semimove>& semimoves = legal_semimoves[semidepth];
        state.get_all_semimoves(cache, semimoves, SEMILENGTH);
        return semimoves;
    }

    bool apply_random_move_exhaustive(reasoner::game_state& state,
                                      reasoner::resettable_bitarray_stack& cache,
                                      std::mt19937& random_numbers_generator,
                                      uint semidepth){
        std::vector<reasoner::semimove>& semimoves = fill_semimoves_table(state, cache, semidepth);
        semidepth++;
        while(not semimoves.empty()){
            auto ri = apply_random_semimove_from_given(state, random_numbers_generator, semimoves);
            if(state.is_nodal())
                return true;
            if(apply_random_move_exhaustive(state, cache, random_numbers_generator, semidepth))
                return true;
            state.revert(ri);
        }
        return false;
    }
}

bool has_nodal_successor(reasoner::game_state& state,
                         reasoner::resettable_bitarray_stack& cache,
                         std::mt19937& mt) {
    while (state.get_current_player() == KEEPER) {
        if (not state.apply_any_move(cache)) {
            return false;
        }
    }
    return apply_random_move_exhaustive(state, cache, mt, 0);
}

bool play(reasoner::game_state& state,
          reasoner::resettable_bitarray_stack& cache,
          std::mt19937& random_numbers_generator,
          simulation_result& results) {
    while(true){
        if(not apply_random_move_exhaustive(state, cache, random_numbers_generator, 0)){
            break;
        }
        while(state.get_current_player() == KEEPER){
            auto any_move = state.apply_any_move(cache);
            if(not any_move){
                break;
            }
        }
    }
    for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
    return state.is_nodal();
}