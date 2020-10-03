#ifndef SIMULATOR
#define SIMULATOR

#include <vector>
#include <random>

#include "game_state.hpp"
#include "reasoner.hpp"
#include "move_chooser.hpp"
#include "types.hpp"

typedef reasoner::semimove simulation_move_type;

uint play(GameState&, MoveChooser<simulation_move_type>&, reasoner::resettable_bitarray_stack&, simulation_result&);

#endif
