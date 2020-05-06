#ifndef CONCURRENT_QUEUE
#define CONCURRENT_QUEUE

// based on: https://github.com/Pixinn/misc/concurrent_deque

#include"types.hpp"

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class concurrent_queue{
        std::queue<T> collection;
        std::mutex mutex;
        std::condition_variable new_data_notifier;
        std::condition_variable free_space_notifier;
    public:
        template<typename... Args>
        void emplace_back(Args&&... args){
            std::unique_lock<std::mutex> lock{mutex};
            collection.emplace(std::forward<Args>(args)...);
            lock.unlock();
            new_data_notifier.notify_one();
        }

        template<typename... Args>
        void emplace_back_or_block_if_too_much(uint limit, Args&&... args){
            std::unique_lock<std::mutex> lock{mutex};
            while(collection.size()>=limit)
                free_space_notifier.wait(lock);
            collection.emplace(std::forward<Args>(args)...);
            lock.unlock();
            new_data_notifier.notify_one();
        }

        size_t size(void){
            std::unique_lock<std::mutex> lock{mutex};
            return collection.size();
        }

        bool empty(){
            return collection.empty();
        }

        T pop_front(void)noexcept{
            std::unique_lock<std::mutex> lock{mutex};
            while(collection.empty())
                new_data_notifier.wait(lock);
            auto elem = std::move(collection.front());
            collection.pop();
            lock.unlock();
            free_space_notifier.notify_one();
            return elem;
        }
};

#endif
