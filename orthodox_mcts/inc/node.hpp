#ifndef NODE
#define NODE

#include "node_base.hpp"
#include "reasoner.hpp"
#include "types.hpp"


struct node : node_base {
    node(void) = default;
    node(const uint, const uint);
    bool is_terminal() const;
};

struct child : child_base {
    reasoner::move move;
    child(void) = delete;
    child(const child&) = default;
    child(child&&) = default;
    child& operator=(const child&) = default;
    child& operator=(child&&) = default;
    ~child(void) = default;
    child(const reasoner::move& move);
    const reasoner::move_representation& get_actions() const;
    const reasoner::move& get_edge() const;
};

#endif
