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
public:
    MoveChooser(void) = default;
    MoveChooser(const MoveChooser&) = delete;
    MoveChooser(MoveChooser&&) = default;
    MoveChooser& operator=(const MoveChooser&) = delete;
    MoveChooser& operator=(MoveChooser&&) = default;
    ~MoveChooser(void) = default;

    template <typename M>
    void update_move(const M& move, const simulation_result& results, const int player) {
        assert(player != KEEPER);
        moves[player - 1].insert_or_update(move, results[player - 1], context);
    }

    void update_all_moves(const simulation_result& results) {
        for (const auto& [move, player] : path) {
            update_move(move, results, player);
        }
    }

    template <typename M>
    void switch_context(const M&, const int) {}

    void revert_context() {}

    void reset_context() {}

    void clear_path() {
        path.clear();
    }
};

#endif
