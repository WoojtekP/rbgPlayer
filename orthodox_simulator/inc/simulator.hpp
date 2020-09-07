#ifndef SIMULATOR
#define SIMULATOR

#include <vector>
#include <random>

#include "reasoner.hpp"
#include "move_chooser.hpp"
#include "types.hpp"

typedef reasoner::move simulation_move_type;

uint play(reasoner::game_state&, MoveChooser<simulation_move_type>&, reasoner::resettable_bitarray_stack&, simulation_result&);

#endif
