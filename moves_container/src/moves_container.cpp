#include "moves_container.hpp"


void MovesContainer::apply_decay_factor() {
    moves.apply_decay_factor();
}

int MovesContainer::get_context(const reasoner::move_representation& mr, const int context) {
    return moves.get_context(mr, context);
}
