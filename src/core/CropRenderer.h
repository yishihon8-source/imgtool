#pragma once

#include "CropTool.h"
#include <imgui.h>

/**
 * @brief Crop Tool 渲染器
 * 
 * 职责：
 * - 渲染裁剪框边框和控制点
 * - 渲染 overlay mask（裁剪区域外的半透明遮罩）
 * - 渲染 Rule of Thirds 网格
 * - 使用 ImDrawList 进行高性能渲染
 * 
 * 性能优化：
 * - 支持 8K 图像实时渲染
 * - 最小化 DrawCall
 * - 使用批量绘制
 */
class CropRenderer {
public:
    CropRenderer();
    ~CropRenderer();
    
    /**
     * @brief 渲染裁剪工具
     * @param drawList ImGui 绘制列表
     * @param cropRect 裁剪矩形（画布逻辑坐标）
     * @param canvasMin 画布左上角（屏幕坐标）
     * @param canvasMax 画布右下角（屏幕坐标）
     * @param canvasLogicalSize 画布逻辑尺寸
     * @param showRuleOfThirds 是否显示三分法网格
     * @param state 裁剪工具状态
     */
    void Render(ImDrawList* drawList,
                const CropRect& cropRect,
                const ImVec2& canvasMin,
                const ImVec2& canvasMax,
                const ImVec2& canvasLogicalSize,
                bool showRuleOfThirds,
                CropToolState state);
    
    /**
     * @brief 渲染控制点
     * @param drawList ImGui 绘制列表
     * @param handles 8个控制点的屏幕坐标
     * @param hoveredHandle 当前悬停的控制点
     */
    void RenderHandles(ImDrawList* drawList,
                      const ImVec2 handles[8],
                      CropHandle hoveredHandle);
    
private:
    /**
     * @brief 渲染 overlay mask（裁剪区域外的半透明遮罩）
     */
    void RenderOverlayMask(ImDrawList* drawList,
                          const ImVec2& canvasMin,
                          const ImVec2& canvasMax,
                          const ImVec2& cropMin,
                          const ImVec2& cropMax);
    
    /**
     * @brief 渲染 Rule of Thirds 网格
     */
    void RenderRuleOfThirds(ImDrawList* drawList,
                           const ImVec2& cropMin,
                           const ImVec2& cropMax);
    
    /**
     * @brief 渲染裁剪框边框
     */
    void RenderCropBorder(ImDrawList* drawList,
                         const ImVec2& cropMin,
                         const ImVec2& cropMax,
                         CropToolState state);
    
    // 颜色配置（Photoshop 风格 - 增强可见性）
    const ImU32 m_MaskColor = IM_COL32(0, 0, 0, 180);           // 遮罩颜色（更深）
    const ImU32 m_BorderColor = IM_COL32(255, 255, 255, 255);   // 边框颜色（白色）
    const ImU32 m_BorderShadowColor = IM_COL32(0, 0, 0, 200);   // 边框阴影（黑色）
    const ImU32 m_GridColor = IM_COL32(255, 255, 255, 150);     // 网格颜色（更亮）
    const ImU32 m_HandleColor = IM_COL32(255, 255, 255, 255);   // 控制点颜色
    const ImU32 m_HandleBorderColor = IM_COL32(0, 0, 0, 255);   // 控制点边框（更深）
    const ImU32 m_HandleHoverColor = IM_COL32(0, 150, 255, 255); // 悬停颜色
    
    const float m_BorderThickness = 3.0f;      // 边框更粗
    const float m_GridThickness = 1.5f;        // 网格更粗
    const float m_HandleSize = 10.0f;          // 控制点更大
};

