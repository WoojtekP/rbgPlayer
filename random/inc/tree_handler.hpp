#ifndef TREE_HANDLER
#define TREE_HANDLER

#include <random>
#include<string>

#include"types.hpp"
#include"reasoner.hpp"

template<typename T>
class concurrent_queue;
class simulation_request;
class simulation_response;
class client_response;
class move_indication;

namespace reasoner{
    class game_state;
}

class tree_handler{
        uint own_player_index;
        reasoner::game_state state;
        reasoner::resettable_bitarray_stack cache;
        std::mt19937 random_numbers_generator;
        concurrent_queue<client_response>& responses_to_server;
        std::vector<reasoner::move> move_list;
        void go_to_completion();
        void handle_status(void);
    public:
        tree_handler(void)=delete;
        tree_handler(const tree_handler&)=delete;
        tree_handler(tree_handler&&)=default;
        tree_handler& operator=(const tree_handler&)=delete;
        tree_handler& operator=(tree_handler&&)=default;
        ~tree_handler(void)=default;
        tree_handler(const reasoner::game_state& initial_state,
                     concurrent_queue<client_response>& responses_to_server);
        void handle_move_request(void);
        void handle_move_indication(const reasoner::move& m);
        void handle_reset_request(const reasoner::game_state& initial_state);
};

#endif
