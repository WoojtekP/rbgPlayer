#ifndef GAMESTATE
#define GAMESTATE

#include "config.hpp"
#include "reasoner.hpp"

#if REASONING_OVERHEAD == 1

typedef reasoner::game_state GameState;

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
    reasoner::revert_information apply_semimove_with_revert(const reasoner::semimove& m) {
        for (auto& state : states) {
            state.apply_semimove_with_revert(m);
        }
        return reasoner::game_state::apply_semimove_with_revert(m);
    }
    void apply_semimove(const reasoner::semimove& m) {
        for (auto& state : states) {
            state.apply_semimove(m);
        }
        reasoner::game_state::apply_semimove(m);
    }
    void get_all_semimoves(reasoner::resettable_bitarray_stack& cache, std::vector<reasoner::semimove>& moves, unsigned int move_length_limit) {
        for (auto& state : states) {
            state.get_all_semimoves(cache, moves, move_length_limit);
        }
        reasoner::game_state::get_all_semimoves(cache, moves, move_length_limit);
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
