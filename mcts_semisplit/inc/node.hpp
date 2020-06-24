#ifndef NODE
#define NODE

#include <random>

#include "reasoner.hpp"
#include "types.hpp"

typedef std::vector<uint> simulation_result;

struct Node {
    std::pair<uint, uint> children_range;
    uint sim_count = 0;
    const bool is_nodal;
    const bool has_nodal_succ;
    Node(void)=default;
    Node(const Node&)=default;
    Node(Node&&)=default;
    Node& operator=(const Node&)=default;
    Node& operator=(Node&&)=default;
    ~Node(void)=default;
    Node(const uint, const uint, const bool, const bool);
    bool is_terminal() const;
};

struct Child {
    // move -> semimove
    reasoner::semimove semimove;
    uint index = 0;
    uint sim_count = 0;
    uint total_score = 0;
    bool is_nodal;
    Child(void)=delete;
    Child(const Child&)=default;
    Child(Child&&)=default;
    Child& operator=(const Child&)=default;
    Child& operator=(Child&&)=default;
    ~Child(void)=default;
    Child(const reasoner::semimove&, const bool);
};


#endif
