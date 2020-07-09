// #include "semisplit_helpers.hpp"
// #include "reasoner.hpp"
// #include "constants.hpp"
// #include "random_generator.hpp"

// namespace {
//     std::vector<reasoner::semimove> legal_semimoves[MAX_SEMIDEPTH];

//     reasoner::revert_information apply_random_semimove_and_save(reasoner::game_state& state,
//                                                                 std::vector<reasoner::semimove>& semimoves,
//                                                                 std::vector<reasoner::semimove>& path) {
//         RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
//         uint chosen_semimove = rand_gen.uniform_choice(semimoves.size());
//         reasoner::revert_information ri = state.apply_semimove_with_revert(semimoves[chosen_semimove]);
//         path.push_back(semimoves[chosen_semimove]);
//         semimoves[chosen_semimove] = semimoves.back();
//         semimoves.pop_back();
//         return ri;
//     }

//     std::vector<reasoner::semimove>& fill_semimoves_table(reasoner::game_state &state,
//                                                         reasoner::resettable_bitarray_stack& cache,
//                                                         uint semidepth) {
//         std::vector<reasoner::semimove>& semimoves = legal_semimoves[semidepth];
//         state.get_all_semimoves(cache, semimoves, SEMILENGTH);
//         return semimoves;
//     }

//     bool apply_random_move_exhaustive_and_save_path(reasoner::game_state& state,
//                                                     reasoner::resettable_bitarray_stack& cache,
//                                                     std::vector<reasoner::semimove>& path,
//                                                     uint semidepth) {
//         std::vector<reasoner::semimove>& semimoves = fill_semimoves_table(state, cache, semidepth);
//         semidepth++;
//         while (not semimoves.empty()) {
//             auto ri = apply_random_semimove_and_save(state, semimoves, path);
//             if (state.is_nodal())
//                 return true;
//             if (apply_random_move_exhaustive_and_save_path(state, cache, path semidepth))
//                 return true;
//             state.revert(ri);
//             path.pop_back();
//         }
//         return false;
//     }
// }

// // bool has_nodal_successor(reasoner::game_state& state,
// //                          reasoner::resettable_bitarray_stack& cache) {
// //     while (state.get_current_player() == KEEPER) {
// //         if (not state.apply_any_move(cache)) {
// //             return false;
// //         }
// //     }
// //     return apply_random_move_exhaustive(state, cache, 0);
// // }

// bool reach_nodal_successor_and_save_path(reasoner::game_state& state,
//                                          reasoner::resettable_bitarray_stack& cache,
//                                          std::vector<reasoner::semimove>& path) {
//     while (state.get_current_player() == KEEPER) {
//         if (not state.apply_any_move(cache)) {
//             return false;
//         }
//     }
//     return apply_random_move_exhaustive_and_save_path(state, cache, path, 0);
// }
