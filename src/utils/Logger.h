#pragma once

#include <iostream>
#include <sstream>
#include <string>

/**
 * @brief 日志工具 - 支持文件输出
 * 
 * 职责：
 * - 统一的日志输出接口
 * - 支持文件和控制台输出
 * - 用于诊断程序崩溃原因
 */
class Logger {
public:
    // 初始化日志系统，指定日志文件路径
    static void Initialize(const std::string& logPath);
    
    // 关闭日志系统
    static void Shutdown();
    
    // 通用日志方法
    static void Log(const std::string& message);
    static void Error(const std::string& message);
    static void Warning(const std::string& message);
    static void Info(const std::string& message);
    static void Debug(const std::string& message);

private:
    static std::string GetTimestamp();
};
