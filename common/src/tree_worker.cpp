#include "client_response.hpp"
#include "config.hpp"
#include "concurrent_queue.hpp"
#include "overloaded.hpp"
#include "reasoner.hpp"
#include "tree_indication.hpp"
#include "tree_handler.hpp"
#include "tree_worker.hpp"
#include "tree.hpp"


void run_tree_worker(concurrent_queue<client_response>& responses_to_server,
                     concurrent_queue<tree_indication>& tree_indications) {
    tree_handler th(responses_to_server, tree_indications);
    while (true) {
        const auto indication = tree_indications.pop_front();
        std::visit(overloaded {
            [&th](const reasoner::move& m) { th.handle_move_indication(m); },
            [&th](const move_request&) { th.handle_move_request(); },
            [&th](const simulation_request&) { th.handle_simulation_request(); },
            [&th](const reset_tree&) { th.handle_reset_request(); },
            [](auto) { assert(false); }
        }, indication.content);
    }
}
