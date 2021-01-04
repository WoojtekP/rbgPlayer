#ifndef HASHMAP_CONTEXT_ENTRY
#define HASHMAP_CONTEXT_ENTRY

#include "reasoner.hpp"
#include "types.hpp"


struct HashmapContextEntry {
    reasoner::action_representation move;
    int context = 0;
    uint next = 0;
    score total_score {0.0, 0.0};
    HashmapContextEntry() = default;
    HashmapContextEntry(const reasoner::action_representation mv, const int ctx, double sum = 0.0, double weight = 0.0)
        : move(mv)
        , context(ctx)
        , next(0)
        , total_score(sum, weight) {}
    inline bool equals(const reasoner::action_representation mv, const int ctx) const {
        return context == ctx && move == mv;
    }
    int get_context() const {
        return context;
    }
};

#endif
