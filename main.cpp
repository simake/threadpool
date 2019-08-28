#include <iostream>
#include <future>
#include <vector>
#include <chrono>
#include <thread>

#include "threadpool.hpp"

int main() {
    ThreadPool pool(2);

    std::vector<std::future<int>> futures;

    auto t0 = std::chrono::high_resolution_clock::now();
    futures.push_back(pool.push([]{ std::this_thread::sleep_for(std::chrono::milliseconds(1500)); return 0; }));
    futures.push_back(pool.push([]{ std::this_thread::sleep_for(std::chrono::milliseconds(1000)); return 1; }));
    pool.wait();
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << "Wait 1: expected=1500, actual="
              << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
              << std::endl;

    auto t2 = std::chrono::high_resolution_clock::now();
    futures.push_back(pool.push([]{ std::this_thread::sleep_for(std::chrono::milliseconds(500)); return 2; }));
    futures.push_back(pool.push([]{ std::this_thread::sleep_for(std::chrono::milliseconds(3000)); return 3; }));
    pool.wait();
    auto t3 = std::chrono::high_resolution_clock::now();
    std::cout << "Wait 2: expected=3000, actual="
              << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count()
              << std::endl;

    for (auto& f : futures) {
        std::cout << f.get() << std::endl;
    }

    pool.stop();
}
