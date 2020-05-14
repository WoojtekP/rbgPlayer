#include <random>

#include "node.hpp"
#include "reasoner.hpp"
#include "constants.hpp"

Node::Node(reasoner::game_state& state, const int& child_index) {
    state.get_all_moves(Node::cache, moves);
    // std::shuffle(moves.begin(), moves.end()); // zamiast późniejszego losowania w get_random_child (???)
    children = std::make_pair(child_index, child_index + moves.size());
}

bool Node::is_leaf() const {
    return moves.size() == 0;
}

bool Node::is_fully_expanded() const {
    return child_counter == (int)moves.size();
}

void Node::update_stats(int current_player, const simulation_result& results) {
    simulation_counter++;
    if (current_player != KEEPER)
        total_score += results[current_player-1] / 100.0;   // czy dzielenie przez 100.0 jest potrzebne???
}

std::pair<int, int> Node::get_children() const {
    return children;
}

int Node::get_simulation_counter() const {
    return simulation_counter;
}

double Node::get_total_score() const {
    return total_score;
}

reasoner::move Node::get_move_by_child_index(int child_index) const {
    return moves[child_index - children.first];
}

std::pair<reasoner::move, int> Node::get_random_move_and_child_index(std::mt19937& random_numbers_generator) {
    std::uniform_int_distribution<int> dist(child_counter, moves.size() - 1);
    auto move_id = dist(random_numbers_generator);
    if (child_counter != move_id)
        std::swap(moves[child_counter], moves[move_id]);
    child_counter++;     // TODO pozbyć się child_counter
    return std::make_pair(moves[child_counter-1], children.first + child_counter-1);
}

int Node::get_child_index_by_move(const reasoner::move& move) const {
    int i = children.first;
    for (const auto& child_move : moves) {
        if (child_move == move) {
            return i;
        }
        i++;
    }
    assert(false);
}
