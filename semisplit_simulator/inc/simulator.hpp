#ifndef SIMULATOR
#define SIMULATOR

#include <vector>
#include <random>

namespace reasoner {
    class game_state;
    class resettable_bitarray_stack;
}

typedef std::vector<uint> simulation_result;

bool has_nodal_successor(reasoner::game_state& , reasoner::resettable_bitarray_stack&);
bool play(reasoner::game_state&, reasoner::resettable_bitarray_stack&, simulation_result&);

#endif
