#include <exception>
#include <iostream>

#include "tree_indication.hpp"
#include "tree_handler.hpp"
#include "tree.hpp"
#include "concurrent_queue.hpp"
#include "client_response.hpp"
#include "constants.hpp"


namespace {
int get_player_index(const std::string& name) {
    for (int i = 0; i < reasoner::NUMBER_OF_PLAYERS - 1; ++i)
        if (reasoner::variable_to_string(i) == name)
            return i;
    throw std::invalid_argument("player \'" + name + "\' not found!");
}
}  // namespace

tree_handler::tree_handler(concurrent_queue<client_response>& responses_to_server,
                           const concurrent_queue<tree_indication>& tree_indications)
  : t(new Tree)
  , own_player_index(get_player_index(NAME))
  , responses_to_server(responses_to_server)
  , tree_indications(tree_indications) {
    handle_status();
}

tree_handler::~tree_handler(void) {
    delete t;
}

void tree_handler::handle_status(void) {
    auto status = t->get_status(own_player_index);
    responses_to_server.emplace_back(client_response{status});
}

void tree_handler::handle_move_request(void) {
    const auto& chosen_move = t->choose_best_move();
    responses_to_server.emplace_back(client_response{chosen_move});
    std::cout << "[PLAYER] Performing move on the basis of "
              << simulations_count << " simulations ("
              << states_count << " states)" << std::endl;
    handle_move_indication(chosen_move);
}

void tree_handler::handle_move_indication(const reasoner::move& m) {
    simulations_count = 0;
    states_count = 0;
    t->reparent_along_move(m);
    handle_status();
}

void tree_handler::handle_simulation_request() {
    if constexpr (SIMULATIONS_LIMIT) {
        uint sim_count = 0;
        while (sim_count < SIMULATIONS_PER_MOVE) {
            perform_simulation();
            ++sim_count;
        }
        handle_move_request();
    }
    else if constexpr (STATES_LIMIT) {
        uint state_count = 0;
        while (state_count < STATES_PER_MOVE) {
            state_count += perform_simulation();
        }
        handle_move_request();
    }
    else {
        while (tree_indications.empty()) {
            perform_simulation();
        }
    }
}

void tree_handler::handle_reset_request() {
    std::cout << "[PLAYER] Reset Request!" << std::endl;
    delete t;
    t = new Tree;
    simulations_count = 0;
    states_count = 0;
    handle_status();
}

uint tree_handler::perform_simulation() {
    simulations_count++;
    uint states = t->perform_simulation();
    states_count += states;
    return states;
}

game_status_indication tree_handler::get_game_status() {
    return t->get_status(own_player_index);
}
