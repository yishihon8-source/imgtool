#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        m_Threads.emplace_back(&ThreadPool::WorkerThread, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Stop = true;
    }

    m_Condition.notify_all();

    for (auto& thread : m_Threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void ThreadPool::WorkerThread() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Condition.wait(lock, [this] {
                return m_Stop || !m_Tasks.empty();
            });

            if (m_Stop && m_Tasks.empty()) {
                return;
            }

            task = std::move(m_Tasks.front());
            m_Tasks.pop();
        }

        task();
    }
}

size_t ThreadPool::GetPendingTaskCount() const {
    std::unique_lock<std::mutex> lock(m_Mutex);
    return m_Tasks.size();
}
