#include <boost/functional/hash.hpp>

#include "config.hpp"
#include "moves_container.hpp"

std::size_t move_hash::operator()(const reasoner::move& move) const noexcept {
    std::size_t seed = 0;
    for (const auto& action : move.mr) {
        boost::hash_combine(seed, action.index);
        boost::hash_combine(seed, action.cell);
    }
    return seed;
}

void moves_container::insert_or_update(const reasoner::move& move, const uint& score, [[maybe_unused]] const uint& depth) {
    auto it = map.find(move);
    if constexpr (WEIGHT_SCALING == 1) {  // stała z config.hpp
        if (it == map.end()) {
            map.insert({move, {1.0 / depth, static_cast<double>(score) / depth}});
        }
        else {
            it->second.first += 1.0 / depth;
            it->second.second += static_cast<double>(score) / depth;
        }
    }
    else {
        if (it == map.end()) {
            map.insert({move, {1.0, static_cast<double>(score)}});
        }
        else {
            it->second.first += 1.0;
            it->second.second += static_cast<double>(score);
        }
    }
}

double moves_container::get_score_or_default_value(const reasoner::move& move) {
    auto it = map.find(move);
    if (it == map.end()) {
        return default_value;
    }
    else {
        return it->second.second / it->second.first;
    }
}

void moves_container::apply_decay_factor() {
    for (auto& [key, val] : map) {
        val.first *= decay_factor;
        val.second *= decay_factor;
    }
}
