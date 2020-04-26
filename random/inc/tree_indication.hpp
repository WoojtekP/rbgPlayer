#ifndef TREE_INDICATION
#define TREE_INDICATION

#include<variant>

#include"reasoner.hpp"

struct move_request{};
struct game_end{};

struct tree_indication{
    std::variant<reasoner::move, move_request, game_end> content;
};

#endif
