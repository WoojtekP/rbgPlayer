#include"client_response.hpp"
#include"concurrent_queue.hpp"
#include"overloaded.hpp"
#include"reasoner.hpp"
#include"tree_indication.hpp"
#include"tree_handler.hpp"
#include"tree_worker.hpp"
#include"config.hpp"
#include"tree.hpp"


void run_tree_worker(concurrent_queue<client_response>& responses_to_server,
                     concurrent_queue<tree_indication>& tree_indications){
    reasoner::game_state initial_state;
    tree_handler th(initial_state, responses_to_server);
    while(true){
        const auto status = th.get_game_status();
        if (not SLEEP_DURING_OPP_TURN or status == own_turn) {
            uint i = 0;
            while (i < SIMULATIONS_PER_MOVE && tree_indications.empty()) {
                th.perform_simulation();
                ++i;
            }
            if (i == SIMULATIONS_PER_MOVE && status == own_turn) {
                th.handle_move_request();
            }
        }
        const auto indication = tree_indications.pop_front();
        std::visit(overloaded{
            [&th](const reasoner::move& m){th.handle_move_indication(m);},
            [&th](const move_request&){th.handle_move_request();},
            [&th,&initial_state](const reset_tree&){th.handle_reset_request(initial_state);},
            [](auto){assert(false);}
        }, indication.content);
    }
}
