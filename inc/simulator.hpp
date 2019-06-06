#ifndef SIMULATOR
#define SIMULATOR

#include<random>

namespace reasoner{
    class game_state;
    class resettable_bitarray_stack;
}

class simulation_result;

simulation_result perform_simulation(reasoner::game_state& state,
                                     reasoner::resettable_bitarray_stack& cache,
                                     std::mt19937& mt);

#endif