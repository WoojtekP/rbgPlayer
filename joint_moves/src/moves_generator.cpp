#include <algorithm>

#include "moves_generator.hpp"
#include "reasoner.hpp"
#include "constants.hpp"


namespace {
    std::vector<reasoner::semimove> legal_semimoves[100];

    void dfs(reasoner::game_state& state,
             reasoner::resettable_bitarray_stack& cache,
             std::vector<reasoner::move>& move_list,
             int indices[],
             const int semidepth) {
        const int player_id = state.get_current_player();
        state.get_all_semimoves(cache, legal_semimoves[semidepth], 100000);
        int i = 0;
        for (const auto& semimove: legal_semimoves[semidepth]) {
            const auto ri = state.apply_semimove_with_revert(semimove);
            indices[semidepth] = i;
            if (state.get_current_player() != player_id) {
                auto& move = move_list.emplace_back();
                for (int j = 0; j <= semidepth; ++j) {
                    const auto& actions = legal_semimoves[j][indices[j]].get_actions();
                    move.mr.insert(move.mr.end(), actions.begin(), actions.end());
                }
            }
            else {
                dfs(state, cache, move_list, indices, semidepth + 1);
            }
            state.revert(ri);
            ++i;
        }
    }
}

void get_all_joint_moves(reasoner::game_state& state,
                         reasoner::resettable_bitarray_stack& cache,
                         std::vector<reasoner::move>& move_list) {
    static int indices[100];
    move_list.clear();
    dfs(state, cache, move_list, indices, 0);
}