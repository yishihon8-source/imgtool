#include "CropInteraction.h"
#include <cmath>
#include <algorithm>

CropInteraction::CropInteraction() = default;
CropInteraction::~CropInteraction() = default;

void CropInteraction::Update(CropTool& cropTool,
                             const ImVec2& mousePos,
                             const ImVec2& canvasMin,
                             const ImVec2& canvasMax,
                             const ImVec2& canvasLogicalSize,
                             bool isMouseDown,
                             bool isMouseClicked,
                             bool isMouseReleased,
                             bool shiftPressed,
                             bool altPressed) {
    // 转换鼠标位置到画布逻辑坐标
    ImVec2 mouseLogical = ScreenToCanvas(mousePos, canvasMin, canvasMax, canvasLogicalSize);
    
    CropToolState state = cropTool.GetState();
    
    // 根据当前状态处理交互
    switch (state) {
        case CropToolState::Inactive:
            // 未激活，不处理
            break;
            
        case CropToolState::Creating:
            // 正在创建裁剪框
            HandleCreating(cropTool, mouseLogical, isMouseDown, isMouseReleased, shiftPressed, altPressed);
            break;
            
        case CropToolState::Active:
            // 裁剪框已创建，检测鼠标悬停和点击
            {
                // 计算8个控制点的屏幕坐标
                ImVec2 handles[8];
                CalculateHandlePositions(cropTool.GetCropRect(), canvasMin, canvasMax, canvasLogicalSize, handles);
                
                // HitTest 控制点
                m_HoveredHandle = HitTestHandles(mousePos, handles);
                
                // 更新鼠标光标
                UpdateMouseCursor(m_HoveredHandle);
                
                // 鼠标点击
                if (isMouseClicked) {
                    if (m_HoveredHandle != CropHandle::None) {
                        // 点击了控制点，进入调整大小模式
                        cropTool.SetDraggingHandle(m_HoveredHandle);
                        cropTool.SaveDragStartState();
                        cropTool.SetState(CropToolState::Resizing);
                        m_DragStartMousePos = mouseLogical;
                    } else {
                        // 检测是否点击了裁剪框内部（移动）
                        CropRect normalized = cropTool.GetCropRect().GetNormalized();
                        ImVec2 cropMin = CanvasToScreen(
                            ImVec2(static_cast<float>(normalized.left), static_cast<float>(normalized.top)),
                            canvasMin, canvasMax, canvasLogicalSize
                        );
                        ImVec2 cropMax = CanvasToScreen(
                            ImVec2(static_cast<float>(normalized.right), static_cast<float>(normalized.bottom)),
                            canvasMin, canvasMax, canvasLogicalSize
                        );
                        
                        if (HitTestCropRect(mousePos, cropMin, cropMax)) {
                            // 点击了裁剪框内部，进入移动模式
                            cropTool.SetDraggingHandle(CropHandle::Center);
                            cropTool.SaveDragStartState();
                            cropTool.SetState(CropToolState::Moving);
                            m_DragStartMousePos = mouseLogical;
                        }
                    }
                }
            }
            break;
            
        case CropToolState::Resizing:
            // 正在调整大小
            HandleResizing(cropTool, mouseLogical, isMouseDown, shiftPressed, altPressed);
            
            if (isMouseReleased) {
                cropTool.SetState(CropToolState::Active);
                cropTool.SetDraggingHandle(CropHandle::None);
            }
            break;
            
        case CropToolState::Moving:
            // 正在移动
            HandleMoving(cropTool, mouseLogical, isMouseDown);
            
            if (isMouseReleased) {
                cropTool.SetState(CropToolState::Active);
                cropTool.SetDraggingHandle(CropHandle::None);
            }
            break;
    }
}

void CropInteraction::HandleCreating(CropTool& cropTool,
                                    const ImVec2& mouseLogical,
                                    bool isMouseDown,
                                    bool isMouseReleased,
                                    bool shiftPressed,
                                    bool altPressed) {
    if (!isMouseDown) {
        // 鼠标松开，完成创建
        if (isMouseReleased) {
            cropTool.SetState(CropToolState::Active);
        }
        return;
    }
    
    // 拖拽创建裁剪框
    CropRect& cropRect = const_cast<CropRect&>(cropTool.GetCropRect());
    
    // Alt 键：从中心扩展
    if (altPressed) {
        float width = std::abs(mouseLogical.x - m_DragStartMousePos.x) * 2.0f;
        float height = std::abs(mouseLogical.y - m_DragStartMousePos.y) * 2.0f;
        
        // Shift 键：正方形
        if (shiftPressed) {
            float size = std::max(width, height);
            width = size;
            height = size;
        }
        
        cropRect.left = m_DragStartMousePos.x - width * 0.5;
        cropRect.right = m_DragStartMousePos.x + width * 0.5;
        cropRect.top = m_DragStartMousePos.y - height * 0.5;
        cropRect.bottom = m_DragStartMousePos.y + height * 0.5;
    } else {
        // 普通拖拽
        cropRect.left = m_DragStartMousePos.x;
        cropRect.top = m_DragStartMousePos.y;
        cropRect.right = mouseLogical.x;
        cropRect.bottom = mouseLogical.y;
        
        // Shift 键：正方形
        if (shiftPressed) {
            float width = cropRect.right - cropRect.left;
            float height = cropRect.bottom - cropRect.top;
            float size = std::max(std::abs(width), std::abs(height));
            
            cropRect.right = cropRect.left + (width >= 0 ? size : -size);
            cropRect.bottom = cropRect.top + (height >= 0 ? size : -size);
        }
    }
}

void CropInteraction::HandleResizing(CropTool& cropTool,
                                    const ImVec2& mouseLogical,
                                    bool isMouseDown,
                                    bool shiftPressed,
                                    bool altPressed) {
    (void)isMouseDown;  // 避免未使用警告
    
    CropHandle handle = cropTool.GetDraggingHandle();
    if (handle == CropHandle::None) {
        return;
    }
    
    // 计算鼠标移动距离
    ImVec2 delta(
        mouseLogical.x - m_DragStartMousePos.x,
        mouseLogical.y - m_DragStartMousePos.y
    );
    
    // 获取初始矩形
    CropRect startRect = cropTool.GetDragStartRect();
    CropRect newRect = startRect;
    
    // 根据控制点调整矩形
    switch (handle) {
        case CropHandle::TopLeft:
            newRect.left = startRect.left + delta.x;
            newRect.top = startRect.top + delta.y;
            break;
            
        case CropHandle::TopCenter:
            newRect.top = startRect.top + delta.y;
            break;
            
        case CropHandle::TopRight:
            newRect.right = startRect.right + delta.x;
            newRect.top = startRect.top + delta.y;
            break;
            
        case CropHandle::MiddleRight:
            newRect.right = startRect.right + delta.x;
            break;
            
        case CropHandle::BottomRight:
            newRect.right = startRect.right + delta.x;
            newRect.bottom = startRect.bottom + delta.y;
            break;
            
        case CropHandle::BottomCenter:
            newRect.bottom = startRect.bottom + delta.y;
            break;
            
        case CropHandle::BottomLeft:
            newRect.left = startRect.left + delta.x;
            newRect.bottom = startRect.bottom + delta.y;
            break;
            
        case CropHandle::MiddleLeft:
            newRect.left = startRect.left + delta.x;
            break;
            
        default:
            break;
    }
    
    // Shift 键：保持宽高比
    if (shiftPressed) {
        double startWidth = startRect.GetWidth();
        double startHeight = startRect.GetHeight();
        double aspectRatio = startWidth / startHeight;
        
        // 根据控制点类型调整
        bool isCorner = (handle == CropHandle::TopLeft || handle == CropHandle::TopRight ||
                        handle == CropHandle::BottomLeft || handle == CropHandle::BottomRight);
        
        if (isCorner) {
            // 角点：等比例缩放
            double newWidth = newRect.GetWidth();
            double newHeight = newRect.GetHeight();
            
            // 以较大的变化为准
            if (std::abs(newWidth - startWidth) > std::abs(newHeight - startHeight)) {
                // 宽度变化更大，根据宽度计算高度
                newHeight = newWidth / aspectRatio;
            } else {
                // 高度变化更大，根据高度计算宽度
                newWidth = newHeight * aspectRatio;
            }
            
            // 根据控制点更新矩形
            switch (handle) {
                case CropHandle::TopLeft:
                    newRect.left = newRect.right - newWidth;
                    newRect.top = newRect.bottom - newHeight;
                    break;
                case CropHandle::TopRight:
                    newRect.right = newRect.left + newWidth;
                    newRect.top = newRect.bottom - newHeight;
                    break;
                case CropHandle::BottomLeft:
                    newRect.left = newRect.right - newWidth;
                    newRect.bottom = newRect.top + newHeight;
                    break;
                case CropHandle::BottomRight:
                    newRect.right = newRect.left + newWidth;
                    newRect.bottom = newRect.top + newHeight;
                    break;
                default:
                    break;
            }
        } else {
            // 边缘中点：等比例缩放，保持中心
            double centerX = startRect.GetCenterX();
            double centerY = startRect.GetCenterY();
            
            if (handle == CropHandle::TopCenter || handle == CropHandle::BottomCenter) {
                // 垂直调整
                double newHeight = newRect.GetHeight();
                double newWidth = newHeight * aspectRatio;
                newRect.left = centerX - newWidth * 0.5;
                newRect.right = centerX + newWidth * 0.5;
            } else {
                // 水平调整
                double newWidth = newRect.GetWidth();
                double newHeight = newWidth / aspectRatio;
                newRect.top = centerY - newHeight * 0.5;
                newRect.bottom = centerY + newHeight * 0.5;
            }
        }
    }
    
    // Alt 键：从中心缩放（暂不实现，可选功能）
    (void)altPressed;
    
    // 更新裁剪矩形
    cropTool.SetCropRect(newRect);
}

void CropInteraction::HandleMoving(CropTool& cropTool,
                                  const ImVec2& mouseLogical,
                                  bool isMouseDown) {
    (void)isMouseDown;  // 避免未使用警告
    
    // 计算鼠标移动距离
    ImVec2 delta(
        mouseLogical.x - m_DragStartMousePos.x,
        mouseLogical.y - m_DragStartMousePos.y
    );
    
    // 获取初始矩形
    CropRect startRect = cropTool.GetDragStartRect();
    CropRect newRect = startRect;
    
    // 平移矩形
    newRect.left = startRect.left + delta.x;
    newRect.right = startRect.right + delta.x;
    newRect.top = startRect.top + delta.y;
    newRect.bottom = startRect.bottom + delta.y;
    
    // 更新裁剪矩形
    cropTool.SetCropRect(newRect);
}

void CropInteraction::HandleKeyboardInput(CropTool& cropTool, int canvasWidth, int canvasHeight) {
    if (!cropTool.IsActive()) {
        return;
    }
    
    ImGuiIO& io = ImGui::GetIO();
    
    // 方向键移动裁剪框
    const float moveStep = 1.0f;  // 每次移动1像素
    const float fastMoveStep = 10.0f;  // Shift 加速
    
    float step = io.KeyShift ? fastMoveStep : moveStep;
    
    CropRect cropRect = cropTool.GetCropRect();
    bool moved = false;
    
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        cropRect.left -= step;
        cropRect.right -= step;
        moved = true;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        cropRect.left += step;
        cropRect.right += step;
        moved = true;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        cropRect.top -= step;
        cropRect.bottom -= step;
        moved = true;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        cropRect.top += step;
        cropRect.bottom += step;
        moved = true;
    }
    
    if (moved) {
        // 限制在画布范围内
        double width = cropRect.GetWidth();
        double height = cropRect.GetHeight();
        
        if (cropRect.left < 0) {
            cropRect.left = 0;
            cropRect.right = width;
        }
        if (cropRect.right > canvasWidth) {
            cropRect.right = canvasWidth;
            cropRect.left = canvasWidth - width;
        }
        if (cropRect.top < 0) {
            cropRect.top = 0;
            cropRect.bottom = height;
        }
        if (cropRect.bottom > canvasHeight) {
            cropRect.bottom = canvasHeight;
            cropRect.top = canvasHeight - height;
        }
        
        cropTool.SetCropRect(cropRect);
    }
}

CropHandle CropInteraction::HitTestHandles(const ImVec2& mousePos, const ImVec2 handles[8]) const {
    for (int i = 0; i < 8; i++) {
        float dist = std::sqrt(
            std::pow(mousePos.x - handles[i].x, 2.0f) +
            std::pow(mousePos.y - handles[i].y, 2.0f)
        );
        
        if (dist < m_HandleHitRadius) {
            return static_cast<CropHandle>(i);
        }
    }
    
    return CropHandle::None;
}

bool CropInteraction::HitTestCropRect(const ImVec2& mousePos,
                                     const ImVec2& cropMin,
                                     const ImVec2& cropMax) const {
    return mousePos.x >= cropMin.x && mousePos.x <= cropMax.x &&
           mousePos.y >= cropMin.y && mousePos.y <= cropMax.y;
}

void CropInteraction::UpdateMouseCursor(CropHandle handle) const {
    switch (handle) {
        case CropHandle::TopLeft:
        case CropHandle::BottomRight:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
            break;
            
        case CropHandle::TopRight:
        case CropHandle::BottomLeft:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
            break;
            
        case CropHandle::TopCenter:
        case CropHandle::BottomCenter:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            break;
            
        case CropHandle::MiddleLeft:
        case CropHandle::MiddleRight:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            break;
            
        default:
            break;
    }
}

ImVec2 CropInteraction::ScreenToCanvas(const ImVec2& screenPos,
                                      const ImVec2& canvasMin,
                                      const ImVec2& canvasMax,
                                      const ImVec2& canvasLogicalSize) const {
    float scaleX = (canvasMax.x - canvasMin.x) / canvasLogicalSize.x;
    float scaleY = (canvasMax.y - canvasMin.y) / canvasLogicalSize.y;
    
    return ImVec2(
        (screenPos.x - canvasMin.x) / scaleX,
        (screenPos.y - canvasMin.y) / scaleY
    );
}

ImVec2 CropInteraction::CanvasToScreen(const ImVec2& canvasPos,
                                      const ImVec2& canvasMin,
                                      const ImVec2& canvasMax,
                                      const ImVec2& canvasLogicalSize) const {
    float scaleX = (canvasMax.x - canvasMin.x) / canvasLogicalSize.x;
    float scaleY = (canvasMax.y - canvasMin.y) / canvasLogicalSize.y;
    
    return ImVec2(
        canvasMin.x + canvasPos.x * scaleX,
        canvasMin.y + canvasPos.y * scaleY
    );
}

void CropInteraction::CalculateHandlePositions(const CropRect& cropRect,
                                              const ImVec2& canvasMin,
                                              const ImVec2& canvasMax,
                                              const ImVec2& canvasLogicalSize,
                                              ImVec2 outHandles[8]) const {
    CropRect normalized = cropRect.GetNormalized();
    
    float scaleX = (canvasMax.x - canvasMin.x) / canvasLogicalSize.x;
    float scaleY = (canvasMax.y - canvasMin.y) / canvasLogicalSize.y;
    
    float left = canvasMin.x + static_cast<float>(normalized.left) * scaleX;
    float right = canvasMin.x + static_cast<float>(normalized.right) * scaleX;
    float top = canvasMin.y + static_cast<float>(normalized.top) * scaleY;
    float bottom = canvasMin.y + static_cast<float>(normalized.bottom) * scaleY;
    float centerX = (left + right) * 0.5f;
    float centerY = (top + bottom) * 0.5f;
    
    outHandles[0] = ImVec2(left, top);           // TopLeft
    outHandles[1] = ImVec2(centerX, top);        // TopCenter
    outHandles[2] = ImVec2(right, top);          // TopRight
    outHandles[3] = ImVec2(right, centerY);      // MiddleRight
    outHandles[4] = ImVec2(right, bottom);       // BottomRight
    outHandles[5] = ImVec2(centerX, bottom);     // BottomCenter
    outHandles[6] = ImVec2(left, bottom);        // BottomLeft
    outHandles[7] = ImVec2(left, centerY);       // MiddleLeft
}

