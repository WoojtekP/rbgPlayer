#include "moves_container.hpp"


void MovesContainer::apply_decay_factor() {
    if constexpr (MAST_EQUIVALENCE_PARAMETER > 0) {
        arr.apply_decay_factor();
    }
    moves.apply_decay_factor();
}

int MovesContainer::get_context(const reasoner::action_representation action, const int context) {
    return moves.get_context(action, context);
}
