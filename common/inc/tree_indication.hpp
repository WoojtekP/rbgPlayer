#ifndef TREE_INDICATION
#define TREE_INDICATION

#include <variant>

#include "reasoner.hpp"


struct move_request {};
struct simulation_request {};
struct reset_tree {};

struct tree_indication {
    std::variant<reasoner::move, move_request, simulation_request, reset_tree> content;
};

#endif
