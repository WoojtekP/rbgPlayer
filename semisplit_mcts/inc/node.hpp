#ifndef NODE
#define NODE

#include "node_base.hpp"
#include "reasoner.hpp"
#include "types.hpp"


struct Node : NodeBase {
    const bool is_nodal;
    const bool has_nodal_succ;
    Node(void)=delete;
    Node(const Node&)=default;
    Node(Node&&)=default;
    Node& operator=(const Node&)=default;
    Node& operator=(Node&&)=default;
    ~Node(void)=default;
    Node(const uint, const uint, const bool, const bool);
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
