#ifndef NODEBASE
#define NODEBASE

#include "reasoner.hpp"
#include "types.hpp"


struct node_base {
    std::pair<uint, uint> children_range = {1, 0};
    node_base(void) = default;
    node_base(const uint, const uint);
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

struct child_base {
    uint index = 0;
    uint sim_count = 0;
    uint total_score = 0;
    #if RAVE > 0
    amaf_score amaf;
    #endif
};

#endif
