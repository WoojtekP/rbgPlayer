#ifndef TREE_HANDLER
#define TREE_HANDLER

#include <string>

#include "tree.hpp"
#include "types.hpp"


template<typename T>
class concurrent_queue;
class simulation_request;
class simulation_response;
class client_response;
class move_indication;

namespace reasoner {
class game_state;
}

class tree_handler{
    Tree* t;
    const int own_player_index;
    uint simulations_count = 0;
    uint states_count = 0;
    concurrent_queue<client_response>& responses_to_server;
    void handle_status(void);
public:
    tree_handler(void) = delete;
    tree_handler(const tree_handler&) = delete;
    tree_handler(tree_handler&&) = delete;
    tree_handler& operator=(const tree_handler&) = delete;
    tree_handler& operator=(tree_handler&&) = delete;
    tree_handler(const reasoner::game_state&, concurrent_queue<client_response>&);
    ~tree_handler(void);
    void handle_move_request(void);
    void handle_move_indication(const reasoner::move&);
    void handle_reset_request(const reasoner::game_state&);
    uint perform_simulation();
    game_status_indication get_game_status();
};

#endif
