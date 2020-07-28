#ifndef TYPES
#define TYPES

#include<tuple>
#include<vector>

typedef unsigned int uint;
typedef double priority;
typedef std::vector<uint> simulation_result;
enum game_status_indication{
    own_turn,
    opponent_turn,
    end_game
};

#endif
