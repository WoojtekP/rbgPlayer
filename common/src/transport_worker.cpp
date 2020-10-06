#include <chrono>
#include <optional>
#include <thread>

#include "transport_worker.hpp"
#include "concurrent_queue.hpp"
#include "client_response.hpp"
#include "tree_indication.hpp"
#include "constants.hpp"
#include "remote_moves_receiver.hpp"
#include "own_moves_sender.hpp"


namespace {
game_status_indication get_game_status(concurrent_queue<client_response>& responses_from_tree) {
    auto response = responses_from_tree.pop_front();
    assert(std::holds_alternative<game_status_indication>(response.content));
    return std::get<game_status_indication>(response.content);
}

void forward_move_from_player_to_server(own_moves_sender& oms,
                                        concurrent_queue<client_response>& responses_from_tree) {
    auto response = responses_from_tree.pop_front();
    assert(std::holds_alternative<reasoner::move>(response.content));
    auto m = std::get<reasoner::move>(response.content);
    oms.send_move(m);
}

void forward_move_from_server_to_player(remote_moves_receiver& rmr,
                                        concurrent_queue<tree_indication>& tree_indications) {
    auto m = rmr.receive_move();
    tree_indications.emplace_back(tree_indication{m});
}

uint trunctated_subtraction(uint a, uint b) {
    return b > a ? 0 : a-b;
}

void wait_for_move(uint milisecond_to_wait,
                   concurrent_queue<tree_indication>& tree_indications) {
    uint reduced_time = trunctated_subtraction(milisecond_to_wait, BUFFER_TIME);
    std::this_thread::sleep_for(std::chrono::milliseconds(reduced_time));
    tree_indications.emplace_back(tree_indication{move_request{}});
}

void handle_own_turn(remote_moves_receiver& rmr,
                     own_moves_sender& oms,
                     concurrent_queue<tree_indication>& tree_indications,
                     concurrent_queue<client_response>& responses_from_tree) {
    uint miliseconds_left = rmr.receive_miliseconds_limit();
    tree_indications.emplace_back(tree_indication{simulation_request{}});
    if constexpr (!SIMULATIONS_LIMIT && !STATES_LIMIT) {
        wait_for_move(miliseconds_left, tree_indications);
    }
    forward_move_from_player_to_server(oms, responses_from_tree);
}
}  // namespace

void run_transport_worker(remote_moves_receiver& rmr,
                          own_moves_sender& oms,
                          concurrent_queue<tree_indication>& tree_indications,
                          concurrent_queue<client_response>& responses_from_tree) {
    while (true) {
        game_status_indication status = get_game_status(responses_from_tree);
        switch (status) {
            case own_turn:
                handle_own_turn(rmr, oms, tree_indications, responses_from_tree);
                break;
            case opponent_turn:
                forward_move_from_server_to_player(rmr, tree_indications);
                break;
            case end_game:
                tree_indications.emplace_back(tree_indication{reset_tree{}});
                break;
        }
    }
}
