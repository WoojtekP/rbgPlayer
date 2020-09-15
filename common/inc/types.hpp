#ifndef TYPES
#define TYPES

#include <array>
#include <tuple>
#include <vector>

#include "reasoner.hpp"


typedef unsigned int uint;
typedef double priority;
typedef std::array<int, reasoner::NUMBER_OF_PLAYERS - 1> simulation_result;

enum game_status_indication {
    own_turn,
    opponent_turn,
    end_game
};

#endif
