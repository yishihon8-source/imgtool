#include "HistoryPerformanceTest.h"
#include "utils/Logger.h"
#include <sstream>
#include <iomanip>

ImageData HistoryPerformanceTest::CreateTestImage(int width, int height) {
    ImageData image;
    image.width = width;
    image.height = height;
    image.channels = 4;
    image.pixels.resize(width * height * 4);
    
    // 填充测试数据
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 4;
            image.pixels[index + 0] = static_cast<uint8_t>((x * 255) / width);
            image.pixels[index + 1] = static_cast<uint8_t>((y * 255) / height);
            image.pixels[index + 2] = 128;
            image.pixels[index + 3] = 255;
        }
    }
    
    return image;
}

double HistoryPerformanceTest::MeasureTime(std::function<void()> func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

HistoryPerformanceTest::TestResult HistoryPerformanceTest::RunTest(
    int imageWidth, int imageHeight,
    int selectionWidth, int selectionHeight,
    int operationCount) {
    
    TestResult result;
    result.imageWidth = imageWidth;
    result.imageHeight = imageHeight;
    result.selectionWidth = selectionWidth;
    result.selectionHeight = selectionHeight;
    result.operationCount = operationCount;
    
    Logger::Info("=== Performance Test Start ===");
    Logger::Info("Image: " + std::to_string(imageWidth) + "x" + std::to_string(imageHeight));
    Logger::Info("Selection: " + std::to_string(selectionWidth) + "x" + std::to_string(selectionHeight));
    Logger::Info("Operations: " + std::to_string(operationCount));
    
    // 创建测试图像
    ImageData testImage = CreateTestImage(imageWidth, imageHeight);
    
    // ========== 测试旧系统（ImageHistory）==========
    Logger::Info("\n[Old System] Testing ImageHistory (Full Bitmap)...");
    
    ImageHistory oldSystem(operationCount + 10);
    
    // 测试 Push
    result.oldSystemPushTime = MeasureTime([&]() {
        for (int i = 0; i < operationCount; ++i) {
            oldSystem.Push(testImage, "Delete Selection " + std::to_string(i));
        }
    });
    
    // 测试 Undo
    result.oldSystemUndoTime = MeasureTime([&]() {
        for (int i = 0; i < operationCount; ++i) {
            ImageData outData;
            std::string outDesc;
            oldSystem.Undo(outData, outDesc);
        }
    });
    
    // 测试 Redo
    result.oldSystemRedoTime = MeasureTime([&]() {
        for (int i = 0; i < operationCount; ++i) {
            ImageData outData;
            std::string outDesc;
            oldSystem.Redo(outData, outDesc);
        }
    });
    
    // 估算内存占用
    result.oldSystemMemoryUsed = oldSystem.GetHistoryCount() * testImage.GetSize();
    
    Logger::Info("[Old System] Push time: " + std::to_string(result.oldSystemPushTime) + "ms");
    Logger::Info("[Old System] Undo time: " + std::to_string(result.oldSystemUndoTime) + "ms");
    Logger::Info("[Old System] Redo time: " + std::to_string(result.oldSystemRedoTime) + "ms");
    Logger::Info("[Old System] Memory: " + std::to_string(result.oldSystemMemoryUsed / 1024 / 1024) + "MB");
    
    // ========== 测试新系统（TileImageHistory）==========
    Logger::Info("\n[New System] Testing TileImageHistory (Tile-based)...");
    
    TileImageHistory newSystem(operationCount + 10);
    newSystem.CreateFromImageData(testImage);
    
    // 测试 Push（通过 DeleteSelection）
    result.newSystemPushTime = MeasureTime([&]() {
        for (int i = 0; i < operationCount; ++i) {
            newSystem.BeginEdit("Delete Selection " + std::to_string(i));
            newSystem.DeleteSelection(i * 10, i * 10, selectionWidth, selectionHeight);
            newSystem.EndEdit();
        }
    });
    
    // 测试 Undo
    result.newSystemUndoTime = MeasureTime([&]() {
        for (int i = 0; i < operationCount; ++i) {
            newSystem.Undo();
        }
    });
    
    // 测试 Redo
    result.newSystemRedoTime = MeasureTime([&]() {
        for (int i = 0; i < operationCount; ++i) {
            newSystem.Redo();
        }
    });
    
    // 获取内存占用
    auto stats = newSystem.GetPerformanceStats();
    result.newSystemMemoryUsed = stats.totalMemoryUsed;
    
    Logger::Info("[New System] Push time: " + std::to_string(result.newSystemPushTime) + "ms");
    Logger::Info("[New System] Undo time: " + std::to_string(result.newSystemUndoTime) + "ms");
    Logger::Info("[New System] Redo time: " + std::to_string(result.newSystemRedoTime) + "ms");
    Logger::Info("[New System] Memory: " + std::to_string(result.newSystemMemoryUsed / 1024 / 1024) + "MB");
    
    // ========== 计算性能提升 ==========
    result.pushSpeedup = result.oldSystemPushTime / result.newSystemPushTime;
    result.undoSpeedup = result.oldSystemUndoTime / result.newSystemUndoTime;
    result.redoSpeedup = result.oldSystemRedoTime / result.newSystemRedoTime;
    result.memoryReduction = (1.0 - static_cast<double>(result.newSystemMemoryUsed) / result.oldSystemMemoryUsed) * 100.0;
    
    Logger::Info("\n=== Performance Improvement ===");
    Logger::Info("Push speedup: " + std::to_string(result.pushSpeedup) + "x");
    Logger::Info("Undo speedup: " + std::to_string(result.undoSpeedup) + "x");
    Logger::Info("Redo speedup: " + std::to_string(result.redoSpeedup) + "x");
    Logger::Info("Memory reduction: " + std::to_string(result.memoryReduction) + "%");
    Logger::Info("=== Performance Test End ===\n");
    
    return result;
}

std::string HistoryPerformanceTest::TestResult::ToString() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    
    ss << "=== Performance Test Result ===\n";
    ss << "Image: " << imageWidth << "x" << imageHeight << "\n";
    ss << "Selection: " << selectionWidth << "x" << selectionHeight << "\n";
    ss << "Operations: " << operationCount << "\n\n";
    
    ss << "Old System (Full Bitmap):\n";
    ss << "  Push: " << oldSystemPushTime << "ms\n";
    ss << "  Undo: " << oldSystemUndoTime << "ms\n";
    ss << "  Redo: " << oldSystemRedoTime << "ms\n";
    ss << "  Memory: " << (oldSystemMemoryUsed / 1024.0 / 1024.0) << "MB\n\n";
    
    ss << "New System (Tile-based):\n";
    ss << "  Push: " << newSystemPushTime << "ms\n";
    ss << "  Undo: " << newSystemUndoTime << "ms\n";
    ss << "  Redo: " << newSystemRedoTime << "ms\n";
    ss << "  Memory: " << (newSystemMemoryUsed / 1024.0 / 1024.0) << "MB\n\n";
    
    ss << "Performance Improvement:\n";
    ss << "  Push speedup: " << pushSpeedup << "x\n";
    ss << "  Undo speedup: " << undoSpeedup << "x\n";
    ss << "  Redo speedup: " << redoSpeedup << "x\n";
    ss << "  Memory reduction: " << memoryReduction << "%\n";
    
    return ss.str();
}

void HistoryPerformanceTest::RunStandardTests() {
    Logger::Info("\n");
    Logger::Info("╔════════════════════════════════════════════════════════════╗");
    Logger::Info("║  Tile-based History System Performance Test Suite         ║");
    Logger::Info("╚════════════════════════════════════════════════════════════╝");
    Logger::Info("\n");
    
    // 测试 1: 小图像，小选区
    Logger::Info("Test 1: Small Image (1024x1024), Small Selection (100x100)");
    auto result1 = RunTest(1024, 1024, 100, 100, 10);
    
    // 测试 2: 中等图像，中等选区
    Logger::Info("\nTest 2: Medium Image (2048x2048), Medium Selection (200x200)");
    auto result2 = RunTest(2048, 2048, 200, 200, 10);
    
    // 测试 3: 大图像，小选区（最佳场景）
    Logger::Info("\nTest 3: Large Image (4096x4096), Small Selection (100x100)");
    auto result3 = RunTest(4096, 4096, 100, 100, 10);
    
    // 测试 4: 大图像，大选区
    Logger::Info("\nTest 4: Large Image (4096x4096), Large Selection (1000x1000)");
    auto result4 = RunTest(4096, 4096, 1000, 1000, 10);
    
    // 汇总报告
    Logger::Info("\n");
    Logger::Info("╔════════════════════════════════════════════════════════════╗");
    Logger::Info("║  Performance Test Summary                                  ║");
    Logger::Info("╚════════════════════════════════════════════════════════════╝");
    Logger::Info("\n");
    
    Logger::Info(result1.ToString());
    Logger::Info("\n" + result2.ToString());
    Logger::Info("\n" + result3.ToString());
    Logger::Info("\n" + result4.ToString());
    
    // 平均性能提升
    double avgPushSpeedup = (result1.pushSpeedup + result2.pushSpeedup + 
                             result3.pushSpeedup + result4.pushSpeedup) / 4.0;
    double avgUndoSpeedup = (result1.undoSpeedup + result2.undoSpeedup + 
                             result3.undoSpeedup + result4.undoSpeedup) / 4.0;
    double avgMemoryReduction = (result1.memoryReduction + result2.memoryReduction + 
                                 result3.memoryReduction + result4.memoryReduction) / 4.0;
    
    Logger::Info("\n");
    Logger::Info("╔════════════════════════════════════════════════════════════╗");
    Logger::Info("║  Average Performance Improvement                           ║");
    Logger::Info("╚════════════════════════════════════════════════════════════╝");
    Logger::Info("Push speedup: " + std::to_string(avgPushSpeedup) + "x");
    Logger::Info("Undo speedup: " + std::to_string(avgUndoSpeedup) + "x");
    Logger::Info("Memory reduction: " + std::to_string(avgMemoryReduction) + "%");
    Logger::Info("\n");
}

