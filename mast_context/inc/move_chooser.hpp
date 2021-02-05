#ifndef MOVECHOOSER
#define MOVECHOOSER

#include <utility>
#include <vector>

#include "constants.hpp"
#include "types.hpp"
#include "mast_chooser.hpp"


template <typename T>
class MoveChooser : public MastChooser<T> {
    using MastChooser<T>::path;
    using MastChooser<T>::moves;
    using MastChooser<T>::context;
private:
    std::vector<int> context_stack;
    bool end_of_context(const reasoner::action_representation action) const {
        return reasoner::is_switch(action.index);
    }
    bool end_of_context(const reasoner::move& move) const {
        assert(end_of_context(move.mr.back()));
        return true;
    }

public:
    MoveChooser(void) = default;
    MoveChooser(const MoveChooser&) = delete;
    MoveChooser(MoveChooser&&) = default;
    MoveChooser& operator=(const MoveChooser&) = delete;
    MoveChooser& operator=(MoveChooser&&) = default;
    ~MoveChooser(void) = default;

    void update_move(const reasoner::action_representation action, const simulation_result& results, const int player) {
        assert(player != KEEPER);
        auto new_context = moves[player - 1].insert_or_update(action, results[player - 1], context);
        if (action.index > 0) {
            context = reasoner::is_switch(action.index) ? 0 : new_context;
        }
    }

    void update_move(const reasoner::move& move, const simulation_result& results, const int player) {
        for (const auto action : move.mr) {
            update_move(action, results, player);
        }
    }

    void update_all_moves(const simulation_result& results) {
        for (const auto& [move, player] : path) {
            update_move(move, results, player);
        }
        assert(context == 0);
    }

    void switch_context(const reasoner::action_representation action, const int current_player) {
        if (end_of_context(action)) {
            reset_context();
        }
        else {
            context_stack.push_back(context);
            context = moves[current_player - 1].get_context(action, context);
        }
    }

    void switch_context(const reasoner::move& move, const int current_player) {
        for (auto it = move.mr.begin(); it < move.mr.end() - 1; ++it) {
            assert(!end_of_context(*it));
            context = moves[current_player - 1].get_context(*it, context);
        }
        switch_context(move.mr.back(), current_player);
    }

    void revert_context() {
        context = context_stack.back();
        context_stack.pop_back();
    }

    void reset_context() {
        context = 0;
        context_stack.clear();
    }

    void clear_path() {
        path.clear();
        reset_context();
    }
};

#endif
