#pragma once

#include <string>
#include <vector>

/**
 * @brief 文件对话框工具
 * 
 * 职责：
 * - 打开文件选择对话框
 * - 打开文件夹选择对话框
 * - 保存文件对话框
 * 
 * 注意：Windows 平台使用原生 API
 */
class FileDialog {
public:
    /**
     * @brief 打开文件选择对话框（单选）
     * @param filter 文件过滤器（例如："Image Files\0*.jpg;*.png\0"）
     * @return 选中的文件路径，取消返回空字符串
     */
    static std::string OpenFile(const char* filter = nullptr);

    /**
     * @brief 打开文件选择对话框（多选）
     * @param filter 文件过滤器
     * @return 选中的文件路径列表
     */
    static std::vector<std::string> OpenFiles(const char* filter = nullptr);

    /**
     * @brief 打开文件夹选择对话框
     * @return 选中的文件夹路径，取消返回空字符串
     */
    static std::string OpenFolder();

    /**
     * @brief 保存文件对话框
     * @param filter 文件过滤器
     * @param defaultExt 默认扩展名
     * @return 保存路径，取消返回空字符串
     */
    static std::string SaveFile(const char* filter = nullptr, const char* defaultExt = nullptr);

    /**
     * @brief 在文件资源管理器中打开文件夹
     * @param folderPath 文件夹路径（为空则打开"我的电脑"）
     */
    static void OpenInExplorer(const std::string& folderPath = "");

private:
    static const char* GetImageFilter();
};
