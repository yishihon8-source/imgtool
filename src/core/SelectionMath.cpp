#include "SelectionMath.h"
#include <algorithm>

// ========== 核心算法实现 ==========

SelectionMath::IntersectionRect SelectionMath::ComputeIntersection(
    const SelectionRect& selection,
    const LayerPixelBounds& layerBounds
) {
    IntersectionRect intersection;

    // 计算交集的左上角
    intersection.x = std::max(selection.x, layerBounds.x);
    intersection.y = std::max(selection.y, layerBounds.y);

    // 计算交集的右下角
    float intersectRight = std::min(selection.Right(), layerBounds.Right());
    float intersectBottom = std::min(selection.Bottom(), layerBounds.Bottom());

    // 计算尺寸（可能为负，表示无交集）
    intersection.width = std::max(0.0f, intersectRight - intersection.x);
    intersection.height = std::max(0.0f, intersectBottom - intersection.y);

    return intersection;
}

bool SelectionMath::IsSelectionCompletelyInsideLayer(
    const SelectionRect& selection,
    const LayerPixelBounds& layerBounds
) {
    // 选区完全在图层内的条件：
    // selection.left >= layer.left AND selection.right <= layer.right
    // selection.top >= layer.top AND selection.bottom <= layer.bottom

    // 更精确的判断：交集面积 == 选区面积
    IntersectionRect intersection = ComputeIntersection(selection, layerBounds);
    
    // 考虑浮点数精度
    const float epsilon = 1e-6f;
    return std::abs(intersection.width - selection.width) < epsilon &&
           std::abs(intersection.height - selection.height) < epsilon;
}

SelectionMath::OutOfBoundsFlags SelectionMath::ComputeOutOfBoundsFlags(
    const SelectionRect& selection,
    const LayerPixelBounds& layerBounds
) {
    OutOfBoundsFlags flags;

    // 按照 RENAME_FIX.md 规范的精确判断
    flags.left = selection.x < layerBounds.x;
    flags.right = selection.Right() > layerBounds.Right();
    flags.top = selection.y < layerBounds.y;
    flags.bottom = selection.Bottom() > layerBounds.Bottom();

    return flags;
}

bool SelectionMath::ComputeOutOfBoundsLineSegments(
    const SelectionRect& selection,
    const LayerPixelBounds& layerBounds,
    const OutOfBoundsFlags& flags,
    LineSegment& outLeftLine,
    LineSegment& outRightLine,
    LineSegment& outTopLine,
    LineSegment& outBottomLine
) {
    outLeftLine.valid = false;
    outRightLine.valid = false;
    outTopLine.valid = false;
    outBottomLine.valid = false;

    bool hasAnyLine = false;

    // 左边线：在图层左边界绘制，从图层顶部到底部
    if (flags.left) {
        outLeftLine.p1 = ImVec2(layerBounds.x, layerBounds.y);
        outLeftLine.p2 = ImVec2(layerBounds.x, layerBounds.Bottom());
        outLeftLine.valid = true;
        hasAnyLine = true;
    }

    // 右边线：在图层右边界绘制，从图层顶部到底部
    if (flags.right) {
        outRightLine.p1 = ImVec2(layerBounds.Right(), layerBounds.y);
        outRightLine.p2 = ImVec2(layerBounds.Right(), layerBounds.Bottom());
        outRightLine.valid = true;
        hasAnyLine = true;
    }

    // 上边线：在图层上边界绘制，从图层左侧到右侧
    if (flags.top) {
        outTopLine.p1 = ImVec2(layerBounds.x, layerBounds.y);
        outTopLine.p2 = ImVec2(layerBounds.Right(), layerBounds.y);
        outTopLine.valid = true;
        hasAnyLine = true;
    }

    // 下边线：在图层下边界绘制，从图层左侧到右侧
    if (flags.bottom) {
        outBottomLine.p1 = ImVec2(layerBounds.x, layerBounds.Bottom());
        outBottomLine.p2 = ImVec2(layerBounds.Right(), layerBounds.Bottom());
        outBottomLine.valid = true;
        hasAnyLine = true;
    }

    return hasAnyLine;
}

// ========== 工具函数实现 ==========

SelectionMath::SelectionRect SelectionMath::CreateSelectionRect(float x, float y, float w, float h) {
    SelectionRect rect;
    rect.x = x;
    rect.y = y;
    rect.width = w;
    rect.height = h;
    return rect;
}

SelectionMath::LayerPixelBounds SelectionMath::CreateLayerBounds(float x, float y, float w, float h) {
    LayerPixelBounds bounds;
    bounds.x = x;
    bounds.y = y;
    bounds.width = w;
    bounds.height = h;
    return bounds;
}

ImVec2 SelectionMath::CanvasToScreenCoord(
    const ImVec2& canvasCoord,
    const ImVec2& canvasScreenMin,
    float zoom
) {
    return ImVec2(
        canvasScreenMin.x + canvasCoord.x * zoom,
        canvasScreenMin.y + canvasCoord.y * zoom
    );
}

ImVec2 SelectionMath::ScreenToCanvasCoord(
    const ImVec2& screenCoord,
    const ImVec2& canvasScreenMin,
    float zoom
) {
    return ImVec2(
        (screenCoord.x - canvasScreenMin.x) / zoom,
        (screenCoord.y - canvasScreenMin.y) / zoom
    );
}
