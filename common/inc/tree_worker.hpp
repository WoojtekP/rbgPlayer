#ifndef TREE_WORKER
#define TREE_WORKER

#include<condition_variable>
#include<mutex>
#include<string>

template<typename T>
class concurrent_queue;
class simulation_request;
class tree_indication;
class client_response;

namespace reasoner{
    class game_state;
}

void run_tree_worker(concurrent_queue<client_response>& responses_to_server,
                     concurrent_queue<tree_indication>& tree_indications,
                     std::condition_variable& cv,
                     std::mutex& cv_mutex);

#endif
