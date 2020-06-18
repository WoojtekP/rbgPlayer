#ifndef MOVESGENERATOR
#define MOVESGENERATOR

#include <vector>

namespace reasoner {
    class game_state;
    class move;
    class resettable_bitarray_stack;
}

void get_all_joint_moves(reasoner::game_state&, reasoner::resettable_bitarray_stack&, std::vector<reasoner::move>&);

#endif
