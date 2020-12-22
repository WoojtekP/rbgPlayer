#ifndef HASHMAP_ENTRY
#define HASHMAP_ENTRY

#include "reasoner.hpp"
#include "types.hpp"


struct HashmapEntry {
    reasoner::move move;
    uint next = 0;
    score total_score;
    HashmapEntry() = default;
    HashmapEntry(const reasoner::move& mv, double sum, double weight, const int = 0)
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
