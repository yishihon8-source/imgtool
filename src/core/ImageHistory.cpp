#include "ImageHistory.h"
#include <iostream>

ImageHistory::ImageHistory(size_t maxHistorySize)
    : m_CurrentIndex(-1)
    , m_MaxHistorySize(maxHistorySize)
{
}

void ImageHistory::Push(const ImageData& imageData, const std::string& description) {
    // 如果当前不在历史记录末尾，清除后续的重做记录
    if (m_CurrentIndex < static_cast<int>(m_History.size()) - 1) {
        m_History.erase(m_History.begin() + m_CurrentIndex + 1, m_History.end());
    }

    // 创建新的历史记录项（深拷贝图像数据）
    ImageHistoryEntry entry;
    entry.imageData = imageData;  // 这会复制整个 vector
    entry.description = description;

    // 添加到历史记录
    m_History.push_back(entry);
    m_CurrentIndex = static_cast<int>(m_History.size()) - 1;

    // 如果超过最大历史记录数，删除最旧的记录
    if (m_History.size() > m_MaxHistorySize) {
        m_History.erase(m_History.begin());
        m_CurrentIndex--;
    }

    printf("[ImageHistory] Pushed: '%s' (index=%d, total=%zu)\n", 
           description.c_str(), m_CurrentIndex, m_History.size());
}

bool ImageHistory::Undo(ImageData& outImageData, std::string& outDescription) {
    if (!CanUndo()) {
        printf("[ImageHistory] Cannot undo: no history available\n");
        return false;
    }

    // 获取上一个状态
    const ImageHistoryEntry& entry = m_History[m_CurrentIndex];
    outImageData = entry.imageData;  // 深拷贝
    outDescription = entry.description;

    m_CurrentIndex--;

    printf("[ImageHistory] Undo: '%s' (new index=%d)\n", 
           outDescription.c_str(), m_CurrentIndex);

    return true;
}

bool ImageHistory::Redo(ImageData& outImageData, std::string& outDescription) {
    if (!CanRedo()) {
        printf("[ImageHistory] Cannot redo: already at latest state\n");
        return false;
    }

    // 移动到下一个状态
    m_CurrentIndex++;

    // 获取下一个状态
    const ImageHistoryEntry& entry = m_History[m_CurrentIndex];
    outImageData = entry.imageData;  // 深拷贝
    outDescription = entry.description;

    printf("[ImageHistory] Redo: '%s' (new index=%d)\n", 
           outDescription.c_str(), m_CurrentIndex);

    return true;
}

bool ImageHistory::CanUndo() const {
    return m_CurrentIndex >= 0;
}

bool ImageHistory::CanRedo() const {
    return m_CurrentIndex < static_cast<int>(m_History.size()) - 1;
}

void ImageHistory::Clear() {
    m_History.clear();
    m_CurrentIndex = -1;
    printf("[ImageHistory] Cleared all history\n");
}

size_t ImageHistory::GetHistoryCount() const {
    return m_History.size();
}

int ImageHistory::GetCurrentIndex() const {
    return m_CurrentIndex;
}

std::vector<std::string> ImageHistory::GetHistoryDescriptions() const {
    std::vector<std::string> descriptions;
    descriptions.reserve(m_History.size());
    
    for (const auto& entry : m_History) {
        descriptions.push_back(entry.description);
    }
    
    return descriptions;
}

