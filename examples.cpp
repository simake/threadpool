#include <cstdio>
#include <functional>
#include <chrono>
#include <thread>
#include <stdexcept>

#include "threadpool.hpp"

void hello(const char* s) {
    printf("Hello, %s!\n", s);
}

void example_hello() {
    // Create a pool with 4 threads
    ThreadPool pool(4);
    // push is templated and takes any callable
    pool.push(hello, "good old function");
    pool.push(std::bind(hello, "bind function object"));
    pool.push([]{ printf("Hello, lambda!\n"); });
    // Pool automatically waits for tasks to finish before destructing
}

void example_future() {
    // Number of threads defaults to std::thread::hardware_concurrency()
    ThreadPool pool;
    // Function with a return value (int)
    auto multiply_by_2 = [](int x) -> int { return x*2; };
    // push returns a future that can be used to retrieve the return value
    auto future = pool.push(multiply_by_2, 5);
    // future.get() blocks until the task is completed and returns the result
    printf("multiply_by_2(5) = %d\n", future.get());
}

void example_exception() {
    ThreadPool pool;
    auto throwing_function = [](){ throw std::exception(); };
    auto future = pool.push(throwing_function);
    // Exceptions thrown by a task can be caught using its future
    try {
        future.get();
    } catch (const std::exception& e) {
        printf("Exception caught!\n");
    }
}

void sleep(unsigned int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void example_baking() {
    // Here's a baking recipe which we'll try to make efficient by including the whole family
    auto add_flour = []{ sleep(200); printf("*puff*\n"); };
    auto add_eggs = []{ sleep(300); printf("*crack*\n"); };
    auto add_milk = []{ sleep(100); printf("*splash*\n"); };
    auto stir_ingredients = []{ printf("*stir stir stir*\n"); sleep(600); };
    auto shape_batter = []{ printf("*shaping some batter into a pastry*\n"); sleep(500); };  // x10
    auto bake_pastries = []{ printf("*baking pastries in oven*\n"); sleep(600); };
    // Baking is a family acitiviy!
    const int family_size = 5;  // Yes, const. We'll be careful.
    ThreadPool pool(family_size);
    // Adding the ingredients can be done in parallel
    pool.push(add_flour);
    pool.push(add_eggs);
    pool.push(add_milk);
    // But we have to finish adding the ingredients before stirring (suspend your disbelief)
    pool.wait();
    pool.push(stir_ingredients);
    // Once done stirring the whole family will be at hard work making pastries
    pool.wait();
    const int number_of_pastries = 10;
    for (int i = 0; i < number_of_pastries; ++i) {
        pool.push(shape_batter);
    }
    // Finally bake them
    pool.wait();
    pool.push(bake_pastries);
    pool.wait();
    printf("Done!\n");
    // disclaimer: I'm no cook
}

void example_future_sync() {
    // synchronization between tasks with dependencies can also be accomplished using futures
}

void example_inception() {
    // meet threadpool-ception
}

int main() {
    printf("----------- example_hello -----------\n");
    example_hello();
    printf("----------- example_future -----------\n");
    example_future();
    printf("----------- example_exception -----------\n");
    example_exception();
}
