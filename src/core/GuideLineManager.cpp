#include "GuideLineManager.h"
#include <cmath>
#include <algorithm>

GuideLineManager::GuideLineManager()
    : m_ShowHorizontalCenter(false)
    , m_ShowVerticalCenter(false)
    , m_ShowLeftEdge(false)
    , m_ShowRightEdge(false)
    , m_ShowTopEdge(false)
    , m_ShowBottomEdge(false)
    , m_LineColor(IM_COL32(0, 200, 255, 255))      // 亮青色（中心线）
    , m_EdgeLineColor(IM_COL32(255, 0, 255, 200))  // 洋红色/粉色（边缘线）
    , m_LineThickness(1.5f)
    , m_DashLength(10.0f)
{
}

ImVec2 GuideLineManager::Update(const ImVec2& imageCenter, const ImVec2& canvasSize, float snapThreshold) {
    // 计算画布中心
    float canvasCenterX = canvasSize.x * 0.5f;
    float canvasCenterY = canvasSize.y * 0.5f;
    
    // 计算吸附偏移量
    ImVec2 snapOffset(0.0f, 0.0f);
    
    // 检测水平对齐（垂直中心线）
    float deltaX = imageCenter.x - canvasCenterX;
    if (std::abs(deltaX) < snapThreshold) {
        m_ShowVerticalCenter = true;
        snapOffset.x = -deltaX;  // 吸附到中心
    } else {
        m_ShowVerticalCenter = false;
    }
    
    // 检测垂直对齐（水平中心线）
    float deltaY = imageCenter.y - canvasCenterY;
    if (std::abs(deltaY) < snapThreshold) {
        m_ShowHorizontalCenter = true;
        snapOffset.y = -deltaY;  // 吸附到中心
    } else {
        m_ShowHorizontalCenter = false;
    }
    
    return snapOffset;
}

void GuideLineManager::Render(ImDrawList* drawList, 
                               const ImVec2& canvasMin, 
                               const ImVec2& canvasMax,
                               const ImVec2& canvasLogicalSize) {
    // 避免未使用参数警告
    (void)canvasLogicalSize;
    
    if (!drawList) {
        return;
    }
    
    // 计算画布中心的屏幕坐标
    ImVec2 canvasCenter;
    canvasCenter.x = (canvasMin.x + canvasMax.x) * 0.5f;
    canvasCenter.y = (canvasMin.y + canvasMax.y) * 0.5f;
    
    // 绘制垂直中心线（竖线）
    if (m_ShowVerticalCenter) {
        drawList->AddLine(
            ImVec2(canvasCenter.x, canvasMin.y),
            ImVec2(canvasCenter.x, canvasMax.y),
            m_LineColor,
            m_LineThickness
        );
    }
    
    // 绘制水平中心线（横线）
    if (m_ShowHorizontalCenter) {
        drawList->AddLine(
            ImVec2(canvasMin.x, canvasCenter.y),
            ImVec2(canvasMax.x, canvasCenter.y),
            m_LineColor,
            m_LineThickness
        );
    }
    
    // ✅ 边缘线绘制已禁用（由 OutOfBoundsRenderer 负责选区越界警告）
    // GuideLineManager 现在只负责中心对齐辅助线
    
    // 注释掉边缘线绘制，避免与 OutOfBoundsRenderer 的粉色警告线冲突
    /*
    // 绘制左边缘线（粉色）
    if (m_ShowLeftEdge) {
        drawList->AddLine(
            ImVec2(canvasMin.x, canvasMin.y),
            ImVec2(canvasMin.x, canvasMax.y),
            m_EdgeLineColor,
            m_LineThickness
        );
    }
    
    // 绘制右边缘线（粉色）
    if (m_ShowRightEdge) {
        drawList->AddLine(
            ImVec2(canvasMax.x, canvasMin.y),
            ImVec2(canvasMax.x, canvasMax.y),
            m_EdgeLineColor,
            m_LineThickness
        );
    }
    
    // 绘制上边缘线（粉色）
    if (m_ShowTopEdge) {
        drawList->AddLine(
            ImVec2(canvasMin.x, canvasMin.y),
            ImVec2(canvasMax.x, canvasMin.y),
            m_EdgeLineColor,
            m_LineThickness
        );
    }
    
    // 绘制下边缘线（粉色）
    if (m_ShowBottomEdge) {
        drawList->AddLine(
            ImVec2(canvasMin.x, canvasMax.y),
            ImVec2(canvasMax.x, canvasMax.y),
            m_EdgeLineColor,
            m_LineThickness
        );
    }
    */
}

ImVec2 GuideLineManager::UpdateWithEdges(const ImVec2& selectionPos, const ImVec2& selectionSize,
                                          const ImVec2& canvasSize, float snapThreshold) {
    // 计算选区的边缘和中心
    float selLeft = selectionPos.x;
    float selRight = selectionPos.x + selectionSize.x;
    float selTop = selectionPos.y;
    float selBottom = selectionPos.y + selectionSize.y;
    float selCenterX = selectionPos.x + selectionSize.x * 0.5f;
    float selCenterY = selectionPos.y + selectionSize.y * 0.5f;
    
    // 画布的边缘和中心
    float canvasLeft = 0.0f;
    float canvasRight = canvasSize.x;
    float canvasTop = 0.0f;
    float canvasBottom = canvasSize.y;
    float canvasCenterX = canvasSize.x * 0.5f;
    float canvasCenterY = canvasSize.y * 0.5f;
    
    // 吸附偏移量
    ImVec2 snapOffset(0.0f, 0.0f);
    
    // 重置所有标志
    m_ShowHorizontalCenter = false;
    m_ShowVerticalCenter = false;
    m_ShowLeftEdge = false;
    m_ShowRightEdge = false;
    m_ShowTopEdge = false;
    m_ShowBottomEdge = false;
    
    // ✅ 检测水平方向对齐（可以同时检测多个）
    // 左边缘
    if (std::abs(selLeft - canvasLeft) <= snapThreshold) {
        m_ShowLeftEdge = true;
        snapOffset.x = canvasLeft - selLeft;
    }
    // 右边缘
    else if (std::abs(selRight - canvasRight) <= snapThreshold) {
        m_ShowRightEdge = true;
        snapOffset.x = canvasRight - selRight;
    }
    // 水平中心
    else if (std::abs(selCenterX - canvasCenterX) <= snapThreshold) {
        m_ShowVerticalCenter = true;
        snapOffset.x = canvasCenterX - selCenterX;
    }
    
    // ✅ 检测垂直方向对齐（可以同时检测多个）
    // 上边缘
    if (std::abs(selTop - canvasTop) <= snapThreshold) {
        m_ShowTopEdge = true;
        snapOffset.y = canvasTop - selTop;
    }
    // 下边缘
    else if (std::abs(selBottom - canvasBottom) <= snapThreshold) {
        m_ShowBottomEdge = true;
        snapOffset.y = canvasBottom - selBottom;
    }
    // 垂直中心
    else if (std::abs(selCenterY - canvasCenterY) <= snapThreshold) {
        m_ShowHorizontalCenter = true;
        snapOffset.y = canvasCenterY - selCenterY;
    }
    
    return snapOffset;
}

void GuideLineManager::Clear() {
    m_ShowHorizontalCenter = false;
    m_ShowVerticalCenter = false;
    m_ShowLeftEdge = false;
    m_ShowRightEdge = false;
    m_ShowTopEdge = false;
    m_ShowBottomEdge = false;
}


