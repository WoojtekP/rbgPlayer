#include "moves_container.hpp"


void MovesContainer::apply_decay_factor() {
    if constexpr (MAST_EQUIVALENCE_PARAMETER > 0) {
        arr.apply_decay_factor();
    }
    moves.apply_decay_factor();
}

int MovesContainer::get_context(const reasoner::move&, const int) {
    return 0;
}
