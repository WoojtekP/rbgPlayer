#ifndef SEMISPLITNODE
#define SEMISPLITNODE

#include "node_base.hpp"
#include "reasoner.hpp"
#include "types.hpp"


enum node_status : char {
    terminal,
    nonterminal,
    unknown,
};

struct semisplit_node : node_base {
    const bool is_nodal;
    node_status status;
    semisplit_node(void) = delete;
    semisplit_node(const semisplit_node&) = default;
    semisplit_node(semisplit_node&&) = default;
    semisplit_node& operator=(const semisplit_node&) = default;
    semisplit_node& operator=(semisplit_node&&) = default;
    ~semisplit_node(void) = default;
    semisplit_node(const uint, const uint, const bool, const node_status);
    semisplit_node(const bool, const node_status);
    bool is_terminal() const;
};

#endif
