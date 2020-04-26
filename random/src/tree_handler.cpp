#include<iostream>

#include"concurrent_queue.hpp"
#include"config.hpp"
#include"client_response.hpp"
#include"tree_handler.hpp"

namespace{
uint get_player_index(const std::string& name){
    for(uint i=0;i<reasoner::NUMBER_OF_PLAYERS-1;++i)
        if(reasoner::variable_to_string(i) == name)
            return i + 1;
    assert(false); // no such player?
}
}

void tree_handler::go_to_completion() {
    while (state.get_current_player() == 0 && state.apply_any_move(cache));
}

tree_handler::tree_handler(const reasoner::game_state& initial_state,
                           concurrent_queue<client_response>& responses_to_server)
  : own_player_index(get_player_index(NAME))
  , state(initial_state)
  , random_numbers_generator(std::random_device{}())
  , responses_to_server(responses_to_server){
    go_to_completion();
    handle_status();
}

void tree_handler::handle_status(void){
    uint current_player = state.get_current_player();
    game_status_indication status;
    if (current_player == 0) {
        status = end_game;
    }
    else if (current_player == own_player_index)
        status = own_turn;
    else
        status = opponent_turn;
    responses_to_server.emplace_back(client_response{status});
}

void tree_handler::handle_move_request(void){
    std::vector<reasoner::move> move_list;
    state.get_all_moves(cache, move_list);
    assert(!move_list.empty());
    uint move_id = std::uniform_int_distribution<uint>(0,move_list.size()-1)(random_numbers_generator);
    std::cout << "[PLAYER] Performing random move" << std::endl;
    state.apply_move(move_list[move_id]);
    responses_to_server.emplace_back(client_response{move_list[move_id]});
    go_to_completion();
    handle_status();
}

void tree_handler::handle_move_indication(const reasoner::move& m){
    state.apply_move(m);
    go_to_completion();
    handle_status();
}

void tree_handler::handle_reset_request(const reasoner::game_state& initial_state){
    std::cout << "[PLAYER] Reset Request!" << std::endl;
    state = initial_state;
    cache = {};
    go_to_completion();
    handle_status();
}
