#ifndef NODE
#define NODE

#include "node_base.hpp"
#include "reasoner.hpp"
#include "types.hpp"

enum node_status : char {
    terminal,
    nonterminal,
    unknown,
};

struct Node : NodeBase {
    const bool is_nodal;
    node_status status;
    Node(void)=delete;
    Node(const Node&)=default;
    Node(Node&&)=default;
    Node& operator=(const Node&)=default;
    Node& operator=(Node&&)=default;
    ~Node(void)=default;
    Node(const uint, const uint, const bool, const node_status);
    bool is_terminal() const;
};

struct Child : ChildBase {
    reasoner::semimove semimove;
    Child(void)=delete;
    Child(const Child&)=default;
    Child(Child&&)=default;
    Child& operator=(const Child&)=default;
    Child& operator=(Child&&)=default;
    ~Child(void)=default;
    Child(const reasoner::semimove&);
    const reasoner::move_representation& get_actions() const;
};

#endif
