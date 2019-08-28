#include <iostream>
#include <future>
#include <vector>
#include <chrono>
#include <thread>

#include "threadpool.hpp"

int main() {
    ThreadPool pool(2);

    std::vector<std::future<int>> futures;

    futures.push_back(pool.push([]{ std::this_thread::sleep_for(std::chrono::milliseconds(1500)); return 0; }));
    futures.push_back(pool.push([]{ std::this_thread::sleep_for(std::chrono::milliseconds(1000)); return 1; }));

    pool.wait();
    std::cout << "done waiting once" << std::endl;

    futures.push_back(pool.push([]{ std::this_thread::sleep_for(std::chrono::milliseconds(1000)); return 2; }));
    futures.push_back(pool.push([]{ std::this_thread::sleep_for(std::chrono::milliseconds(3000)); return 3; }));
    
    pool.wait();
    std::cout << "done waiting twice" << std::endl;

    for (auto& f : futures) {
        std::cout << f.get() << std::endl;
    }

    pool.join();
}
