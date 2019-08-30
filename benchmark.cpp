#include <iostream>
#include <future>
#include <vector>
#include <chrono>
#include <thread>

#include "threadpool.hpp"

const int num_tasks = 100;
const int num_runs = 10;

int benchmark(std::function<void()> f) {
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < num_runs; ++i) {
        f();
    }
    auto t1 = std::chrono::steady_clock::now();
    auto total = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    auto avg = int(double(total) / num_runs);
    return avg;
}

void task() {
    // empty to benchmark the thread creation overhead
}

int main() {
    std::cout << "tasks: " << num_tasks << std::endl;

    // ThreadPool
    int avg = benchmark([]{
        ThreadPool pool(2);
        for (int j = 0; j < num_tasks; ++j) {
            pool.push(task);
        }
        pool.stop();
    });
    std::cout << "ThreadPool (avg):\t\t\t" << avg << " μs" << std::endl;

    // std::thread
    avg = benchmark([]{
        std::vector<std::thread> threads;
        threads.reserve(num_tasks);
        for (int j = 0; j < num_tasks; ++j) {
            threads.emplace_back(task);
        }
        for (auto& t : threads) {
            t.join();
        }
    });
    std::cout << "std::thread (avg):\t\t\t" << avg << " μs" << std::endl;

    // std::async
    avg = benchmark([]{
        std::vector<std::future<void>> futures;
        futures.reserve(num_tasks);
        for (int j = 0; j < num_tasks; ++j) {
            futures.emplace_back(std::async(task));
        }
        for (auto& f : futures) {
            f.get();
        }
    });
    std::cout << "std::async default policy (avg):\t" << avg << " μs" << std::endl;

    // std::async, std::launch::async
    avg = benchmark([]{
        std::vector<std::future<void>> futures;
        futures.reserve(num_tasks);
        for (int j = 0; j < num_tasks; ++j) {
            futures.emplace_back(std::async(std::launch::async, task));
        }
        for (auto& f : futures) {
            f.get();
        }
    });
    std::cout << "std::async async policy (avg):\t\t" << avg << " μs" << std::endl;
}
