#ifndef NODEBASE
#define NODEBASE

#include "reasoner.hpp"
#include "types.hpp"


struct NodeBase {
    std::pair<uint, uint> children_range;
    uint sim_count = 0;
    NodeBase(const uint, const uint);
};

struct ChildBase {
    uint index = 0;
    uint sim_count = 0;
    uint total_score = 0;
    #if RAVE > 0
    uint amaf_score = 0;
    uint amaf_count = 0;
    #endif
};

#endif
