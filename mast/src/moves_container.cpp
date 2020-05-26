#include <boost/functional/hash.hpp>

#include "moves_container.hpp"

std::size_t move_hash::operator()(const reasoner::move& move) const noexcept {
    std::size_t seed = 0;
    for (const auto& action : move.mr) {
        boost::hash_combine(seed, action.index);
        boost::hash_combine(seed, action.cell);
    }
    return seed;
}

void moves_container::insert_or_update(const reasoner::move& move, const uint& score) {
    auto it = map.find(move);
    if (it == map.end()) {
        map.insert({move, {1, score}});
    }
    else {
        it->second.first++;
        it->second.second += score;
    }
}

uint moves_container::get_score_or_default_value(const reasoner::move& move) {
    auto it = map.find(move);
    if (it == map.end()) {
        return default_value;
    }
    else {
        return static_cast<double>(it->second.second) / it->second.first;
    }
}
