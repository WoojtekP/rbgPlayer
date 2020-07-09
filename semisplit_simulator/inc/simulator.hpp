#ifndef SIMULATOR
#define SIMULATOR

#include <vector>
#include <random>

#include "reasoner.hpp"
#include "move_chooser.hpp"

typedef reasoner::semimove move_type;
typedef std::vector<uint> simulation_result;

bool has_nodal_successor(reasoner::game_state& , reasoner::resettable_bitarray_stack&);
bool play(reasoner::game_state&, MoveChooser<move_type>&, reasoner::resettable_bitarray_stack&, simulation_result&);

#endif
