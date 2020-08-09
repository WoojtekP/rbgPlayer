#include"tree_handler.hpp"
#include"tree.hpp"
#include"concurrent_queue.hpp"
#include"client_response.hpp"
#include"constants.hpp"
#include<iostream>


namespace{
int get_player_index(const std::string& name){
    for(int i = 0; i < reasoner::NUMBER_OF_PLAYERS - 1; ++i)
        if(reasoner::variable_to_string(i) == name)
            return i;
    assert(false); // no such player?
}
}

tree_handler::tree_handler(const reasoner::game_state& initial_state,
                           concurrent_queue<client_response>& responses_to_server)
  : t(initial_state)
  , own_player_index(get_player_index(NAME))
  , responses_to_server(responses_to_server){
    handle_status();
}

void tree_handler::handle_status(void){
    auto status = t.get_status(own_player_index);
    responses_to_server.emplace_back(client_response{status});
}

void tree_handler::handle_move_request(void){
    const auto& chosen_move = t.choose_best_move();
    responses_to_server.emplace_back(client_response{chosen_move});
    std::cout << "[PLAYER] Performing move on the basis of "
              << simulations_count << " simulations ("
              << states_count << " states)" << std::endl;
    handle_move_indication(chosen_move);
}

void tree_handler::handle_move_indication(const reasoner::move& m){
    simulations_count = 0;
    states_count = 0;
    t.reparent_along_move(m);
    handle_status();
}

void tree_handler::handle_reset_request(const reasoner::game_state& initial_state){
    std::cout << "[PLAYER] Reset Request!" << std::endl;
    t = initial_state;
    simulations_count = 0;
    states_count = 0;
    handle_status();
}

uint tree_handler::perform_simulation() {
    simulations_count++;
    uint states = t.perform_simulation();
    states_count += states;
    return states;
}

game_status_indication tree_handler::get_game_status() {
    return t.get_status(own_player_index);
}
