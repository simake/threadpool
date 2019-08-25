#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>

#include <iostream>  // temp for debugging


//template <typename T>
class Task {
public:
    void invoke() {
        std::cout << "* Running task *" << std::endl;
    }
};


class Worker {
public:
    Worker(std::queue<Task>& a_tasks, std::mutex& a_mutex, std::condition_variable& a_cond, const bool& a_stop)
        : m_tasks(a_tasks), m_mutex(a_mutex), m_cond(a_cond), m_stop(a_stop), m_thread([this]{ run(); }) {}
    
    Worker(Worker&& rhs)
        : m_tasks(rhs.m_tasks), m_mutex(rhs.m_mutex), m_cond(rhs.m_cond), m_stop(rhs.m_stop), m_thread(std::move(rhs.m_thread)) {}

    void join() {
        m_thread.join();
    }

    ~Worker() {
        if (m_thread.joinable()) {
            join();
        }
    }

private:
    void run() {
        while (true) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock, [this]{ return !m_tasks.empty() || m_stop; });
            
            if (m_stop && m_tasks.empty()) {
                lock.unlock();
                m_cond.notify_one();
                return;
            }
        
            Task task = std::move(m_tasks.front());
            m_tasks.pop();
            lock.unlock();
            m_cond.notify_one();
            task.invoke();
        }
    }

    std::queue<Task>& m_tasks;
    std::mutex& m_mutex;
    std::condition_variable& m_cond;
    const bool& m_stop;
    std::thread m_thread;
};


class ThreadPool {
public:
    explicit ThreadPool(size_t n_threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < n_threads; ++i) {
            m_workers.emplace_back(m_tasks, m_mutex, m_cond, m_stop);
        }
    }

    void push(Task task) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push(task);
    }

    void join() {
        m_stop = true;
        m_cond.notify_all();
        for (Worker& w : m_workers) {
            w.join();
        }
    }

private:
    std::queue<Task> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_stop = false;
    std::vector<Worker> m_workers;
};
