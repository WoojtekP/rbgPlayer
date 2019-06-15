#ifndef REMOTE_MOVES_RECEIVER
#define REMOTE_MOVES_RECEIVER

#include"types.hpp"
#include"constants.hpp"

namespace reasoner{
    class move;
}

class remote_moves_receiver{
        int socket_descriptor;
        std::string buffer = {};
        uint next_data_index = 0;
        void fill_buffer(void);
    public:
        remote_moves_receiver(void)=delete;
        remote_moves_receiver(const remote_moves_receiver&)=delete;
        remote_moves_receiver& operator=(const remote_moves_receiver&)=delete;
        remote_moves_receiver(remote_moves_receiver&&)=default;
        remote_moves_receiver& operator=(remote_moves_receiver&&)=default;
        remote_moves_receiver(int socket_descriptor);
        reasoner::move receive_move(void);
};

#endif