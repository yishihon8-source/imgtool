#pragma once

#include "ThreadPool.h"
#include "core/Types.h"
#include <string>
#include <vector>
#include <atomic>
#include <functional>

/**
 * @brief 批量处理任务
 */
struct BatchTask {
    std::string inputPath;
    std::string outputPath;
    ProcessConfig config;
    ImageTransformState transformState;  // 用户的变换状态
};

/**
 * @brief 批量处理进度
 */
struct BatchProgress {
    size_t total = 0;
    std::atomic<size_t> completed{0};
    std::atomic<size_t> failed{0};
    std::atomic<bool> running{false};

    float GetProgress() const {
        return total > 0 ? static_cast<float>(completed) / total : 0.0f;
    }
};

/**
 * @brief 批量处理器
 * 
 * 职责：
 * - 管理批量处理任务
 * - 进度跟踪
 * - 错误处理
 */
class BatchProcessor {
public:
    using ProgressCallback = std::function<void(const BatchProgress&)>;
    using CompletionCallback = std::function<void(bool success)>;

    BatchProcessor();
    ~BatchProcessor();

    /**
     * @brief 开始批量处理
     * @param tasks 任务列表
     * @param onProgress 进度回调
     * @param onComplete 完成回调
     */
    void Start(const std::vector<BatchTask>& tasks,
               ProgressCallback onProgress = nullptr,
               CompletionCallback onComplete = nullptr);

    /**
     * @brief 停止批量处理
     */
    void Stop();

    /**
     * @brief 获取当前进度
     */
    const BatchProgress& GetProgress() const { return m_Progress; }

    /**
     * @brief 是否正在运行
     */
    bool IsRunning() const { return m_Progress.running; }

private:
    /**
     * @brief 处理单个任务
     */
    bool ProcessTask(const BatchTask& task);

    /**
     * @brief 监控线程
     */
    void MonitorThread();

private:
    ThreadPool m_ThreadPool;
    BatchProgress m_Progress;
    
    ProgressCallback m_OnProgress;
    CompletionCallback m_OnComplete;
    
    std::thread m_MonitorThread;
    std::atomic<bool> m_StopMonitor{false};
};
