#ifndef TREE_WORKER
#define TREE_WORKER

#include <string>


template <typename T>
class concurrent_queue;
class simulation_request;
class tree_indication;
class client_response;


void run_tree_worker(concurrent_queue<client_response>&, concurrent_queue<tree_indication>&);

#endif
