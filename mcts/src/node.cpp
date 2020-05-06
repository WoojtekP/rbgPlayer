#include <algorithm>
#include <random>

#include "constants.hpp"
#include "node.hpp"
#include "reasoner.hpp"

node::node(const reasoner::game_state& state, node* parent, const reasoner::resettable_bitarray_stack& cache) :
    parent(parent),
    state(state),
    cache(cache) {
    complete_turn();
    (this->state).get_all_moves(this->cache, moves);
    children.reserve(moves.size());
}

node::node(const reasoner::game_state& state) : node(state, nullptr, {}) {}

void node::complete_turn() {
    while (state.get_current_player() == KEEPER && state.apply_any_move(cache));
}

void node::increment_simulations_counter() {
    simulations_counter++;
}

void node::increment_wins_counter() {
    wins_counter++;
}

void node::set_parent(node* parent) {
    this->parent = parent;
}

void node::set_root() {
    parent = nullptr;
    for (auto& child : children) {
        child.set_parent(this);
    }
}

void node::reset(const reasoner::game_state& game_state) {
    parent = nullptr;
    state = game_state;
    cache = {};
    moves.clear();
    children.clear();
    simulations_counter = 0;
    wins_counter = 0;
    complete_turn();
    state.get_all_moves(cache, moves);
    children.reserve(moves.size());
}


int node::get_current_player() const {
    return state.get_current_player();
}

int node::get_simulations_count() const {
    return simulations_counter;
}

double node::get_priority() const {
    return static_cast<double>(wins_counter) / simulations_counter +
           EXPLORATION_CONSTANT * std::sqrt(std::log(parent->get_simulations_count()) / simulations_counter);
}

bool node::is_root() const {
    return parent == nullptr;
}

bool node::is_leaf() const {
    return moves.empty();
}

bool node::is_fully_expanded() const {
    return children.size() == moves.size();
}

node* node::get_best_uct() {
    std::vector<double> priorities;
    std::transform(children.begin(), children.end(), std::back_inserter(priorities),
        [](const auto& e) {
            return e.get_priority();
        });
    auto best_node = std::distance(priorities.begin(), std::max_element(priorities.begin(), priorities.end()));
    return &children[best_node];
}

node* node::get_random_child(std::mt19937& rand_gen) {
    int offset = children.size();
    std::uniform_int_distribution<int> dist(offset, moves.size()-1);
    int move_id = dist(rand_gen);
    if (offset != move_id) {
        std::swap(moves[offset], moves[move_id]);
    }
    reasoner::game_state next_state = state;
    next_state.apply_move(moves[offset]);
    children.emplace_back(next_state, this, cache);
    return &children.back();
}

node&& node::get_node_by_move(const reasoner::move& move) {
    const auto size = children.size();
    for (unsigned i = 0; i < size; ++i) {
        if (moves[i] == move) {
            return std::move(children[i]);
        }
    }
    reasoner::game_state next_state = state;
    next_state.apply_move(move);
    children.emplace_back(next_state, this, cache);
    return std::move(children.back());
}

node* node::get_parent() {
    return parent;
}

reasoner::move node::choose_best_move() {
    std::vector<int> simulations_counters;
    std::transform(children.begin(), children.end(), std::back_inserter(simulations_counters),
        [](const auto& e) {
            return e.get_simulations_count();
        });
    auto move_id = std::distance(simulations_counters.begin(), std::max_element(simulations_counters.begin(), simulations_counters.end()));
    return moves[move_id];
}

reasoner::game_state node::get_game_state() const {
    return state;
}

reasoner::resettable_bitarray_stack node::get_cache() const {
    return cache;
}

std::vector<reasoner::move> node::get_move_list() const {
    return moves;
}
