#include<cassert>
#include<condition_variable>
#include<thread>
#include<mutex>

#include<arpa/inet.h>
#include<sys/socket.h>

#include"concurrent_queue.hpp"
#include"config.hpp"
#include"client_response.hpp"
#include"own_moves_sender.hpp"
#include"remote_moves_receiver.hpp"
#include"transport_worker.hpp"
#include"tree_indication.hpp"
#include"tree_worker.hpp"
#include"types.hpp"


int connect_to_play(){
    int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_descriptor < 0)
        return 0;
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, ADDRESS.data(), &server_address.sin_addr);
    auto connect_result = connect(socket_descriptor, (sockaddr*)&server_address, sizeof(server_address));
    if(connect_result < 0)
        return 0;
    return socket_descriptor;
}

int main(){
    concurrent_queue<client_response> client_responses;
    concurrent_queue<tree_indication> tree_indications;
    auto socket_descriptor = connect_to_play();
    if(not socket_descriptor)
        return -1;
    remote_moves_receiver rmr(socket_descriptor);
    own_moves_sender oms(socket_descriptor);
    std::condition_variable cv;
    std::mutex cv_mutex;
    std::thread transportw(
        run_transport_worker,
        std::ref(rmr),
        std::ref(oms),
        std::ref(tree_indications),
        std::ref(client_responses),
        std::ref(cv));
    std::thread treew(
        run_tree_worker,
        std::ref(client_responses),
        std::ref(tree_indications),
        std::ref(cv),
        std::ref(cv_mutex));
    transportw.join();
    close(socket_descriptor);
    treew.join();
    return 0;
}
