#ifndef HASHSET_ENTRY
#define HASHSET_ENTRY

#include "reasoner.hpp"


struct HashsetEntry {
    reasoner::move mv;
    uint next = 0;
    HashsetEntry() = default;
    HashsetEntry(const reasoner::move _mv)
        : mv(_mv)
        , next(0) {}
};

#endif
