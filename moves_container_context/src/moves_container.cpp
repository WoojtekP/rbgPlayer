#include "moves_container.hpp"


void moves_container::apply_decay_factor() {
    if constexpr (MAST_EQUIVALENCE_PARAMETER > 0) {
        arr.apply_decay_factor();
    }
    moves.apply_decay_factor();
}

int moves_container::get_context(const reasoner::move_representation& mr, const int context) {
    return moves.get_context(mr, context);
}
