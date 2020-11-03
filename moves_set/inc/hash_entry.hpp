#ifndef HASHSET_ENTRY
#define HASHSET_ENTRY

#include "reasoner.hpp"

struct Entry {
    reasoner::move mv;
    uint next = 0;
    Entry() = default;
    Entry(const reasoner::move _mv)
        : mv(_mv)
        , next(0) {}
};

#endif
