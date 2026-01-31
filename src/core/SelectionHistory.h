#pragma once

#include <vector>
#include <cstddef>

// 前向声明
struct SelectionRect;

/**
 * @brief 选区历史记录系统
 * 
 * 职责：
 * - 管理选区操作的撤销/重做栈
 * - 支持 Undo/Redo 操作
 * - 限制历史记录大小
 */
class SelectionHistory {
public:
    SelectionHistory();

    /**
     * @brief 提交一个新的选区状态到历史记录
     * @param selection 选区状态
     */
    void Push(const SelectionRect& selection);

    /**
     * @brief 撤销到上一个状态
     * @return 上一个选区状态
     */
    SelectionRect Undo();

    /**
     * @brief 重做到下一个状态
     * @return 下一个选区状态
     */
    SelectionRect Redo();

    /**
     * @brief 是否可以撤销
     */
    bool CanUndo() const;

    /**
     * @brief 是否可以重做
     */
    bool CanRedo() const;

    /**
     * @brief 清空历史记录
     */
    void Clear();

    /**
     * @brief 获取当前历史记录数量
     */
    size_t GetHistorySize() const { return m_UndoStack.size(); }

private:
    std::vector<SelectionRect> m_UndoStack;     // 撤销栈
    std::vector<SelectionRect> m_RedoStack;     // 重做栈
    size_t m_CurrentIndex;                       // 当前位置索引
    const size_t m_MaxHistorySize = 50;         // 最大历史记录数
};

