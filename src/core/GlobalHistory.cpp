#include "GlobalHistory.h"
#include "utils/Logger.h"

GlobalHistory::GlobalHistory(size_t maxHistorySize)
    : m_CurrentIndex(-1), m_MaxHistorySize(maxHistorySize) {
}

void GlobalHistory::Push(const std::string& description, std::function<void()> restoreCallback) {
    // 如果当前不在历史记录末尾，删除后续的记录
    if (m_CurrentIndex < static_cast<int>(m_History.size()) - 1) {
        m_History.erase(m_History.begin() + m_CurrentIndex + 1, m_History.end());
    }

    // 添加新记录
    m_History.emplace_back(description, restoreCallback);
    m_CurrentIndex = static_cast<int>(m_History.size()) - 1;

    // 如果超过最大数量，删除最旧的记录
    if (m_History.size() > m_MaxHistorySize) {
        m_History.erase(m_History.begin());
        m_CurrentIndex--;
    }

    Logger::Debug("GlobalHistory::Push() - Added: " + description + 
                  ", Current index: " + std::to_string(m_CurrentIndex));
}

bool GlobalHistory::RestoreTo(int index) {
    if (index < 0 || index >= static_cast<int>(m_History.size())) {
        Logger::Warning("GlobalHistory::RestoreTo() - Invalid index: " + std::to_string(index));
        return false;
    }

    if (index == m_CurrentIndex) {
        Logger::Debug("GlobalHistory::RestoreTo() - Already at index: " + std::to_string(index));
        return true;
    }

    Logger::Info("GlobalHistory::RestoreTo() - Restoring to index " + std::to_string(index) + 
                 ": " + m_History[index].description);

    // 执行恢复回调
    if (m_History[index].restoreCallback) {
        m_History[index].restoreCallback();
        m_CurrentIndex = index;
        return true;
    } else {
        Logger::Error("GlobalHistory::RestoreTo() - No restore callback for index: " + std::to_string(index));
        return false;
    }
}

size_t GlobalHistory::GetHistoryCount() const {
    return m_History.size();
}

int GlobalHistory::GetCurrentIndex() const {
    return m_CurrentIndex;
}

std::string GlobalHistory::GetDescription(int index) const {
    if (index >= 0 && index < static_cast<int>(m_History.size())) {
        return m_History[index].description;
    }
    return "";
}

void GlobalHistory::Clear() {
    m_History.clear();
    m_CurrentIndex = -1;
    Logger::Info("GlobalHistory::Clear() - All history cleared");
}

