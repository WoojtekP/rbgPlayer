#ifndef NODE
#define NODE

#include "node_base.hpp"
#include "reasoner.hpp"
#include "types.hpp"


struct Node : NodeBase {
    Node(void)=default;
    Node(const uint, const uint);
    bool is_terminal() const;
};

struct Child : ChildBase {
    reasoner::move move;
    Child(void)=delete;
    Child(const Child&)=default;
    Child(Child&&)=default;
    Child& operator=(const Child&)=default;
    Child& operator=(Child&&)=default;
    ~Child(void)=default;
    Child(const reasoner::move& move);
    const reasoner::move_representation& get_actions() const;
};

#endif
