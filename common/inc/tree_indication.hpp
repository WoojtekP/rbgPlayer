#ifndef TREE_INDICATION
#define TREE_INDICATION

#include"reasoner.hpp"

#include<variant>

struct move_request{};
struct reset_tree{};

struct tree_indication{
    std::variant<reasoner::move, move_request, reset_tree> content;
};

#endif
