#ifndef GAMESTATE
#define GAMESTATE

#include "config.hpp"
#include "reasoner.hpp"

#if REASONING_OVERHEAD == 1

class GameState final : public reasoner::game_state {
public:
    #if defined(GETTER_S)
    void get_all_semimoves(reasoner::resettable_bitarray_stack& cache, std::vector<reasoner::move>& semimoves) {
        reasoner::game_state::get_all_semimoves(cache, semimoves);
    }
    #elif defined(GETTER_A)
    void get_all_semimoves(reasoner::resettable_bitarray_stack& cache, std::vector<reasoner::action_representation>& actions) {
        reasoner::game_state::get_all_actions(cache, actions);
    }
    #endif
};

#else

#include <array>


class GameState final : public reasoner::game_state {
private:
    std::array<reasoner::game_state, REASONING_OVERHEAD - 1> states;

public:
    bool apply_any_move(reasoner::resettable_bitarray_stack& cache) {
        for (auto& state : states) {
            state.apply_any_move(cache);
        }
        return reasoner::game_state::apply_any_move(cache);
    }
    void apply(const reasoner::move& move) {
        for (auto& state : states) {
            state.apply(move);
        }
        reasoner::game_state::apply(move);
    }
    void apply(const reasoner::action_representation action) {
        for (auto& state : states) {
            state.apply(action);
        }
        reasoner::game_state::apply(action);
    }
    reasoner::move_reverter apply_with_revert(const reasoner::move& move) {
        for (auto& state : states) {
            state.apply_with_revert(move);
        }
        return reasoner::game_state::apply_with_revert(move);
    }
    reasoner::action_reverter apply_with_revert(const reasoner::action_representation action) {
        for (auto& state : states) {
            state.apply_with_revert(action);
        }
        return reasoner::game_state::apply_with_revert(action);
    }
    void revert(const reasoner::move_reverter& moverev) {
        for (auto& state : states) {
            state.revert(moverev);
        }
        reasoner::game_state::revert(moverev);
    }
    void revert(const reasoner::action_reverter& actionrev) {
        for (auto& state : states) {
            state.revert(actionrev);
        }
        reasoner::game_state::revert(actionrev);
    }
    #if defined(GETTER_M)
    void get_all_moves(reasoner::resettable_bitarray_stack& cache, std::vector<reasoner::move>& moves) {
        for (auto& state : states) {
            state.get_all_moves(cache, moves);
        }
        reasoner::game_state::get_all_moves(cache, moves);
    }
    #endif
    #if defined(GETTER_S)
    void get_all_semimoves(reasoner::resettable_bitarray_stack& cache, std::vector<reasoner::move>& semimoves) {
        for (auto& state : states) {
            state.get_all_semimoves(cache, semimoves);
        }
        reasoner::game_state::get_all_semimoves(cache, semimoves);
    }
    #elif defined(GETTER_A)
    void get_all_actions(reasoner::resettable_bitarray_stack& cache, std::vector<reasoner::action_representation>& actions) {
        for (auto& state : states) {
            state.get_all_actions(cache, actions);
        }
        reasoner::game_state::get_all_actions(cache, actions);
    }
    #endif
public:
    GameState(void) = default;
    GameState(const GameState&) = default;
    GameState(GameState&&) = default;
    GameState& operator=(const GameState&) = default;
    GameState& operator=(GameState&&) = default;
    ~GameState(void) = default;
};

#endif
#endif
