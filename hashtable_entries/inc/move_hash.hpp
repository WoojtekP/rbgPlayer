#ifndef MOVE_HASH
#define MOVE_HASH

#include "reasoner.hpp"


template <typename T>
struct move_hash {
private:
    std::size_t hash(const reasoner::move& move) const noexcept {
        std::size_t hash = 0;
        for (const auto& action : move.mr) {
            hash = (hash * 13 + action.index) * 17 + action.cell;
        }
        return hash;
    }
public:
    std::size_t operator()(const reasoner::move& move, const int) const noexcept {
        return hash(move);
    }
    std::size_t operator()(const T& bucket) const noexcept {
        return hash(bucket.move);
    }
};

#endif