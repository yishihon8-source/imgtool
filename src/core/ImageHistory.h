#pragma once

#include "Types.h"
#include <vector>
#include <memory>

/**
 * @brief 图像历史记录项
 * 
 * 保存图像的完整状态，用于撤销/重做
 */
struct ImageHistoryEntry {
    ImageData imageData;        // 图像数据的完整副本
    std::string description;    // 操作描述（如 "Delete Selection"）
    
    ImageHistoryEntry() = default;
    ImageHistoryEntry(const ImageData& data, const std::string& desc)
        : imageData(data), description(desc) {}
};

/**
 * @brief 图像历史记录管理器
 * 
 * 职责：
 * - 管理图像编辑的历史记录
 * - 支持撤销（Undo）和重做（Redo）
 * - 自动管理内存（限制历史记录数量）
 * 
 * 使用方式：
 * 1. 在执行破坏性操作前调用 Push() 保存当前状态
 * 2. 按 Ctrl+Z 调用 Undo() 撤销
 * 3. 按 Ctrl+Shift+Z 调用 Redo() 重做
 */
class ImageHistory {
public:
    /**
     * @brief 构造函数
     * @param maxHistorySize 最大历史记录数量（默认 20）
     */
    explicit ImageHistory(size_t maxHistorySize = 20);

    /**
     * @brief 保存当前图像状态到历史记录
     * @param imageData 当前图像数据
     * @param description 操作描述
     * 
     * 注意：
     * - 会创建图像数据的完整副本
     * - 如果超过最大历史记录数，会删除最旧的记录
     * - 如果当前不在历史记录末尾，会清除后续的重做记录
     */
    void Push(const ImageData& imageData, const std::string& description);

    /**
     * @brief 撤销到上一个状态
     * @param outImageData 输出：上一个状态的图像数据
     * @param outDescription 输出：操作描述
     * @return 成功返回 true，无法撤销返回 false
     */
    bool Undo(ImageData& outImageData, std::string& outDescription);

    /**
     * @brief 重做到下一个状态
     * @param outImageData 输出：下一个状态的图像数据
     * @param outDescription 输出：操作描述
     * @return 成功返回 true，无法重做返回 false
     */
    bool Redo(ImageData& outImageData, std::string& outDescription);

    /**
     * @brief 检查是否可以撤销
     */
    bool CanUndo() const;

    /**
     * @brief 检查是否可以重做
     */
    bool CanRedo() const;

    /**
     * @brief 清空所有历史记录
     */
    void Clear();

    /**
     * @brief 获取当前历史记录数量
     */
    size_t GetHistoryCount() const;

    /**
     * @brief 获取当前位置（用于调试）
     */
    int GetCurrentIndex() const;

    /**
     * @brief 获取历史记录描述列表（用于 UI 显示）
     */
    std::vector<std::string> GetHistoryDescriptions() const;
    
    /**
     * @brief 获取指定索引的历史记录描述
     * @param index 历史记录索引
     * @return 描述字符串，如果索引无效返回空字符串
     */
    std::string GetDescription(int index) const;

private:
    std::vector<ImageHistoryEntry> m_History;  // 历史记录栈
    int m_CurrentIndex;                        // 当前位置（-1 表示在最开始）
    size_t m_MaxHistorySize;                   // 最大历史记录数量
};

