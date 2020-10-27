#include "moves_container.hpp"

#include <unordered_map>

#include <boost/functional/hash.hpp>

#include "constants.hpp"


std::size_t MovesContainer::move_hash::operator()(const reasoner::move& move) const noexcept {
    std::size_t seed = 0;
    for (const auto& action : move.mr) {
        //boost::hash_combine(seed, action.index);
        //boost::hash_combine(seed, action.cell);
        seed = (seed * 13 + action.index) * 17 + action.cell;
    }
    return seed;
}

int MovesContainer::insert_or_update(const reasoner::move& move, const uint score, const uint, const int) {
    auto it = map.find(move);
    if (it == map.end()) {
        map.insert({move, {1.0, static_cast<double>(score)}});
    }
    else {
        ++(it->second.first);
        it->second.second += static_cast<double>(score);
    }
    return 0;
}

double MovesContainer::get_score_or_default_value(const reasoner::move& move, const int) {
    auto it = map.find(move);
    if (it == map.end()) {
        return EXPECTED_MAX_SCORE;
    }
    else {
        return it->second.second / it->second.first;
    }
}

void MovesContainer::apply_decay_factor() {
    for (auto& el : map) {
        el.second.first *= DECAY_FACTOR;
        el.second.second *= DECAY_FACTOR;
    }
}
