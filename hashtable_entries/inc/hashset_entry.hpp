#ifndef HASHSET_ENTRY
#define HASHSET_ENTRY

#include "reasoner.hpp"


struct HashsetEntry {
    reasoner::move move;
    uint next = 0;
    HashsetEntry() = default;
    HashsetEntry(const reasoner::move mv, const int)
        : move(mv)
        , next(0) {}
    inline bool equals(const reasoner::move& mv, const int = 0) const {
        return move == mv;
    }
    int get_context() const {
        return 0;
    }
};

#endif
