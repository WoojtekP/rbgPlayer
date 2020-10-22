#ifndef RAVETREE
#define RAVETREE

#include <unordered_set>

#include "constants.hpp"
#include "reasoner.hpp"


struct move_hash {
    std::size_t operator()(const reasoner::move& move) const noexcept;
};

class RaveTree {
private:
    std::unordered_set<reasoner::move, move_hash> moves;
public:
    int insert_or_update(const reasoner::move&, const int = 0);
    bool find(const reasoner::move&, const int = 0);
    void reset();
};

#endif
