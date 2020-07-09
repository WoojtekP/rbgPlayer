#ifndef SIMULATOR
#define SIMULATOR

#include <vector>
#include <random>

#include "reasoner.hpp"
#include "move_chooser.hpp"

typedef reasoner::move move_type;
typedef std::vector<uint> simulation_result;

void play(reasoner::game_state&, MoveChooser<move_type>&, reasoner::resettable_bitarray_stack&, simulation_result&);

#endif
