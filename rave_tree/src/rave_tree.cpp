#include "rave_tree.hpp"

#include <algorithm>
#include <vector>

#include <boost/functional/hash.hpp>

#include "reasoner.hpp"
#include "constants.hpp"


std::size_t RaveTree::move_hash::operator()(const reasoner::move& move) const noexcept {
    std::size_t seed = 0;
    for (const auto& action : move.mr) {
        boost::hash_combine(seed, action.index);
        boost::hash_combine(seed, action.cell);
    }
    return seed;
}

int RaveTree::insert_or_update(const reasoner::move& move, const int) {
    moves.insert(move);
    return 0;
}

bool RaveTree::find(const reasoner::move& move, const int) {
    return moves.find(move) != moves.end();
}

void RaveTree::reset() {
    moves.clear();
}
