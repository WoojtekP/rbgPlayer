#include <algorithm>
#include <random>

#include "node.hpp"
#include "reasoner.hpp"
#include "constants.hpp"

Node::Node(reasoner::game_state& state, const uint& child_index) {
    state.get_all_moves(Node::cache, moves);
    const auto size = moves.size();
    children = std::make_pair(child_index, child_index + size);
    simulation_counters.reserve(size);
    total_scores.reserve(size);
}

bool Node::is_terminal() const {
    return moves.empty();
}

bool Node::is_fully_expanded() const {
    return simulation_counters.size() == moves.size();
}

void Node::update_stats(int current_player, uint child_index, const simulation_result& results) {
    simulation_counter++;
    if (child_index != 0) {
        child_index -= children.first;
        simulation_counters[child_index]++;
        total_scores[child_index] += results[current_player - 1];
    }
}

void Node::set_children(const uint& fst) {
    children = std::make_pair(fst, fst + moves.size());
}

std::pair<uint, uint> Node::get_children() const {
    return children;
}

uint Node::get_child_index_by_move(const reasoner::move& move) const {
    auto i = children.first;
    for (const auto& child_move : moves) {
        if (child_move == move) {
            return i;
        }
        i++;
    }
    assert(false);
}

std::pair<reasoner::move, uint> Node::get_best_uct_and_child_index(std::mt19937& random_numbers_generator) {
    static std::vector<uint> best_children_indices;
    best_children_indices.resize(1);
    best_children_indices[0] = 0;
    double logN = std::log(simulation_counter);
    double maxPriority = total_scores.front() / EXPECTED_MAX_SCORE / simulation_counters.front() +
                         EXPLORATION_CONSTANT * std::sqrt(logN / simulation_counters.front());
    const uint size = children.second - children.first;
    for (uint i = 1; i < size; ++i) {
        double priority = total_scores[i] / EXPECTED_MAX_SCORE / simulation_counters[i] +
                          EXPLORATION_CONSTANT * std::sqrt(logN / simulation_counters[i]);
        if (priority > maxPriority) {
            maxPriority = priority;
            best_children_indices.resize(1);
            best_children_indices[0] = i;
        }
        else if (priority == maxPriority) {
            best_children_indices.push_back(i);
        }
    }
    uint best_child_index = best_children_indices.front();
    if (best_children_indices.size() > 1) {
        std::uniform_int_distribution<> dist(0, best_children_indices.size() - 1);
        best_child_index = best_children_indices[dist(random_numbers_generator)];
    }
    return { moves[best_child_index], best_child_index + children.first };
}


std::pair<reasoner::move, uint> Node::get_random_move_and_child_index(std::mt19937& random_numbers_generator) {
    auto child_counter = simulation_counters.size();
    std::uniform_int_distribution<uint> dist(child_counter, moves.size() - 1);
    auto move_id = dist(random_numbers_generator);
    if (child_counter != move_id)
        std::swap(moves[child_counter], moves[move_id]);
    simulation_counters.push_back(0);
    total_scores.push_back(0.0);
    return { moves[child_counter], children.first + child_counter };
}

reasoner::move Node::choose_best_move() const {
    auto mx = std::max_element(simulation_counters.begin(), simulation_counters.end());
    return moves[std::distance(simulation_counters.begin(), mx)];
}
