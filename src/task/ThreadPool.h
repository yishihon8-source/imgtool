#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

/**
 * @brief 线程池
 * 
 * 职责：
 * - 管理工作线程
 * - 任务队列调度
 * - 支持异步任务提交
 */
class ThreadPool {
public:
    /**
     * @brief 构造函数
     * @param numThreads 线程数量（默认为硬件并发数）
     */
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency());

    /**
     * @brief 析构函数（等待所有任务完成）
     */
    ~ThreadPool();

    // 禁止拷贝
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * @brief 提交任务
     * @param func 任务函数
     * @param args 参数
     * @return future 对象
     */
    template<typename Func, typename... Args>
    auto Submit(Func&& func, Args&&... args) 
        -> std::future<typename std::invoke_result<Func, Args...>::type>;

    /**
     * @brief 获取线程数量
     */
    size_t GetThreadCount() const { return m_Threads.size(); }

    /**
     * @brief 获取待处理任务数量
     */
    size_t GetPendingTaskCount() const;

private:
    /**
     * @brief 工作线程函数
     */
    void WorkerThread();

private:
    std::vector<std::thread> m_Threads;
    std::queue<std::function<void()>> m_Tasks;
    
    mutable std::mutex m_Mutex;
    std::condition_variable m_Condition;
    std::atomic<bool> m_Stop{false};
};

// 模板实现
template<typename Func, typename... Args>
auto ThreadPool::Submit(Func&& func, Args&&... args)
    -> std::future<typename std::invoke_result<Func, Args...>::type> {
    
    using ReturnType = typename std::invoke_result<Func, Args...>::type;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
    );

    std::future<ReturnType> result = task->get_future();

    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        if (m_Stop) {
            throw std::runtime_error("Cannot submit task to stopped ThreadPool");
        }
        m_Tasks.emplace([task]() { (*task)(); });
    }

    m_Condition.notify_one();
    return result;
}
