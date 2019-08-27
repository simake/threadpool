#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>

class ThreadPool {
public:
    explicit ThreadPool(size_t n_threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < n_threads; ++i) {
            m_threads.emplace_back([this]{ work(); });
        }
    }

    template<class F, class ...Args>
    auto push(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));
        auto f_bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task_ptr = new std::packaged_task<return_type()>(std::move(f_bound));
        auto future = task_ptr->get_future();
        // std::function requires a CopyConstructible Callable, which std::packaged_task is not.
        // It's possible to solve this using move capture from C++14. But to keep C++11 compatibility,
        // here's a workaround using a pointer, which is CopyConstructible.
        std::function<void()> void_wrapper = [task_ptr]{ (*task_ptr)(); delete task_ptr; };
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
