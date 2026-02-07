#pragma once

#include "core/ImageHistory.h"
#include "core/GlobalHistory.h"
#include <string>

class PreviewPanel;

/**
 * @brief 历史记录面板
 * 
 * 职责：
 * - 显示撤销/重做按钮
 * - 显示历史记录列表
 * - 显示当前历史状态
 * - 支持点击历史记录恢复到对应状态
 */
class HistoryPanel {
public:
    HistoryPanel();
    ~HistoryPanel();

    /**
     * @brief 渲染历史记录面板
     * @param previewPanel 预览面板引用（用于调用撤销/重做）
     * @param globalHistory 全局操作历史记录
     */
    void Render(PreviewPanel* previewPanel, GlobalHistory* globalHistory = nullptr);
};

