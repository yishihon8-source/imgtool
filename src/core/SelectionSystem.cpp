#include "SelectionSystem.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

SelectionSystem::SelectionSystem()
    : m_AnchorPoint(0.0f, 0.0f)
    , m_MoveOffset(0.0f, 0.0f)
    , m_IsCreating(false)
    , m_IsMoving(false)
    , m_DashOffset(0.0f)
{
}

void SelectionSystem::Update(const ImVec2& mousePos, bool isMouseDown, bool isMouseClicked,
                              bool shiftPressed, bool altPressed) {
    if (isMouseClicked) {
        // 鼠标按下：判断是创建新选区还是移动现有选区
        if (m_Selection.active && m_Selection.Contains(mousePos.x, mousePos.y)) {
            // 点击在选区内部 -> 进入移动模式
            m_IsMoving = true;
            m_IsCreating = false;
            SelectionRect norm = m_Selection.GetNormalized();
            m_MoveOffset.x = mousePos.x - norm.x;
            m_MoveOffset.y = mousePos.y - norm.y;
        } else {
            // 点击在选区外部 -> 创建新选区
            m_IsCreating = true;
            m_IsMoving = false;
            m_AnchorPoint = mousePos;
            m_Selection.x = mousePos.x;
            m_Selection.y = mousePos.y;
            m_Selection.width = 0.0f;
            m_Selection.height = 0.0f;
            m_Selection.active = true;
        }
    }

    if (isMouseDown) {
        if (m_IsCreating) {
            CreateSelection(mousePos, shiftPressed, altPressed);
        } else if (m_IsMoving) {
            MoveSelection(mousePos);
        }
    } else {
        // 鼠标松开：结束创建或移动
        if (m_IsCreating || m_IsMoving) {
            // 如果选区太小（小于3像素），则取消选区
            if (m_IsCreating) {
                SelectionRect norm = m_Selection.GetNormalized();
                if (norm.width < 3.0f || norm.height < 3.0f) {
                    m_Selection.Clear();
                } else {
                    // 提交到历史记录
                    CommitSelection();
                }
            } else if (m_IsMoving) {
                // 移动完成，提交到历史记录
                CommitSelection();
            }
        }
        m_IsCreating = false;
        m_IsMoving = false;
    }
}

void SelectionSystem::CreateSelection(const ImVec2& mousePos, bool shiftPressed, bool altPressed) {
    float dx = mousePos.x - m_AnchorPoint.x;
    float dy = mousePos.y - m_AnchorPoint.y;

    if (altPressed && shiftPressed) {
        // Shift + Alt: 中心对称正方形
        float size = std::max(std::abs(dx), std::abs(dy));
        float signX = (dx >= 0) ? 1.0f : -1.0f;
        float signY = (dy >= 0) ? 1.0f : -1.0f;
        
        m_Selection.x = m_AnchorPoint.x - size * signX;
        m_Selection.y = m_AnchorPoint.y - size * signY;
        m_Selection.width = 2.0f * size * signX;
        m_Selection.height = 2.0f * size * signY;
    } else if (altPressed) {
        // Alt: 从中心向外创建
        m_Selection.x = m_AnchorPoint.x - dx;
        m_Selection.y = m_AnchorPoint.y - dy;
        m_Selection.width = 2.0f * dx;
        m_Selection.height = 2.0f * dy;
    } else if (shiftPressed) {
        // Shift: 等比例正方形
        float size = std::max(std::abs(dx), std::abs(dy));
        float signX = (dx >= 0) ? 1.0f : -1.0f;
        float signY = (dy >= 0) ? 1.0f : -1.0f;
        
        m_Selection.x = m_AnchorPoint.x;
        m_Selection.y = m_AnchorPoint.y;
        m_Selection.width = size * signX;
        m_Selection.height = size * signY;
    } else {
        // 默认：自由矩形
        m_Selection.x = m_AnchorPoint.x;
        m_Selection.y = m_AnchorPoint.y;
        m_Selection.width = dx;
        m_Selection.height = dy;
    }
}

void SelectionSystem::MoveSelection(const ImVec2& mousePos) {
    // 移动选区：保持宽高不变，只改变位置
    SelectionRect norm = m_Selection.GetNormalized();
    float newX = mousePos.x - m_MoveOffset.x;
    float newY = mousePos.y - m_MoveOffset.y;
    
    // 更新选区位置（保持标准化状态）
    m_Selection.x = newX;
    m_Selection.y = newY;
    m_Selection.width = norm.width;
    m_Selection.height = norm.height;
}

void SelectionSystem::Render(ImDrawList* drawList, const ImVec2& canvasMin, 
                              const ImVec2& canvasMax, float deltaTime, float scale,
                              const SelectionRect* layerBounds) {
    // 注意：layerBounds 参数保留用于向后兼容，但不再在渲染时使用
    // 选区裁剪应该在鼠标松开时通过 ClampSelectionToLayer() 完成
    (void)layerBounds;  // 避免未使用参数警告
    
    if (!m_Selection.active || !drawList) {
        return;
    }

    // 更新蚂蚁线动画
    m_DashOffset += deltaTime * 30.0f;  // 动画速度
    if (m_DashOffset > 20.0f) {
        m_DashOffset -= 20.0f;
    }

    // 获取标准化选区（画布逻辑坐标）
    SelectionRect norm = m_Selection.GetNormalized();

    // ✅ 将选区从画布逻辑坐标转换为屏幕坐标（使用正确的 scale）
    SelectionRect screenRect;
    screenRect.x = canvasMin.x + norm.x * scale;
    screenRect.y = canvasMin.y + norm.y * scale;
    screenRect.width = norm.width * scale;
    screenRect.height = norm.height * scale;
    screenRect.active = true;

    // 渲染半透明遮罩（可选）
    RenderMask(drawList, canvasMin, canvasMax, screenRect);

    // 渲染蚂蚁线
    RenderMarchingAnts(drawList, screenRect, m_DashOffset);
}

void SelectionSystem::RenderMarchingAnts(ImDrawList* drawList, const SelectionRect& rect, float dashOffset) {
    const float dashLength = 10.0f;
    const float gapLength = 10.0f;
    const ImU32 lineColor1 = IM_COL32(255, 255, 255, 255);  // 白色
    const ImU32 lineColor2 = IM_COL32(0, 0, 0, 255);        // 黑色
    const float lineThickness = 2.0f;

    SelectionRect norm = rect.GetNormalized();
    float x1 = norm.x;
    float y1 = norm.y;
    float x2 = norm.x + norm.width;
    float y2 = norm.y + norm.height;

    // 绘制四条边的虚线
    auto drawDashedLine = [&](float startX, float startY, float endX, float endY) {
        float dx = endX - startX;
        float dy = endY - startY;
        float length = std::sqrt(dx * dx + dy * dy);
        float dirX = dx / length;
        float dirY = dy / length;

        float offset = std::fmod(dashOffset, dashLength + gapLength);
        float currentPos = -offset;

        while (currentPos < length) {
            float segStart = std::max(0.0f, currentPos);
            float segEnd = std::min(length, currentPos + dashLength);

            if (segEnd > segStart) {
                ImVec2 p1(startX + dirX * segStart, startY + dirY * segStart);
                ImVec2 p2(startX + dirX * segEnd, startY + dirY * segEnd);

                // 交替绘制黑白虚线
                int dashIndex = static_cast<int>((currentPos + offset) / (dashLength + gapLength));
                ImU32 color = (dashIndex % 2 == 0) ? lineColor1 : lineColor2;

                drawList->AddLine(p1, p2, color, lineThickness);
            }

            currentPos += dashLength + gapLength;
        }
    };

    // 上边
    drawDashedLine(x1, y1, x2, y1);
    // 右边
    drawDashedLine(x2, y1, x2, y2);
    // 下边
    drawDashedLine(x2, y2, x1, y2);
    // 左边
    drawDashedLine(x1, y2, x1, y1);
}

void SelectionSystem::RenderMask(ImDrawList* drawList, const ImVec2& canvasMin, 
                                 const ImVec2& canvasMax, const SelectionRect& rect) {
    // 半透明遮罩：非选区区域暗化
    const ImU32 maskColor = IM_COL32(0, 0, 0, 100);  // 半透明黑色

    SelectionRect norm = rect.GetNormalized();
    float selX1 = norm.x;
    float selY1 = norm.y;
    float selX2 = norm.x + norm.width;
    float selY2 = norm.y + norm.height;

    // 绘制四个矩形遮罩（选区外的区域）
    // 上方
    if (selY1 > canvasMin.y) {
        drawList->AddRectFilled(canvasMin, ImVec2(canvasMax.x, selY1), maskColor);
    }
    // 下方
    if (selY2 < canvasMax.y) {
        drawList->AddRectFilled(ImVec2(canvasMin.x, selY2), canvasMax, maskColor);
    }
    // 左侧
    if (selX1 > canvasMin.x) {
        drawList->AddRectFilled(ImVec2(canvasMin.x, selY1), ImVec2(selX1, selY2), maskColor);
    }
    // 右侧
    if (selX2 < canvasMax.x) {
        drawList->AddRectFilled(ImVec2(selX2, selY1), ImVec2(canvasMax.x, selY2), maskColor);
    }
}

void SelectionSystem::ClearSelection() {
    // 清空前先提交到历史记录
    if (m_Selection.active) {
        SelectionRect empty;
        empty.Clear();
        m_History.Push(empty);
    }
    
    m_Selection.Clear();
    m_IsCreating = false;
    m_IsMoving = false;
}

void SelectionSystem::CommitSelection() {
    // 提交当前选区到历史记录
    if (m_Selection.active) {
        m_History.Push(m_Selection);
    }
}

void SelectionSystem::Undo() {
    if (m_History.CanUndo()) {
        m_Selection = m_History.Undo();
        m_IsCreating = false;
        m_IsMoving = false;
    }
}

void SelectionSystem::Redo() {
    if (m_History.CanRedo()) {
        m_Selection = m_History.Redo();
        m_IsCreating = false;
        m_IsMoving = false;
    }
}

void SelectionSystem::ApplySnapOffset(const ImVec2& offset) {
    if (m_Selection.active && (offset.x != 0.0f || offset.y != 0.0f)) {
        m_Selection.x += offset.x;
        m_Selection.y += offset.y;
    }
}

void SelectionSystem::ClampSelectionToLayer(const SelectionRect& layerBounds) {
    if (!m_Selection.active || !layerBounds.IsValid()) {
        printf("ClampSelectionToLayer: Invalid input - selection.active=%d, layerBounds.IsValid()=%d\n",
               m_Selection.active, layerBounds.IsValid());
        return;
    }

    // ✅ PS 风格：结算式选区收缩（遵循你的需求文档）
    // 核心：拖拽时自由，松手后理智
    
    // 获取标准化选区
    SelectionRect norm = m_Selection.GetNormalized();
    
    printf("\n=== ClampSelectionToLayer DETAILED DEBUG ===\n");
    printf("BEFORE clamp - Selection (normalized):\n");
    printf("  x=%.2f, y=%.2f, width=%.2f, height=%.2f\n", norm.x, norm.y, norm.width, norm.height);
    printf("  LEFT=%.2f, RIGHT=%.2f, TOP=%.2f, BOTTOM=%.2f\n",
           norm.x, norm.x + norm.width, norm.y, norm.y + norm.height);
    
    printf("\nLayer bounds:\n");
    printf("  x=%.2f, y=%.2f, width=%.2f, height=%.2f\n",
           layerBounds.x, layerBounds.y, layerBounds.width, layerBounds.height);
    printf("  LEFT=%.2f, RIGHT=%.2f, TOP=%.2f, BOTTOM=%.2f\n",
           layerBounds.x, layerBounds.x + layerBounds.width,
           layerBounds.y, layerBounds.y + layerBounds.height);
    
    // 将 SelectionRect 转换为 SelectionMath 格式
    SelectionMath::SelectionRect rawSelection = SelectionMath::CreateSelectionRect(
        norm.x, norm.y, norm.width, norm.height
    );
    
    SelectionMath::LayerPixelBounds layer = SelectionMath::CreateLayerBounds(
        layerBounds.x, layerBounds.y, layerBounds.width, layerBounds.height
    );
    
    // 计算交集（选区自动收缩到图层范围）
    SelectionMath::IntersectionRect intersection = SelectionMath::ComputeIntersection(
        rawSelection, layer
    );
    
    printf("\nAFTER clamp - Intersection result:\n");
    printf("  x=%.2f, y=%.2f, width=%.2f, height=%.2f\n",
           intersection.x, intersection.y, intersection.width, intersection.height);
    printf("  LEFT=%.2f, RIGHT=%.2f, TOP=%.2f, BOTTOM=%.2f\n",
           intersection.x, intersection.x + intersection.width,
           intersection.y, intersection.y + intersection.height);
    printf("  IsValid=%d\n", intersection.IsValid());
    printf("==========================================\n\n");
    
    // 如果交集无效（选区完全在图层外），清空选区
    if (!intersection.IsValid()) {
        m_Selection.Clear();
        return;
    }
    
    // 更新选区为裁剪后的结果
    m_Selection.x = intersection.x;
    m_Selection.y = intersection.y;
    m_Selection.width = intersection.width;
    m_Selection.height = intersection.height;
    m_Selection.active = true;
}

void SelectionSystem::MoveSelectionByOffset(float offsetX, float offsetY) {
    if (!m_Selection.active) {
        return;
    }
    
    // 移动选区（保持宽高不变）
    m_Selection.x += offsetX;
    m_Selection.y += offsetY;
    
    printf("[MoveSelectionByOffset] Moved selection by (%.2f, %.2f)\n", offsetX, offsetY);
}

