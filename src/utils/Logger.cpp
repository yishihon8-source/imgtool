#include "Logger.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iomanip>

// 全局日志文件流
static std::ofstream g_logFile;
static bool g_initialized = false;

void Logger::Initialize(const std::string& logPath) {
    try {
        // 禁用日志文件生成，只输出到控制台
        (void)logPath;  // 避免未使用参数警告
        g_initialized = false;
        
        // 只在控制台输出启动信息
        std::cout << "========== Application Started ==========" << std::endl;
        std::cout << "Log file generation disabled (no file will be created)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception initializing logger: " << e.what() << std::endl;
    }
}

void Logger::Shutdown() {
    try {
        if (g_logFile.is_open()) {
            Log("========== Application Closing ==========");
            g_logFile.close();
            g_initialized = false;
        }
    } catch (...) {
    }
}

std::string Logger::GetTimestamp() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Logger::Log(const std::string& message) {
    try {
        std::string logMsg = "[" + GetTimestamp() + "] " + message;
        
        // 输出到控制台
        std::cout << logMsg << std::endl;
        std::cout.flush();
        
        // 输出到日志文件
        if (g_initialized && g_logFile.is_open()) {
            g_logFile << logMsg << std::endl;
            g_logFile.flush();
        }
    } catch (...) {
        // 最后的防线：如果日志系统本身出错，至少输出到控制台
        std::cerr << message << std::endl;
    }
}

void Logger::Error(const std::string& message) {
    Log("ERROR: " + message);
}

void Logger::Warning(const std::string& message) {
    Log("WARNING: " + message);
}

void Logger::Info(const std::string& message) {
    Log("INFO: " + message);
}

void Logger::Debug(const std::string& message) {
    Log("DEBUG: " + message);
}
