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
    semisplit_node(void)=delete;
    semisplit_node(const semisplit_node&)=default;
    semisplit_node(semisplit_node&&)=default;
    semisplit_node& operator=(const semisplit_node&)=default;
    semisplit_node& operator=(semisplit_node&&)=default;
    ~semisplit_node(void)=default;
    semisplit_node(const uint, const uint, const bool, const node_status);
    semisplit_node(const bool, const node_status);
    bool is_terminal() const;
};

struct semisplit_child : child_base {
    reasoner::semimove semimove;
    semisplit_child(void)=delete;
    semisplit_child(const semisplit_child&)=default;
    semisplit_child(semisplit_child&&)=default;
    semisplit_child& operator=(const semisplit_child&)=default;
    semisplit_child& operator=(semisplit_child&&)=default;
    ~semisplit_child(void)=default;
    semisplit_child(const reasoner::semimove&);
    const reasoner::move_representation& get_actions() const;
    const reasoner::semimove& get_edge() const;
};

#endif
