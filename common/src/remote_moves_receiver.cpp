#include"remote_moves_receiver.hpp"
#include"reasoner.hpp"
#include"constants.hpp"
#include<cassert>
#include<sstream>
#include<vector>
#include<cmath>
#include<iostream>

remote_moves_receiver::remote_moves_receiver(int socket_descriptor)
  : socket_descriptor(socket_descriptor){}

namespace{
std::vector<std::string> split_string(const std::string& t, char delimiter){
    std::vector<std::string> result;
    std::string current_part;
    for(const auto el: t){
        if(el == delimiter){
            if(not current_part.empty()){
                result.push_back(std::move(current_part));
                current_part.clear();
            }
        }
        else
            current_part.push_back(el);
    }
    if(not current_part.empty())
        result.push_back(std::move(current_part));
    return result;

}

reasoner::move string_to_move(const std::string& t){
    auto split_text = split_string(t, ' ');
    reasoner::move result;
    assert(split_text.size() % 2 == 0);
    for(uint i=0;i<split_text.size();i+=2){
        uint cell = std::stoi(split_text[i]);
        uint index = std::stoi(split_text[i+1]);
        result.mr.emplace_back(index, cell);
    }
    return result;
}
}

bool remote_moves_receiver::extend_buffer(void){
    uint previous_size = buffer.size();
    buffer.resize(previous_size+SOCKET_BUFFER_SIZE, '\0');
    char* str = &buffer[previous_size];
    int n = read(socket_descriptor, str, SOCKET_BUFFER_SIZE);
    buffer.resize(previous_size+n); // shrink to actually read bytes
    return n>=1;
}

std::string remote_moves_receiver::cut_buffer_at(uint index){
    std::string new_buffer = buffer.substr(index+1);
    buffer.resize(index);
    buffer.swap(new_buffer);
    return new_buffer;
}

std::string remote_moves_receiver::read_until(char byte){
    uint last_checked_index = 0;
    do{
        for(;last_checked_index<buffer.size();++last_checked_index)
            if(buffer[last_checked_index] == byte)
                return cut_buffer_at(last_checked_index);
    }while(extend_buffer());
    std::string to_return;
    buffer.swap(to_return);
    return to_return;
}

reasoner::move remote_moves_receiver::receive_move(void){
    auto sdu = read_until('\0');
    return string_to_move(sdu);
}

uint remote_moves_receiver::receive_miliseconds_limit(void){
    auto sdu = read_until('\0');
    double seconds_for_turn = std::stod(sdu);
    std::cout << "[PLAYER] Have " << seconds_for_turn << " seconds for turn." << std::endl;
    return lround(seconds_for_turn*MILISECONDS_IN_SECONDS);
}
