#include "CropRenderer.h"

CropRenderer::CropRenderer() = default;
CropRenderer::~CropRenderer() = default;

void CropRenderer::Render(ImDrawList* drawList,
                         const CropRect& cropRect,
                         const ImVec2& canvasMin,
                         const ImVec2& canvasMax,
                         const ImVec2& canvasLogicalSize,
                         bool showRuleOfThirds,
                         CropToolState state) {
    if (!drawList || !cropRect.IsValid()) {
        return;
    }
    
    // 计算画布逻辑坐标到屏幕坐标的缩放比例
    float scaleX = (canvasMax.x - canvasMin.x) / canvasLogicalSize.x;
    float scaleY = (canvasMax.y - canvasMin.y) / canvasLogicalSize.y;
    
    // 将裁剪矩形转换为屏幕坐标
    CropRect normalized = cropRect.GetNormalized();
    ImVec2 cropMin(
        canvasMin.x + static_cast<float>(normalized.left) * scaleX,
        canvasMin.y + static_cast<float>(normalized.top) * scaleY
    );
    ImVec2 cropMax(
        canvasMin.x + static_cast<float>(normalized.right) * scaleX,
        canvasMin.y + static_cast<float>(normalized.bottom) * scaleY
    );
    
    // 1. 渲染 overlay mask（裁剪区域外的半透明遮罩）
    RenderOverlayMask(drawList, canvasMin, canvasMax, cropMin, cropMax);
    
    // 2. 渲染裁剪框边框
    RenderCropBorder(drawList, cropMin, cropMax, state);
    
    // 3. 渲染 Rule of Thirds 网格
    if (showRuleOfThirds) {
        RenderRuleOfThirds(drawList, cropMin, cropMax);
    }
}

void CropRenderer::RenderOverlayMask(ImDrawList* drawList,
                                    const ImVec2& canvasMin,
                                    const ImVec2& canvasMax,
                                    const ImVec2& cropMin,
                                    const ImVec2& cropMax) {
    // 绘制四个矩形遮罩（上、下、左、右）
    
    // 上方遮罩
    if (cropMin.y > canvasMin.y) {
        drawList->AddRectFilled(
            canvasMin,
            ImVec2(canvasMax.x, cropMin.y),
            m_MaskColor
        );
    }
    
    // 下方遮罩
    if (cropMax.y < canvasMax.y) {
        drawList->AddRectFilled(
            ImVec2(canvasMin.x, cropMax.y),
            canvasMax,
            m_MaskColor
        );
    }
    
    // 左侧遮罩
    if (cropMin.x > canvasMin.x) {
        drawList->AddRectFilled(
            ImVec2(canvasMin.x, cropMin.y),
            ImVec2(cropMin.x, cropMax.y),
            m_MaskColor
        );
    }
    
    // 右侧遮罩
    if (cropMax.x < canvasMax.x) {
        drawList->AddRectFilled(
            ImVec2(cropMax.x, cropMin.y),
            ImVec2(canvasMax.x, cropMax.y),
            m_MaskColor
        );
    }
}

void CropRenderer::RenderRuleOfThirds(ImDrawList* drawList,
                                     const ImVec2& cropMin,
                                     const ImVec2& cropMax) {
    float width = cropMax.x - cropMin.x;
    float height = cropMax.y - cropMin.y;
    
    // 垂直线（1/3 和 2/3 位置）
    float x1 = cropMin.x + width / 3.0f;
    float x2 = cropMin.x + width * 2.0f / 3.0f;
    
    drawList->AddLine(
        ImVec2(x1, cropMin.y),
        ImVec2(x1, cropMax.y),
        m_GridColor,
        m_GridThickness
    );
    
    drawList->AddLine(
        ImVec2(x2, cropMin.y),
        ImVec2(x2, cropMax.y),
        m_GridColor,
        m_GridThickness
    );
    
    // 水平线（1/3 和 2/3 位置）
    float y1 = cropMin.y + height / 3.0f;
    float y2 = cropMin.y + height * 2.0f / 3.0f;
    
    drawList->AddLine(
        ImVec2(cropMin.x, y1),
        ImVec2(cropMax.x, y1),
        m_GridColor,
        m_GridThickness
    );
    
    drawList->AddLine(
        ImVec2(cropMin.x, y2),
        ImVec2(cropMax.x, y2),
        m_GridColor,
        m_GridThickness
    );
}

void CropRenderer::RenderCropBorder(ImDrawList* drawList,
                                   const ImVec2& cropMin,
                                   const ImVec2& cropMax,
                                   CropToolState state) {
    // 根据状态选择边框颜色
    ImU32 borderColor = m_BorderColor;
    if (state == CropToolState::Resizing || state == CropToolState::Moving) {
        borderColor = IM_COL32(0, 150, 255, 255);  // 拖拽时使用蓝色
    }
    
    // ✅ 双层边框：先绘制黑色阴影，再绘制白色边框（增强对比度）
    // 黑色外边框（阴影效果）
    drawList->AddRect(
        ImVec2(cropMin.x - 1, cropMin.y - 1),
        ImVec2(cropMax.x + 1, cropMax.y + 1),
        m_BorderShadowColor,
        0.0f,
        0,
        m_BorderThickness + 2.0f
    );
    
    // 白色主边框
    drawList->AddRect(
        cropMin,
        cropMax,
        borderColor,
        0.0f,
        0,
        m_BorderThickness
    );
}

void CropRenderer::RenderHandles(ImDrawList* drawList,
                                const ImVec2 handles[8],
                                CropHandle hoveredHandle) {
    if (!drawList) {
        return;
    }
    
    // 绘制8个控制点
    for (int i = 0; i < 8; i++) {
        ImVec2 handleMin(
            handles[i].x - m_HandleSize * 0.5f,
            handles[i].y - m_HandleSize * 0.5f
        );
        ImVec2 handleMax(
            handles[i].x + m_HandleSize * 0.5f,
            handles[i].y + m_HandleSize * 0.5f
        );
        
        // 判断是否悬停
        bool isHovered = (static_cast<int>(hoveredHandle) == i);
        ImU32 fillColor = isHovered ? m_HandleHoverColor : m_HandleColor;
        
        // 绘制控制点（白色填充 + 黑色边框）
        drawList->AddRectFilled(handleMin, handleMax, fillColor);
        drawList->AddRect(handleMin, handleMax, m_HandleBorderColor, 0.0f, 0, 1.5f);
    }
}

