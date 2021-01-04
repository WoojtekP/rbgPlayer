#ifndef HASHMAP_ENTRY
#define HASHMAP_ENTRY

#include "reasoner.hpp"
#include "types.hpp"


struct HashmapEntry {
    reasoner::move move;
    uint next = 0;
    score total_score {0.0, 0.0};
    HashmapEntry() = default;
    HashmapEntry(const reasoner::move& mv, const int, double sum = 0.0, double weight = 0.0)
        : move(mv)
        , next(0)
        , total_score(sum, weight) {}
    inline bool equals(const reasoner::move& mv, const int) const {
        return move == mv;
    }
    int get_context() const {
        return 0;
    }
};

#endif
