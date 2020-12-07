#ifndef HASHMAP_ENTRY
#define HASHMAP_ENTRY

#include "reasoner.hpp"

struct score {
    double sum = 0;
    double weight = 0;
    score() = default;
    score(const double _sum, const double _weight) : sum(_sum), weight(_weight) {}
    inline double get_score() const {
        return sum / weight;
    }
};

struct HashmapEntry {
    reasoner::move mv;
    uint next = 0;
    score total_score;
    HashmapEntry() = default;
    HashmapEntry(const reasoner::move _mv, double _sum, double _weight)
        : mv(_mv)
        , next(0)
        , total_score(_sum, _weight) {}
};

#endif
