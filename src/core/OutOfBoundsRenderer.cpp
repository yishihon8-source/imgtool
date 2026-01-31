#include "OutOfBoundsRenderer.h"
#include <algorithm>

OutOfBoundsRenderer::OutOfBoundsRenderer()
{
    Clear();
}

bool OutOfBoundsRenderer::Update(
    const ImVec2& selectionPos,
    const ImVec2& selectionSize,
    const ImVec2& layerPos,
    const ImVec2& layerSize
) {
    // ✅ 创建标准化的几何数据（画布逻辑坐标）
    m_CachedSelection = SelectionMath::CreateSelectionRect(
        selectionPos.x, selectionPos.y,
        selectionSize.x, selectionSize.y
    );

    m_CachedLayerBounds = SelectionMath::CreateLayerBounds(
        layerPos.x, layerPos.y,
        layerSize.x, layerSize.y
    );

    // ✅ 使用 SelectionMath 计算越界标志（遵循文档规范）
    m_OutOfBoundsFlags = SelectionMath::ComputeOutOfBoundsFlags(
        m_CachedSelection,
        m_CachedLayerBounds
    );

    return m_OutOfBoundsFlags.HasAnyOutOfBounds();
}

void OutOfBoundsRenderer::Render(
    ImDrawList* drawList,
    const ImVec2& canvasMin,
    const ImVec2& canvasMax,
    float canvasZoom
) {
    if (!drawList || !m_OutOfBoundsFlags.HasAnyOutOfBounds()) {
        return;
    }

    // ✅ 使用 SelectionMath 计算需要绘制的线段
    SelectionMath::LineSegment leftLine, rightLine, topLine, bottomLine;

    bool hasAnyLine = SelectionMath::ComputeOutOfBoundsLineSegments(
        m_CachedSelection,
        m_CachedLayerBounds,
        m_OutOfBoundsFlags,
        leftLine, rightLine, topLine, bottomLine
    );

    if (!hasAnyLine) {
        return;
    }

    // ✅ 计算考虑缩放的线条粗细
    float lineThickness = std::max(1.0f, LINE_THICKNESS_BASE * canvasZoom);

    // ✅ 绘制四条越界警告线（只有标志为 true 的才会绘制）
    // 这些线段坐标是画布逻辑坐标，需要转换为屏幕坐标

    auto drawLineSegment = [&](const SelectionMath::LineSegment& seg) {
        if (!seg.valid) return;

        // 转换为屏幕坐标
        ImVec2 screenP1 = SelectionMath::CanvasToScreenCoord(seg.p1, canvasMin, canvasZoom);
        ImVec2 screenP2 = SelectionMath::CanvasToScreenCoord(seg.p2, canvasMin, canvasZoom);

        // 绘制粉色警告线（绝对颜色，不受缩放影响）
        drawList->AddLine(screenP1, screenP2, WARNING_COLOR, lineThickness);
    };

    // 按顺序绘制四边（顺序无关紧要，但保持一致性有助于调试）
    drawLineSegment(leftLine);
    drawLineSegment(rightLine);
    drawLineSegment(topLine);
    drawLineSegment(bottomLine);
}

void OutOfBoundsRenderer::Clear() {
    m_OutOfBoundsFlags.left = false;
    m_OutOfBoundsFlags.right = false;
    m_OutOfBoundsFlags.top = false;
    m_OutOfBoundsFlags.bottom = false;

    m_CachedSelection = SelectionMath::CreateSelectionRect(0, 0, 0, 0);
    m_CachedLayerBounds = SelectionMath::CreateLayerBounds(0, 0, 0, 0);
}

