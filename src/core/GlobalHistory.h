#pragma once

#include "Types.h"
#include <vector>
#include <string>
#include <functional>

/**
 * @brief 全局操作历史记录项
 * 
 * 保存操作描述和恢复状态的回调函数
 */
struct GlobalHistoryEntry {
    std::string description;                    // 操作描述
    std::function<void()> restoreCallback;      // 恢复状态的回调函数
    
    GlobalHistoryEntry() = default;
    GlobalHistoryEntry(const std::string& desc, std::function<void()> callback)
        : description(desc), restoreCallback(callback) {}
};

/**
 * @brief 全局操作历史管理器
 * 
 * 职责：
 * - 管理全局操作的历史记录（添加文件、应用画布等）
 * - 支持点击历史记录恢复到对应状态
 * - 类似 Photoshop 的历史记录面板
 */
class GlobalHistory {
public:
    /**
     * @brief 构造函数
     * @param maxHistorySize 最大历史记录数量（默认 50）
     */
    explicit GlobalHistory(size_t maxHistorySize = 50);

    /**
     * @brief 添加历史记录
     * @param description 操作描述
     * @param restoreCallback 恢复状态的回调函数
     */
    void Push(const std::string& description, std::function<void()> restoreCallback);

    /**
     * @brief 恢复到指定索引的历史状态
     * @param index 历史记录索引
     * @return 成功返回 true，失败返回 false
     */
    bool RestoreTo(int index);

    /**
     * @brief 获取历史记录数量
     */
    size_t GetHistoryCount() const;

    /**
     * @brief 获取当前位置索引
     */
    int GetCurrentIndex() const;

    /**
     * @brief 获取指定索引的描述
     */
    std::string GetDescription(int index) const;

    /**
     * @brief 清空所有历史记录
     */
    void Clear();

private:
    std::vector<GlobalHistoryEntry> m_History;  // 历史记录栈
    int m_CurrentIndex;                         // 当前位置（-1 表示在最开始）
    size_t m_MaxHistorySize;                    // 最大历史记录数量
};

