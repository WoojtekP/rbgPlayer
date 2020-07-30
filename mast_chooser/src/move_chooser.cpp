#include "move_chooser.hpp"


template<>
const reasoner::move_representation MoveChooser<reasoner::move>::extract_actions(const reasoner::move& move) {
    return move.mr;
}
