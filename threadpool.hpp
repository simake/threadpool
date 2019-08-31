#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <algorithm>

class ThreadPool {
public:
    /**
     * Creates a thread pool with a set number of worker threads.
     * Defaults to the number of threads supported by the system (or 1 if it's not computable).
     * 
     * Note: Throws std::system_error if the threads could not be started.
     */
    explicit ThreadPool(size_t num_threads = std::max(std::thread::hardware_concurrency(), 1u)) {
        if (num_threads == 0) {
            throw std::invalid_argument("Creating ThreadPool with 0 threads");
        }
        m_threads.reserve(num_threads);
        std::lock_guard<std::mutex> lock(m_mutex);
        try {
            for (size_t i = 0; i < num_threads; ++i) {
                m_threads.emplace_back([this]{ work(); });
            }
        } catch (std::system_error& e) {
            stop();
            throw;
        }
    }

    /**
     * Pushes a new task to the processing queue and returns a future used to access the return value.
     * The task will be invoked when there is an available worker thread and any previously
     * pushed tasks have been or are being handled by a worker thread.
     * 
     * Example usage:
     *  auto future = pool.push([](int x){ return x*2; }, 42);
     *  std::cout << future.get() << std::endl;  // 84
     * 
     * Note: Reference arguments need to be wrapped with std::ref.
     */
    template<class F, class ...Args>
    auto push(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));
        auto f_bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<return_type()>>(std::move(f_bound));
        auto future = task_ptr->get_future();
        // std::function requires a CopyConstructible Callable, which std::packaged_task is not.
        // Here's a workaround using std::shared_ptr, which is CopyConstructible.
        std::function<void()> void_wrapper = [task_ptr]{ (*task_ptr)(); };
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_stop) {
            throw std::runtime_error("Pushing new task to stopped ThreadPool");
        }
        m_tasks.push(std::move(void_wrapper));
        lock.unlock();
        m_task_cv.notify_one();
        return future;
    }

    /**
     * Blocks until all pushed tasks have been completed. Threads are kept alive.
     */
    void wait() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_idle_cv.wait(lock, [this]{ return m_tasks.empty() && m_idle_count == m_threads.size(); });
    }

    /**
     * Threads are automatically joined upon destruction of the thread pool.
     */
    ~ThreadPool() {
        stop();
    }

private:
    /**
     * Initiates thread pool shutdown. Worker threads are joined when
     * they are done processing the remaining tasks in the queue.
     */
    void stop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
        lock.unlock();
        m_task_cv.notify_all();
        for (std::thread& t : m_threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    /**
     * The worker thread loop. Pops and invokes tasks from the processing queue until signaled to stop.
     */
    void work() {
        while (true) {
            std::unique_lock<std::mutex> lock(m_mutex);
            
            if (++m_idle_count == m_threads.size()) {
                m_idle_cv.notify_one();
            }
            m_task_cv.wait(lock, [this]{ return !m_tasks.empty() || m_stop; });
            --m_idle_count;

            if (m_stop && m_tasks.empty()) {
                lock.unlock();
                return;
            }
        
            auto task = std::move(m_tasks.front());
            m_tasks.pop();
            lock.unlock();
            m_task_cv.notify_one();
            task();
        }
    }

    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_task_cv;  // Synchronizes the scheduling and acquisition of tasks
    std::condition_variable m_idle_cv;  // Used to signal the wait method when all threads are idle
    size_t m_idle_count = 0;
    bool m_stop = false;
    std::vector<std::thread> m_threads;
};
