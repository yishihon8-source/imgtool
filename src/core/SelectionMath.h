#pragma once

#include <imgui.h>
#include <cmath>

/**
 * @file SelectionMath.h
 * @brief 选区数学库 - Photoshop 级越界判断与几何计算
 * 
 * 职责：
 * - 选区与图层边界的交集计算
 * - 越界判断（基于文档规范）
 * - 边界线段的精确定位
 * - 坐标系转换
 * 
 * 遵循 RENAME_FIX.md 中的完整规范
 */
class SelectionMath {
public:
    /**
     * @brief 选区矩形（标准化，画布逻辑坐标）
     */
    struct SelectionRect {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;

        // 便利访问器
        float Left() const { return x; }
        float Right() const { return x + width; }
        float Top() const { return y; }
        float Bottom() const { return y + height; }

        bool IsValid() const { return width > 0.0f && height > 0.0f; }
    };

    /**
     * @brief 图层像素边界（标准化，画布逻辑坐标）
     */
    struct LayerPixelBounds {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;

        // 便利访问器
        float Left() const { return x; }
        float Right() const { return x + width; }
        float Top() const { return y; }
        float Bottom() const { return y + height; }

        bool IsValid() const { return width > 0.0f && height > 0.0f; }
    };

    /**
     * @brief 交集矩形
     */
    struct IntersectionRect {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;

        bool IsValid() const { return width > 0.0f && height > 0.0f; }
        bool IsEmpty() const { return !IsValid(); }
    };

    /**
     * @brief 越界标志集合（四边独立）
     */
    struct OutOfBoundsFlags {
        bool left = false;
        bool right = false;
        bool top = false;
        bool bottom = false;

        bool HasAnyOutOfBounds() const {
            return left || right || top || bottom;
        }
    };

    // ========== 核心算法 ==========

    /**
     * @brief 计算选区与图层边界的交集
     * @param selection 选区（标准化）
     * @param layerBounds 图层像素边界
     * @return 交集矩形
     */
    static IntersectionRect ComputeIntersection(
        const SelectionRect& selection,
        const LayerPixelBounds& layerBounds
    );

    /**
     * @brief 判断选区是否完全包含于图层
     * @param selection 选区（标准化）
     * @param layerBounds 图层像素边界
     * @return true 表示选区完全在图层内；false 表示选区有越界
     */
    static bool IsSelectionCompletelyInsideLayer(
        const SelectionRect& selection,
        const LayerPixelBounds& layerBounds
    );

    /**
     * @brief 计算四边的越界标志
     * 
     * 规范（来自 RENAME_FIX.md）：
     * - 如果 selection.left < layer.left，则左边越界
     * - 如果 selection.right > layer.right，则右边越界
     * - 如果 selection.top < layer.top，则上边越界
     * - 如果 selection.bottom > layer.bottom，则下边越界
     *
     * @param selection 选区（标准化）
     * @param layerBounds 图层像素边界
     * @return 越界标志集合
     */
    static OutOfBoundsFlags ComputeOutOfBoundsFlags(
        const SelectionRect& selection,
        const LayerPixelBounds& layerBounds
    );

    /**
     * @brief 计算需要绘制粉色线的四条边线段
     * 
     * 粉色线仅在"发生越界"的选区边缘处绘制
     * 
     * @param selection 选区（标准化）
     * @param layerBounds 图层像素边界
     * @param flags 越界标志（通常由 ComputeOutOfBoundsFlags 计算）
     * @param outLeftLine 左边线段（如果越界）
     * @param outRightLine 右边线段（如果越界）
     * @param outTopLine 上边线段（如果越界）
     * @param outBottomLine 下边线段（如果越界）
     * @return true 表示有线段需要绘制
     */
    struct LineSegment {
        ImVec2 p1;
        ImVec2 p2;
        bool valid = false;
    };

    static bool ComputeOutOfBoundsLineSegments(
        const SelectionRect& selection,
        const LayerPixelBounds& layerBounds,
        const OutOfBoundsFlags& flags,
        LineSegment& outLeftLine,
        LineSegment& outRightLine,
        LineSegment& outTopLine,
        LineSegment& outBottomLine
    );

    // ========== 工具函数 ==========

    /**
     * @brief 从 ImVec2 创建选区
     */
    static SelectionRect CreateSelectionRect(float x, float y, float w, float h);

    /**
     * @brief 从 ImVec2 创建图层边界
     */
    static LayerPixelBounds CreateLayerBounds(float x, float y, float w, float h);

    /**
     * @brief 坐标转换：画布逻辑坐标 -> 屏幕显示坐标
     */
    static ImVec2 CanvasToScreenCoord(
        const ImVec2& canvasCoord,
        const ImVec2& canvasScreenMin,
        float zoom
    );

    /**
     * @brief 坐标转换：屏幕显示坐标 -> 画布逻辑坐标
     */
    static ImVec2 ScreenToCanvasCoord(
        const ImVec2& screenCoord,
        const ImVec2& canvasScreenMin,
        float zoom
    );
};
