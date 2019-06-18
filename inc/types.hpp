#ifndef TYPES
#define TYPES

#include<tuple>
#include<vector>

typedef unsigned int uint;
typedef std::vector<uint> node_address;
typedef double priority;
enum game_status_indication{
    own_turn,
    opponent_turn,
    end_game
};

#endif
