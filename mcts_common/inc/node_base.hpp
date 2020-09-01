#ifndef NODEBASE
#define NODEBASE

#include "reasoner.hpp"
#include "types.hpp"


struct NodeBase {
    std::pair<uint, uint> children_range = {1, 0};
    NodeBase(void)=default;
    NodeBase(const uint, const uint);
    bool is_expanded() const;
};

struct amaf_score {
    uint64_t score = 0;
    uint count = 0;
    #if RAVE == 3
    uint count_base = 0;
    uint64_t score_base = 0;
    #endif
};

struct ChildBase {
    uint index = 0;
    uint sim_count = 0;
    uint total_score = 0;
    #if RAVE > 0
    amaf_score amaf;
    #endif
};

#endif
