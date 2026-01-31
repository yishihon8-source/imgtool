#include "app/App.h"
#include "utils/Logger.h"
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

// 主程序逻辑
int RunApplication() {
    try {
        // 初始化日志系统
        Logger::Initialize("ImageBatchTool.log");
        Logger::Info("=== Application Startup ===");
        Logger::Info("Starting ImageBatchTool...");
        
        try {
            Logger::Info("Creating application instance...");
            App app;
            
            Logger::Info("Initializing application...");
            if (!app.Initialize()) {
                Logger::Error("Failed to initialize application");
                Logger::Shutdown();
                return -1;
            }
            
            Logger::Info("Application initialized successfully");
            Logger::Info("Starting main loop...");
            app.Run();
            
            Logger::Info("Main loop exited");
            app.Shutdown();
            Logger::Info("Application shutdown completed");
            
            Logger::Shutdown();
            return 0;
        } catch (const std::exception& e) {
            Logger::Error(std::string("Exception in main: ") + e.what());
            Logger::Shutdown();
            return -1;
        } catch (...) {
            Logger::Error("Unknown exception in main");
            Logger::Shutdown();
            return -1;
        }
    }
    catch (...) {
        return -1;
    }
}

#ifdef _WIN32
// Windows GUI 应用程序入口点
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 避免未使用参数警告
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    
    return RunApplication();
}
#endif

// 标准控制台应用程序入口点（用于调试）
int main(int argc, char** argv) {
    // 避免未使用参数警告
    (void)argc;
    (void)argv;
    
    return RunApplication();
}
