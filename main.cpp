#include "threadpool.hpp"

#include <chrono>
#include <thread>

int main() {
    ThreadPool pool(2);
    Task task;
    Task task1;
    Task task2;
    Task task3;
    pool.push(task);
    // pool.push(task1);
    // pool.push(task2);
    // pool.push(task3);
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    pool.join();
}
