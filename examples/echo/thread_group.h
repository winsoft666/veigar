#ifndef THREAD_GROUP_H
#define THREAD_GROUP_H
#pragma once

#include <thread>
#include <vector>

class ThreadGroup {
   public:
    ThreadGroup() {}
    ThreadGroup(ThreadGroup const&) = delete;

    void createThreads(std::size_t thread_count, std::string targetName, std::function<void(std::size_t, std::string)> func) {
        for (std::size_t i = 0; i < thread_count; ++i) {
            threads_.push_back(std::thread(func, i, targetName));
        }
    }

    void joinAll() {
        for (auto& t : threads_) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    ~ThreadGroup() { joinAll(); }

   private:
    std::vector<std::thread> threads_;
};

#endif