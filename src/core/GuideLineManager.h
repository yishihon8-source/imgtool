#pragma once

#include <imgui.h>

/**
 * @brief 对齐辅助线管理器
 * 
 * 职责：
 * - 检测图片是否接近画布中心
 * - 管理对齐辅助线的显示
 * - 提供 PS/Figma 风格的对齐提示
 */
class GuideLineManager {
public:
    GuideLineManager();
    ~GuideLineManager() = default;

    /**
     * @brief 更新对齐状态（用于图片/选区中心）
     * @param imageCenter 图片/选区中心点（画布逻辑坐标）
     * @param canvasSize 画布逻辑尺寸
     * @param snapThreshold 吸附阈值（像素）
     * @return 吸附偏移量（如果需要吸附）
     */
    ImVec2 Update(const ImVec2& imageCenter, const ImVec2& canvasSize, float snapThreshold = 8.0f);
    
    /**
     * @brief 更新对齐状态（用于选区边缘）
     * @param selectionRect 选区矩形（画布逻辑坐标：x, y, width, height）
     * @param canvasSize 画布逻辑尺寸
     * @param snapThreshold 吸附阈值（像素）
     * @return 吸附偏移量（如果需要吸附）
     */
    ImVec2 UpdateWithEdges(const ImVec2& selectionPos, const ImVec2& selectionSize, 
                           const ImVec2& canvasSize, float snapThreshold = 8.0f);

    /**
     * @brief 渲染对齐辅助线
     * @param drawList ImGui 绘制列表
     * @param canvasMin 画布左上角屏幕坐标
     * @param canvasMax 画布右下角屏幕坐标
     * @param canvasLogicalSize 画布逻辑尺寸
     */
    void Render(ImDrawList* drawList, 
                const ImVec2& canvasMin, 
                const ImVec2& canvasMax,
                const ImVec2& canvasLogicalSize);

    /**
     * @brief 清除所有辅助线
     */
    void Clear();

    /**
     * @brief 是否显示水平中心线
     */
    bool IsShowingHorizontalCenter() const { return m_ShowHorizontalCenter; }

    /**
     * @brief 是否显示垂直中心线
     */
    bool IsShowingVerticalCenter() const { return m_ShowVerticalCenter; }

private:
    bool m_ShowHorizontalCenter;  // 是否显示水平中心线
    bool m_ShowVerticalCenter;    // 是否显示垂直中心线
    
    // 边缘对齐线
    bool m_ShowLeftEdge;          // 是否显示左边缘线
    bool m_ShowRightEdge;         // 是否显示右边缘线
    bool m_ShowTopEdge;           // 是否显示上边缘线
    bool m_ShowBottomEdge;        // 是否显示下边缘线
    
    // 辅助线样式
    ImU32 m_LineColor;            // 中心线颜色（青色）
    ImU32 m_EdgeLineColor;        // 边缘线颜色（洋红色/粉色）
    float m_LineThickness;
    float m_DashLength;
};


