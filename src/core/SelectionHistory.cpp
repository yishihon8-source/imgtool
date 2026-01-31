#include "SelectionHistory.h"
#include "SelectionSystem.h"

SelectionHistory::SelectionHistory()
    : m_CurrentIndex(0)
{
}

void SelectionHistory::Push(const SelectionRect& selection) {
    // 如果当前不在栈顶，清空重做栈
    if (!m_RedoStack.empty()) {
        m_RedoStack.clear();
    }

    // 添加到撤销栈
    m_UndoStack.push_back(selection);

    // 限制历史记录大小
    if (m_UndoStack.size() > m_MaxHistorySize) {
        m_UndoStack.erase(m_UndoStack.begin());
    }

    m_CurrentIndex = m_UndoStack.size();
}

SelectionRect SelectionHistory::Undo() {
    if (!CanUndo()) {
        // 返回一个空选区
        SelectionRect empty;
        empty.Clear();
        return empty;
    }

    // 当前状态移到重做栈
    if (m_CurrentIndex > 0 && m_CurrentIndex <= m_UndoStack.size()) {
        m_RedoStack.push_back(m_UndoStack[m_CurrentIndex - 1]);
    }

    // 移动到上一个状态
    m_CurrentIndex--;

    if (m_CurrentIndex > 0) {
        return m_UndoStack[m_CurrentIndex - 1];
    } else {
        // 返回空选区（表示没有选区）
        SelectionRect empty;
        empty.Clear();
        return empty;
    }
}

SelectionRect SelectionHistory::Redo() {
    if (!CanRedo()) {
        // 返回当前状态
        if (m_CurrentIndex > 0 && m_CurrentIndex <= m_UndoStack.size()) {
            return m_UndoStack[m_CurrentIndex - 1];
        }
        SelectionRect empty;
        empty.Clear();
        return empty;
    }

    // 从重做栈恢复
    SelectionRect selection = m_RedoStack.back();
    m_RedoStack.pop_back();

    m_CurrentIndex++;

    return selection;
}

bool SelectionHistory::CanUndo() const {
    return m_CurrentIndex > 0;
}

bool SelectionHistory::CanRedo() const {
    return !m_RedoStack.empty();
}

void SelectionHistory::Clear() {
    m_UndoStack.clear();
    m_RedoStack.clear();
    m_CurrentIndex = 0;
}

