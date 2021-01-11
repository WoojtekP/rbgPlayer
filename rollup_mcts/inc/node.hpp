#ifndef NODE
#define NODE

#include "node_base.hpp"
#include "semisplit_node.hpp"
#include "reasoner.hpp"
#include "types.hpp"


typedef semisplit_node node;

struct child : child_base {
    reasoner::move move;
    child(void) = delete;
    child(const child&) = default;
    child(child&&) = default;
    child& operator=(const child&) = default;
    child& operator=(child&&) = default;
    ~child(void) = default;
    child(const reasoner::action_representation);
    child(const reasoner::move&);
    const reasoner::action_representation get_action() const;
    const reasoner::move& get_edge() const;
};

#endif
