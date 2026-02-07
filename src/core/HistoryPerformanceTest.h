#pragma once

#include "core/ImageHistory.h"
#include "core/TileImageHistory.h"
#include "core/Types.h"
#include <string>
#include <chrono>
#include <functional>

/**
 * @brief 历史记录系统性能对比工具
 * 
 * 对比 ImageHistory（完整 Bitmap）vs TileImageHistory（Tile-based）
 */
class HistoryPerformanceTest {
public:
    struct TestResult {
        // 测试配置
        int imageWidth;
        int imageHeight;
        int selectionWidth;
        int selectionHeight;
        int operationCount;
        
        // ImageHistory（旧系统）结果
        double oldSystemPushTime;      // Push 耗时（毫秒）
        double oldSystemUndoTime;      // Undo 耗时（毫秒）
        double oldSystemRedoTime;      // Redo 耗时（毫秒）
        size_t oldSystemMemoryUsed;    // 内存占用（字节）
        
        // TileImageHistory（新系统）结果
        double newSystemPushTime;
        double newSystemUndoTime;
        double newSystemRedoTime;
        size_t newSystemMemoryUsed;
        
        // 性能提升倍数
        double pushSpeedup;
        double undoSpeedup;
        double redoSpeedup;
        double memoryReduction;
        
        std::string ToString() const;
    };
    
    /**
     * @brief 运行性能对比测试
     * @param imageWidth 图像宽度
     * @param imageHeight 图像高度
     * @param selectionWidth 选区宽度
     * @param selectionHeight 选区高度
     * @param operationCount 操作次数
     */
    static TestResult RunTest(int imageWidth, int imageHeight,
                             int selectionWidth, int selectionHeight,
                             int operationCount);
    
    /**
     * @brief 运行标准测试套件
     */
    static void RunStandardTests();

private:
    static ImageData CreateTestImage(int width, int height);
    static double MeasureTime(std::function<void()> func);
};

