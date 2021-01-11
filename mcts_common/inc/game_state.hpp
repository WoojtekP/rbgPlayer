#ifndef GAMESTATE
#define GAMESTATE

#include "config.hpp"
#include "reasoner.hpp"

#if REASONING_OVERHEAD == 1

class GameState final : public reasoner::game_state {
public:
    void apply_move(const reasoner::move& move) {
        reasoner::game_state::apply_move(move);
    }
    #if defined(SEMISPLIT_TREE) || defined(SEMISPLIT_SIMULATOR)
    void apply_move(const reasoner::action_representation action) {
        reasoner::game_state::apply_action(action);
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
    void apply_move(const reasoner::move& m) {
        for (auto& state : states) {
            state.apply_move(m);
        }
        reasoner::game_state::apply_move(m);
    }
    void get_all_moves(reasoner::resettable_bitarray_stack& cache, std::vector<reasoner::move>& moves) {
        for (auto& state : states) {
            state.get_all_moves(cache, moves);
        }
        reasoner::game_state::get_all_moves(cache, moves);
    }
#if defined(SEMISPLIT_TREE) || defined(SEMISPLIT_SIMULATOR)
    reasoner::revert_information apply_action_with_revert(const reasoner::action_representation action) {
        for (auto& state : states) {
            state.apply_action_with_revert(action);
        }
        return reasoner::game_state::apply_action_with_revert(action);
    }
    void apply_move(const reasoner::action_representation action) {
        for (auto& state : states) {
            state.apply_action(action);
        }
        reasoner::game_state::apply_action(action);
    }
    void apply_action(const reasoner::action_representation action) {
        for (auto& state : states) {
            state.apply_action(action);
        }
        reasoner::game_state::apply_action(action);
    }
    void fast_apply_action(const reasoner::action_representation action) {
        for (auto& state : states) {
            state.fast_apply_action(action);
        }
        reasoner::game_state::fast_apply_action(action);
    }
    void get_all_actions(reasoner::resettable_bitarray_stack& cache, std::vector<reasoner::action_representation>& actions) {
        for (auto& state : states) {
            state.get_all_actions(cache, actions);
        }
        reasoner::game_state::get_all_actions(cache, actions);
    }
    void revert(const reasoner::revert_information& ri) {
        for (auto& state : states) {
            state.revert(ri);
        }
        reasoner::game_state::revert(ri);
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
