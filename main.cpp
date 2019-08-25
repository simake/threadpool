#include <iostream>

#include "threadpool.hpp"

int main() {
    ThreadPool pool(2);

    for (int i = 0; i < 10; ++i) {
        pool.push([i]{ std::cout << i << std::endl; });
    }

    pool.join();
}
