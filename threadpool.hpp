#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>


//template <typename T>
class Task {
public:
    //Task(const std::function<T>& function) {

    //}
private:
    //std::function<T>
};


class Worker {
public:
    Worker(std::queue<Task>& a_tasks, std::mutex& a_mutex, std::condition_variable& a_cond)
        : m_tasks(a_tasks), m_mutex(a_mutex), m_cond(a_cond), m_stop(false), m_thread([this](){ run(); }) {}
    
    Worker(Worker&& rhs)
        : m_tasks(rhs.m_tasks), m_mutex(rhs.m_mutex), m_cond(rhs.m_cond), m_stop(rhs.m_stop), m_thread(std::move(rhs.m_thread)) {}

    void join() {
        m_stop = true;
        m_thread.join();
    }

    ~Worker() {
        join();
    }

private:
    Task pop_task() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [&]{ return !m_tasks.empty() || m_stop; });
        if (m_stop && m_tasks.empty()) {
            lock.unlock();
            m_cond.notify_one();
            return Task();  // TODO
        } else {
            Task task = std::move(m_tasks.front());
            m_tasks.pop();
            lock.unlock();
            m_cond.notify_one();
            return std::move(task);
        }
    }

    void run() {
        while (!m_stop) {
            Task task = pop_task();
            /* Invoke task */
        }
    }

    std::queue<Task>& m_tasks;
    std::mutex& m_mutex;  // TODO: bake into queue (create thread-safe queue)
    std::condition_variable& m_cond;
    bool m_stop;
    std::thread m_thread;
};


class ThreadPool {
public:
    explicit ThreadPool(size_t n_threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < n_threads; ++i) {
            m_workers.emplace_back(m_tasks, m_mutex, m_cond);
        }
    }

    void join() {
        for (Worker& w : m_workers) {
            w.join();
        }
    }

private:
    std::queue<Task> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::vector<Worker> m_workers;
};
