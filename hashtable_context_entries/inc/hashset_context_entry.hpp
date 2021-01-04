#ifndef HASHSET_CONTEXT_ENTRY
#define HASHSET_CONTEXT_ENTRY

#include "reasoner.hpp"


struct HashsetContextEntry {
    reasoner::action_representation move;
    int context = 0;
    uint next = 0;
    bool inserted = false;
    HashsetContextEntry() = default;
    HashsetContextEntry(const reasoner::action_representation mv, const int ctx, const bool ins = false)
        : move(mv)
        , context(ctx)
        , next(0)
        , inserted(ins) {}
    inline bool equals(const reasoner::action_representation mv, const int ctx) const {
        return context == ctx && move == mv;
    }
    int get_context() const {
        return context;
    }
};

#endif
