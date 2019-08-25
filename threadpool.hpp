#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>

class ThreadPool {
public:
    using task_type = std::function<void()>;

    explicit ThreadPool(size_t n_threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < n_threads; ++i) {
            m_threads.emplace_back([this]{ work(); });
        }
    }

    void push(task_type task) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push(task);
    }

    void join() {
        m_stop = true;
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
                m_cond.notify_one();
                return;
            }
        
            task_type task = std::move(m_tasks.front());
            m_tasks.pop();
            lock.unlock();
            m_cond.notify_one();
            task();
        }
    }

    std::queue<task_type> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_stop = false;
    std::vector<std::thread> m_threads;
};
