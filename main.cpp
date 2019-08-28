#include <iostream>
#include <future>
#include <vector>

#include "threadpool.hpp"

int main() {
    ThreadPool pool(2);

    std::vector<std::future<int>> futures;

    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool.push([i]{ return i; }));
    }

    for (auto& f : futures) {
        std::cout << f.get() << std::endl;
    }

    pool.join();
}
