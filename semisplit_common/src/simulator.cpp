#include"simulator.hpp"
#include"reasoner.hpp"
#include"simulation_result.hpp"
#include"constants.hpp"

namespace{
bool handle_keeper_move(reasoner::game_state& state, reasoner::resettable_bitarray_stack& cache){
    return state.apply_any_move(cache);
}

reasoner::revert_information apply_random_semimove_from_given(reasoner::game_state& state, std::vector<reasoner::semimove>& semimoves, std::mt19937& mt){
    std::uniform_int_distribution<uint> distribution(0,semimoves.size()-1);
    uint chosen_semimove = distribution(mt);
    reasoner::revert_information ri = state.apply_semimove_with_revert(semimoves[chosen_semimove]);
    semimoves[chosen_semimove] = semimoves.back();
    semimoves.pop_back();
    return ri;
}

void fill_semimoves_table(reasoner::game_state& state,
                          uint semidepth,
                          moves_container& legal_semimoves,
                          reasoner::resettable_bitarray_stack& cache,
                          uint semimoves_length){
    while(semidepth >= legal_semimoves.size())
        legal_semimoves.emplace_back();
    std::vector<reasoner::semimove>& semimoves = legal_semimoves[semidepth];
    state.get_all_semimoves(cache, semimoves, semimoves_length);
}

bool apply_random_move_charge(reasoner::game_state &state,
                              uint semidepth,
                              moves_container& legal_semimoves,
                              reasoner::resettable_bitarray_stack& cache,
                              std::mt19937& mt,
                              uint semimoves_length){
    fill_semimoves_table(state, semidepth, legal_semimoves, cache, semimoves_length);
    if(legal_semimoves[semidepth].empty())
        return false;
    auto ri = apply_random_semimove_from_given(state, legal_semimoves[semidepth], mt);
    if(state.is_nodal())
        return true;
    if(apply_random_move_charge(state, semidepth+1, legal_semimoves, cache, mt, semimoves_length))
        return true;
    state.revert(ri);
    return false;
}

bool apply_random_charge(reasoner::game_state& state,
                         moves_container& legal_semimoves,
                         reasoner::resettable_bitarray_stack& cache,
                         std::mt19937& mt,
                         uint semimoves_length){
    for (uint i=0; i<CHARGES; ++i)
        if(apply_random_move_charge(state, 0, legal_semimoves, cache, mt, semimoves_length))
            return true;
    return false;
}

bool apply_random_move_exhaustive(reasoner::game_state &state,
                                  uint semidepth,
                                  moves_container& legal_semimoves,
                                  reasoner::resettable_bitarray_stack& cache,
                                  std::mt19937& mt,
                                  uint semimoves_length){
    fill_semimoves_table(state, semidepth, legal_semimoves, cache, semimoves_length);
    while(not legal_semimoves[semidepth].empty()){
        auto ri = apply_random_semimove_from_given(state, legal_semimoves[semidepth], mt);
        if(state.is_nodal())
            return true;
        if(apply_random_move_exhaustive(state, semidepth+1, legal_semimoves, cache, mt, semimoves_length))
            return true;
        state.revert(ri);
    }
    return false;
}

bool apply_random_exhaustive(reasoner::game_state &state,
                             moves_container& legal_semimoves,
                             reasoner::resettable_bitarray_stack& cache,
                             std::mt19937& mt,
                             uint semimoves_length){
    return apply_random_move_exhaustive(state, 0, legal_semimoves, cache, mt, semimoves_length);
}

bool handle_player_move(reasoner::game_state &state,
                        reasoner::resettable_bitarray_stack& cache,
                        moves_container& legal_semimoves,
                        std::mt19937& mt,
                        uint semimoves_length){
    return apply_random_charge(state, legal_semimoves, cache, mt, semimoves_length)
        or apply_random_exhaustive(state, legal_semimoves, cache, mt, semimoves_length);
}

bool handle_move(reasoner::game_state& state,
                 reasoner::resettable_bitarray_stack& cache,
                 moves_container& legal_semimoves,
                 std::mt19937& mt,
                 uint semimoves_length){
    if(state.get_current_player() == KEEPER)
        return handle_keeper_move(state, cache);
    else
        return handle_player_move(state, cache, legal_semimoves, mt, semimoves_length);
}
}

simulation_result perform_simulation(reasoner::game_state& state,
                                     reasoner::resettable_bitarray_stack& cache,
                                     moves_container& legal_semimoves,
                                     std::mt19937& mt,
                                     uint semimoves_length){
    while(handle_move(state, cache, legal_semimoves, mt, semimoves_length));
    return simulation_result(state);
}

