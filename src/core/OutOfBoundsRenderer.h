#pragma once

#include "SelectionMath.h"
#include <imgui.h>

/**
 * @file OutOfBoundsRenderer.h
 * @brief 选区越界警告线渲染器
 * 
 * 职责（Renderer 层）：
 * - 基于 SelectionMath 计算的越界标志，绘制粉色警告线
 * - 仅负责视觉渲染，不负责逻辑判断
 * 
 * 规范（遵循 RENAME_FIX.md）：
 * - 粉色线是纯 UI 提示，不参与任何逻辑计算
 * - 仅在发生越界的选区边缘处绘制
 * - 渲染层级：高于蚂蚁线（最高优先级）
 * - 颜色：RGB(255, 0, 255) 或 RGB(255, 64, 255)，Alpha=1.0
 * - 线宽：2px（考虑缩放）
 */
class OutOfBoundsRenderer {
public:
    OutOfBoundsRenderer();

    /**
     * @brief 更新越界检测状态
     * 
     * 基于 SelectionMath 的算法执行越界判断
     * 
     * @param selectionPos 选区左上角位置（画布逻辑坐标，已标准化）
     * @param selectionSize 选区尺寸
     * @param layerPos 图层左上角位置（画布逻辑坐标）
     * @param layerSize 图层尺寸
     * @return 是否有越界（便于外部判断是否需要渲染）
     */
    bool Update(const ImVec2& selectionPos, const ImVec2& selectionSize,
                const ImVec2& layerPos, const ImVec2& layerSize);

    /**
     * @brief 渲染越界警告线
     * 
     * ⚠️ 必须在选区蚂蚁线之后调用，确保粉线覆盖在上面
     * 
     * @param drawList ImGui 绘制列表
     * @param canvasMin 画布屏幕坐标最小值
     * @param canvasMax 画布屏幕坐标最大值（用于边界裁剪）
     * @param canvasZoom 画布缩放比例（用于坐标转换和线宽计算）
     */
    void Render(ImDrawList* drawList, const ImVec2& canvasMin, 
                const ImVec2& canvasMax, float canvasZoom);

    /**
     * @brief 清除越界状态
     * 
     * 当选区被清除或图层改变时调用
     */
    void Clear();

    /**
     * @brief 查询越界状态（调试用）
     */
    const SelectionMath::OutOfBoundsFlags& GetOutOfBoundsFlags() const {
        return m_OutOfBoundsFlags;
    }

private:
    // 越界标志（基于 SelectionMath 计算）
    SelectionMath::OutOfBoundsFlags m_OutOfBoundsFlags;

    // 缓存的几何数据（用于渲染）
    SelectionMath::SelectionRect m_CachedSelection;
    SelectionMath::LayerPixelBounds m_CachedLayerBounds;

    // 视觉样式
    static constexpr ImU32 WARNING_COLOR = IM_COL32(255, 0, 255, 255);  // 纯粉色（洋红）
    static constexpr float LINE_THICKNESS_BASE = 2.0f;  // 基础线宽（屏幕像素）
};

