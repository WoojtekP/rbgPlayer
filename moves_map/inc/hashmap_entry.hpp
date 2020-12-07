#ifndef HASHMAP_ENTRY
#define HASHMAP_ENTRY

#include "reasoner.hpp"


struct HashmapEntry {
    reasoner::move mv;
    uint next = 0;
    double scores = 0.0;
    double weight = 0.0;
    HashmapEntry() = default;
    HashmapEntry(const reasoner::move _mv, double _scores, double _weight)
        : mv(_mv)
        , next(0)
        , scores(_scores)
        , weight(_weight) {}
};

#endif
