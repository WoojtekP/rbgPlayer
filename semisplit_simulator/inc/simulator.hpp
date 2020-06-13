#ifndef SIMULATOR
#define SIMULATOR

#include <vector>
#include <random>

namespace reasoner {
    class game_state;
    class move;
    class resettable_bitarray_stack;
}

typedef std::vector<uint> simulation_result;

void play(reasoner::game_state&, reasoner::resettable_bitarray_stack&, std::mt19937&, simulation_result&);

#endif
