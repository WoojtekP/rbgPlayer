#ifndef TREE_HANDLER
#define TREE_HANDLER

#include <string>

#if STATS
#include <chrono>
#endif

#include "tree.hpp"
#include "types.hpp"


template <typename T>
class concurrent_queue;
class simulation_request;
class simulation_response;
class client_response;
class move_indication;
struct tree_indication;


class tree_handler {
    Tree* t;
    const int own_player_index;
    uint simulations_count = 0;
    uint states_count = 0;
    #if STATS
    long int longest_simulation = 0;
    #endif
    concurrent_queue<client_response>& responses_to_server;
    const concurrent_queue<tree_indication>& tree_indications;
    void handle_status(void);
public:
    tree_handler(void) = delete;
    tree_handler(const tree_handler&) = delete;
    tree_handler(tree_handler&&) = delete;
    tree_handler& operator=(const tree_handler&) = delete;
    tree_handler& operator=(tree_handler&&) = delete;
    tree_handler(concurrent_queue<client_response>&, const concurrent_queue<tree_indication>&);
    ~tree_handler(void);
    void handle_move_request(void);
    void handle_move_indication(const reasoner::move&);
    void handle_simulation_request();
    void handle_reset_request();
    uint perform_simulation();
    game_status_indication get_game_status();
};

#endif
