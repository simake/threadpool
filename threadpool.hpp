#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>
#include <future>

class ThreadPool {
public:
    explicit ThreadPool(size_t n_threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < n_threads; ++i) {
            m_threads.emplace_back([this]{ work(); });
        }
    }

    std::future<int> push(std::function<int()> task) {
        std::packaged_task<int()> packaged_task(task);
        std::future<int> future = packaged_task.get_future();
        std::function<void()> void_wrapper = std::bind(std::move(packaged_task));
        std::unique_lock<std::mutex> lock(m_mutex);
        m_tasks.push(std::move(void_wrapper));
        lock.unlock();
        m_cond.notify_one();
        return future;
    }

    void join() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
        lock.unlock();
        m_cond.notify_all();
        for (std::thread& t : m_threads) {
            t.join();
        }
    }

private:
    void work() {
        while (true) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock, [this]{ return !m_tasks.empty() || m_stop; });
            
            if (m_stop && m_tasks.empty()) {
                lock.unlock();
                return;
            }
        
            auto task = std::move(m_tasks.front());
            m_tasks.pop();
            lock.unlock();
            m_cond.notify_one();
            task();
        }
    }

    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_stop = false;
    std::vector<std::thread> m_threads;
};
