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

struct SemisplitNode : NodeBase {
    const bool is_nodal;
    node_status status;
    SemisplitNode(void)=delete;
    SemisplitNode(const SemisplitNode&)=default;
    SemisplitNode(SemisplitNode&&)=default;
    SemisplitNode& operator=(const SemisplitNode&)=default;
    SemisplitNode& operator=(SemisplitNode&&)=default;
    ~SemisplitNode(void)=default;
    SemisplitNode(const uint, const uint, const bool, const node_status);
    SemisplitNode(const bool, const node_status);
    bool is_terminal() const;
};

struct SemisplitChild : ChildBase {
    reasoner::semimove semimove;
    SemisplitChild(void)=delete;
    SemisplitChild(const SemisplitChild&)=default;
    SemisplitChild(SemisplitChild&&)=default;
    SemisplitChild& operator=(const SemisplitChild&)=default;
    SemisplitChild& operator=(SemisplitChild&&)=default;
    ~SemisplitChild(void)=default;
    SemisplitChild(const reasoner::semimove&);
    const reasoner::move_representation& get_actions() const;
    const reasoner::semimove& get_edge() const;
};

#endif
