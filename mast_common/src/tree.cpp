#include "constants.hpp"
#include "tree.hpp"
#include "node.hpp"
#include "moves_container.hpp"

Tree::Tree(const reasoner::game_state& initial_state) : MctsTree(initial_state), prob(0.0, 1.0) {}

uint Tree::get_unvisited_child_index(const uint& node_index, const uint& current_player) {
    static std::vector<uint> children_indices;
    const auto& [fst, lst] = nodes[node_index].children_range;
    children_indices.resize(1);
    double best_score = 0.0;
    uint child_index = 0;
    if (prob(random_numbers_generator) < epsilon) {
        for (uint i = fst; i < lst; ++i) {
            if (children[i].index == 0) {
                double score = moves[current_player - 1].get_score_or_default_value(children[i].move);
                if (score > best_score) {
                    best_score = score;
                    children_indices.resize(1);
                    children_indices[0] = i;
                }
                else if (score == best_score) {
                    children_indices.push_back(i);
                }
            }
        }
    }
    else {
        uint unvisited = lst - fst;
        children_indices.resize(unvisited);
        uint j = 0;
        for (uint i = fst; i < lst; ++i) {
            if (children[i].index == 0) {
                children_indices[j] = i;
                j++;
            }
        }
    }
    child_index = children_indices.front();
    if (children_indices.size() > 1) {
        std::uniform_int_distribution<uint> dist(0, children_indices.size() - 1);
        child_index = children_indices[dist(random_numbers_generator)];
    }
    return child_index;
}

void Tree::play(reasoner::game_state& state, simulation_result& results) {
    static std::vector<reasoner::move> legal_moves;
    static std::vector<uint> moves_indices;
    static std::vector<reasoner::move> move_list[reasoner::NUMBER_OF_PLAYERS - 1];
    while (true) {
        state.get_all_moves(cache, legal_moves);
        if(legal_moves.empty()) {
            break;
        }
        else {
            const auto current_player = state.get_current_player();
            const auto size = legal_moves.size();
            uint chosen_move;
            if (prob(random_numbers_generator) < epsilon) {
                double best_score = moves[current_player - 1].get_score_or_default_value(legal_moves.front());
                moves_indices.resize(1);
                moves_indices[0] = 0;
                for (uint i = 1; i < size; ++i) {
                    double score = moves[current_player - 1].get_score_or_default_value(legal_moves[i]);
                    if (score > best_score) {
                        best_score = score;
                        moves_indices.resize(1);
                        moves_indices[0] = i;
                    }
                    else if (score == best_score) {
                        moves_indices.push_back(i);
                    }
                }
                chosen_move = moves_indices.front();
                if (moves_indices.size() > 1) {
                    std::uniform_int_distribution<uint> dist(0, moves_indices.size() - 1);
                    chosen_move = moves_indices[dist(random_numbers_generator)];
                }
            }
            else {
                std::uniform_int_distribution<uint> dist(0, size - 1);
                chosen_move = dist(random_numbers_generator);
            }
            state.apply_move(legal_moves[chosen_move]);
            moves[current_player - 1].apply_decay_factor();
            if constexpr (HEURISTIC_NAME == "MAST") {
                depth++;
                move_list[current_player - 1].push_back(legal_moves[chosen_move]);
            }
        }
        while (state.get_current_player() == KEEPER) {
            if (not state.apply_any_move(cache)) {
                break;
            }
        }
    }
    for (int i = 1; i <= reasoner::NUMBER_OF_PLAYERS; ++i) {
        results[i - 1] = state.get_player_score(i);
    }
    if constexpr (HEURISTIC_NAME == "MAST") {
        for (int i = 0; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
            for (const auto& move : move_list[i]) {
                moves[i].insert_or_update(move, results[i], depth);
            }
            move_list[i].clear();
        }
    }
}


void Tree::mcts(reasoner::game_state& state, const uint& node_index, simulation_result& results) {
    depth++;
    if (nodes[node_index].is_terminal()) {
        for (int i = 1; i <= reasoner::NUMBER_OF_PLAYERS; ++i) {
            results[i - 1] = state.get_player_score(i);
        }
    }
    else {
        uint current_player = state.get_current_player();
        uint child_index;
        if (nodes[node_index].is_fully_expanded()) {
            child_index = get_best_uct_child_index(node_index);
            state.apply_move(children[child_index].move);
            complete_turn(state);
            moves[current_player - 1].apply_decay_factor();
            mcts(state, children[child_index].index, results);
        }
        else {
            child_index = get_unvisited_child_index(node_index, current_player);
            state.apply_move(children[child_index].move);
            complete_turn(state);
            moves[current_player - 1].apply_decay_factor();
            children[child_index].index = create_node(state);
            play(state, results);
        }
        children[child_index].sim_count++;
        children[child_index].total_score += results[current_player - 1];
        moves[current_player - 1].insert_or_update(children[child_index].move, results[current_player - 1], depth);
    }
    nodes[node_index].sim_count++;
}

void Tree::perform_simulation() {
    static simulation_result results(reasoner::NUMBER_OF_PLAYERS);
    reasoner::game_state root_state_copy = root_state;
    static const uint root_index = 0;
    depth = 0;
    mcts(root_state_copy, root_index, results);
}
