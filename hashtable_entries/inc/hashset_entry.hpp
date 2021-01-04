#ifndef HASHSET_ENTRY
#define HASHSET_ENTRY

#include "reasoner.hpp"


struct HashsetEntry {
    reasoner::move move;
    uint next = 0;
    bool inserted = false;
    HashsetEntry() = default;
    HashsetEntry(const reasoner::move mv, const int, const bool ins = false)
        : move(mv)
        , next(0)
        , inserted(ins) {}
    inline bool equals(const reasoner::move& mv, const int) const {
        return move == mv;
    }
    int get_context() const {
        return 0;
    }
};

#endif
