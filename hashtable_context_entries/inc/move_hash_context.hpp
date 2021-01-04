#ifndef MOVE_HASH_CONTEXT
#define MOVE_HASH_CONTEXT

#include "reasoner.hpp"


template <typename T>
struct move_hash_context {
private:
    std::size_t hash(const reasoner::action_representation action, const int context) const noexcept {
        if (action.index > 0) {
            return (action.index * 17 + action.cell) * 13 + context;
        }
        return context;
    }
public:
    std::size_t operator()(const reasoner::action_representation action, const int context) const noexcept {
        return hash(action, context);
    }
    std::size_t operator()(const T& bucket) const noexcept {
        return hash(bucket.move, bucket.context);
    }
};

#endif