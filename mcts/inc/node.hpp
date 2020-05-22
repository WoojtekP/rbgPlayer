#ifndef NODE
#define NODE

#include <random>

#include "reasoner.hpp"
#include "types.hpp"

typedef std::vector<uint> simulation_result;

struct Node {
    std::pair<uint, uint> children_range;
    uint sim_count = 0;
    Node(void)=default;
    Node(const Node&)=default;
    Node(Node&&)=default;
    Node& operator=(const Node&)=default;
    Node& operator=(Node&&)=default;
    ~Node(void)=default;
    Node(const uint&, const uint&);
    bool is_terminal() const;

    static reasoner::resettable_bitarray_stack cache;
};

struct Child {
    uint index = 0;
    uint sim_count = 0;
    uint total_score = 0;
    reasoner::move move;
    Child(void) = default;
    Child(const reasoner::move& move);
    void update_stats(const uint& current_player, simulation_result& results);
};

#endif
