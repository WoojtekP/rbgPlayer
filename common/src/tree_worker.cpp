#include<condition_variable>
#include<mutex>
#include<thread>

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
                     concurrent_queue<tree_indication>& tree_indications,
                     std::condition_variable& cv,
                     std::mutex& cv_mutex) {
    reasoner::game_state initial_state;
    tree_handler th(initial_state, responses_to_server);
    std::unique_lock<std::mutex> lock(cv_mutex);
    cv.wait(lock);
    while(true) {
        auto status = th.get_game_status();
        if (status == own_turn) {
            if constexpr (SIMULATIONS_LIMIT) {
                uint sim_count = 0;
                while (sim_count < SIMULATIONS_PER_MOVE) {
                    th.perform_simulation();
                    ++sim_count;
                }
                th.handle_move_request();
                continue;
            }
            else if constexpr (STATES_LIMIT) {
                uint state_count = 0;
                while (state_count < STATES_PER_MOVE) {
                    state_count += th.perform_simulation();
                }
                th.handle_move_request();
                continue;
            }
            else {
                while (tree_indications.empty()) {
                    th.perform_simulation();
                }
            }
        }
        const auto indication = tree_indications.pop_front();
        std::visit(overloaded {
            [&th](const reasoner::move& m){ th.handle_move_indication(m); },
            [&th](const move_request&){ th.handle_move_request(); },
            [&th,&initial_state](const reset_tree&){ th.handle_reset_request(initial_state); },
            [](auto){ assert(false); }
        }, indication.content);
    }
}
