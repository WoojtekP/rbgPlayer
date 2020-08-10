#ifndef TRANSPORT_WORKER
#define TRANSPORT_WORKER

#include"types.hpp"

template<typename T>
class concurrent_queue;
class client_response;
class tree_indication;
class remote_moves_receiver;
class own_moves_sender;

void run_transport_worker(remote_moves_receiver& rmr,
                          own_moves_sender& oms,
                          concurrent_queue<tree_indication>& tree_indications,
                          concurrent_queue<client_response>& responses_from_tree);

#endif
