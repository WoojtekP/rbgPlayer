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

struct score {
    double sum = 0;
    double weight = 0;
    score() = default;
    score(const double _sum, const double _weight)
        : sum(_sum)
        , weight(_weight) {}
    inline double get_score() const {
        return sum / weight;
    }
};

#endif
