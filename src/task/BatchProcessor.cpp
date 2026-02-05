#include "BatchProcessor.h"
#include "core/ImageLoader.h"
#include "core/ImageProcessor.h"
#include <chrono>
#include <iostream>

BatchProcessor::BatchProcessor() = default;

BatchProcessor::~BatchProcessor() {
    Stop();
}

void BatchProcessor::Start(const std::vector<BatchTask>& tasks,
                            ProgressCallback onProgress,
                            CompletionCallback onComplete) {
    if (m_Progress.running) {
        std::cerr << "Batch processing already running" << std::endl;
        return;
    }

    // 确保上一次的监控线程已经结束
    if (m_MonitorThread.joinable()) {
        m_StopMonitor = true;
        m_MonitorThread.join();
    }

    // 重置进度状态
    m_Progress.total = tasks.size();
    m_Progress.completed = 0;
    m_Progress.failed = 0;
    m_Progress.running = true;

    m_OnProgress = onProgress;
    m_OnComplete = onComplete;

    // 提交所有任务到线程池
    for (const auto& task : tasks) {
        m_ThreadPool.Submit([this, task]() {
            bool success = ProcessTask(task);
            if (success) {
                m_Progress.completed++;
            } else {
                m_Progress.failed++;
            }
        });
    }

    // 启动监控线程
    m_StopMonitor = false;
    m_MonitorThread = std::thread(&BatchProcessor::MonitorThread, this);
}

void BatchProcessor::Stop() {
    m_StopMonitor = true;
    if (m_MonitorThread.joinable()) {
        m_MonitorThread.join();
    }
    m_Progress.running = false;
}

bool BatchProcessor::ProcessTask(const BatchTask& task) {
    try {
        // ✅ 优先使用预处理的图片数据（如果有修改，如删除选区）
        ImageData source;
        if (task.usePreprocessed && task.preprocessedImage.IsValid()) {
            std::cout << "Using preprocessed image data for: " << task.inputPath << std::endl;
            source = task.preprocessedImage;
        } else {
            // 从磁盘加载原始图片
            if (!ImageLoader::Load(task.inputPath, source)) {
                std::cerr << "Failed to load: " << task.inputPath << std::endl;
                return false;
            }
        }

        // 处理图片，传递用户的变换状态
        const ImageTransformState* transformPtr = task.transformState.hasTransform ? &task.transformState : nullptr;
        ImageData result = ImageProcessor::Process(source, task.config, transformPtr);
        if (!result.IsValid()) {
            std::cerr << "Failed to process: " << task.inputPath << std::endl;
            return false;
        }

        // 保存图片
        bool saved = false;
        if (task.config.format == OutputFormat::PNG) {
            saved = ImageLoader::SavePNG(task.outputPath, result);
        } else {
            saved = ImageLoader::SaveJPG(task.outputPath, result, task.config.jpgQuality);
        }

        if (!saved) {
            std::cerr << "Failed to save: " << task.outputPath << std::endl;
            return false;
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception processing " << task.inputPath << ": " << e.what() << std::endl;
        return false;
    }
}

void BatchProcessor::MonitorThread() {
    using namespace std::chrono_literals;

    while (!m_StopMonitor) {
        // 检查是否完成
        size_t totalProcessed = m_Progress.completed + m_Progress.failed;
        if (totalProcessed >= m_Progress.total) {
            m_Progress.running = false;

            // 调用完成回调
            if (m_OnComplete) {
                bool success = m_Progress.failed == 0;
                m_OnComplete(success);
            }

            break;
        }

        // 调用进度回调
        if (m_OnProgress) {
            m_OnProgress(m_Progress);
        }

        std::this_thread::sleep_for(100ms);
    }
}
