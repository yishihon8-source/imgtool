#include "PreviewPanel.h"
#include "core/ImageLoader.h"
#include "core/TransformManager.h"
#include "core/GuideLineManager.h"
#include <imgui.h>
#include <algorithm>
#include <iostream>

// OpenGL
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GLFW/glfw3.h>

// OpenGL 常量定义（如果系统头文件中没有）
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

PreviewPanel::PreviewPanel() = default;

PreviewPanel::~PreviewPanel() {
    ReleaseTexture();
}

void PreviewPanel::Render(std::vector<ImageInfo>& imageList,
                           int currentIndex,
                           ProcessConfig& config,
                           bool canvasApplied,
                           bool& transformMode,
                           bool& selectionMode) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("画布", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();
    
    // 保存输入状态
    bool inputTransformMode = transformMode;
    bool inputSelectionMode = selectionMode;
    
    // 检测外部是否主动改变了状态（比如点击工具栏按钮）
    // 只有当外部状态与上一帧的输出状态不同时，才认为是外部主动改变
    static bool lastOutputTransformMode = false;
    static bool lastOutputSelectionMode = false;
    
    bool externalChange = (inputTransformMode != lastOutputTransformMode) || 
                          (inputSelectionMode != lastOutputSelectionMode);
    
    // 只在外部主动改变时才同步到内部（避免每帧覆盖）
    if (externalChange) {
        m_TransformMode = transformMode;
        m_SelectionMode = selectionMode;
    }

    // Ctrl+T 切换变换模式（全局快捷键，不需要窗口焦点）
    // 使用 ImGui::IsKeyPressed 而不是窗口焦点检测
    ImGuiIO& io = ImGui::GetIO();
    bool ctrlPressed = io.KeyCtrl;
    bool tPressed = ImGui::IsKeyPressed(ImGuiKey_T, false);
    
    if (ctrlPressed && tPressed) {
        if (!m_TransformMode) {
            // ✅ 进入变换模式
            m_TransformMode = true;
            m_SelectionMode = false;
            m_HasSelection = false;
        } else {
            // ✅ 退出变换模式（相当于确认变换）
            m_TransformMode = false;
            // 保持当前的变换矩形状态，不重置
        }
    }
    
    // M 键切换选区模式（类似 PS 的选框工具）
    if (ImGui::IsKeyPressed(ImGuiKey_M, false) && !m_TransformMode) {
        m_SelectionMode = !m_SelectionMode;
        if (!m_SelectionMode) {
            m_HasSelection = false;
        }
    }
    
    // ✅ Delete / Backspace 删除选区内容（PS 风格）
    // 只在选区模式下且有活动选区时生效
    if (m_SelectionMode && m_SelectionSystem.HasActiveSelection()) {
        if (ImGui::IsKeyPressed(ImGuiKey_Delete, false) || 
            ImGui::IsKeyPressed(ImGuiKey_Backspace, false)) {
            printf("\n[Shortcut] Delete/Backspace pressed, deleting selection content...\n");
            DeleteSelectionContent(config);
        }
    }
    
    // ✅ Ctrl+Z 撤销（全局快捷键）
    if (ctrlPressed && ImGui::IsKeyPressed(ImGuiKey_Z, false) && !io.KeyShift) {
        printf("\n[Shortcut] Ctrl+Z pressed, undoing...\n");
        Undo();
    }
    
    // ✅ Ctrl+Shift+Z 重做（全局快捷键）
    if (ctrlPressed && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
        printf("\n[Shortcut] Ctrl+Shift+Z pressed, redoing...\n");
        Redo();
    }
    
    // 变换模式下的快捷键
    if (m_TransformMode) {
        // Enter 确认变换
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
            // ✅ 确认变换：保持当前的变换矩形状态
            m_TransformMode = false;
            // 变换矩形状态已经保存在 m_TransformRect 中，会在切换图片时自动保存
        }
        
        // ESC 取消变换
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            m_TransformMode = false;
            // ✅ 取消变换：重置变换矩形到初始状态
            ResetTransform();
        }
        
        // 方向键移动图片（使用 PS 矩形模型）
        const float moveStep = 1.0f;  // 每次移动1像素（画布逻辑坐标）
        const float fastMoveStep = 10.0f;  // 按住 Shift 时快速移动10像素
        
        bool shiftPressed = io.KeyShift;
        float step = shiftPressed ? fastMoveStep : moveStep;
        
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            m_TransformRect.left -= step;
            m_TransformRect.right -= step;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
            m_TransformRect.left += step;
            m_TransformRect.right += step;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            m_TransformRect.top -= step;
            m_TransformRect.bottom -= step;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            m_TransformRect.top += step;
            m_TransformRect.bottom += step;
        }
    }
    
    // 选区模式下的快捷键（需要在渲染前处理，以便传递正确的图片边界）
    bool shouldEraseSelection = false;
    if (m_SelectionMode && m_HasSelection) {
        // Backspace 或 Delete 擦除选区
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) || ImGui::IsKeyPressed(ImGuiKey_Delete)) {
            shouldEraseSelection = true;
        }
        
        // ESC 取消选区
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            m_SelectionSystem.ClearSelection();
            m_HasSelection = false;
        }
    }
    
    // 选区模式下的 Undo/Redo（Ctrl+Z / Ctrl+Y）
    if (m_SelectionMode) {
        // Ctrl+Z 撤销
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false) && !io.KeyShift) {
            m_SelectionSystem.Undo();
            m_HasSelection = m_SelectionSystem.HasActiveSelection();
        }
        // Ctrl+Shift+Z 或 Ctrl+Y 重做
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Y, false) || 
                          (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_Z, false)))) {
            m_SelectionSystem.Redo();
            m_HasSelection = m_SelectionSystem.HasActiveSelection();
        }
    }

    // Alt + 鼠标滚轮缩放画布视图（PS 级体验 - 以鼠标为中心）
    ImGuiIO& ioWheel = ImGui::GetIO();
    bool shouldZoomCanvas = ImGui::IsWindowHovered() && ioWheel.MouseWheel != 0.0f && 
                           ioWheel.KeyAlt && !ioWheel.KeyCtrl &&
                           !(ioWheel.KeyCtrl && ioWheel.KeyShift && m_TransformMode);  // 排除图片缩放快捷键
    
    if (shouldZoomCanvas) {
        // 保存鼠标位置和窗口信息
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 windowPos = ImGui::GetWindowPos();
        
        // 计算缩放前的画布中心位置（屏幕坐标）
        ImVec2 oldCanvasCenter;
        oldCanvasCenter.x = windowPos.x + windowSize.x * 0.5f + m_CanvasOffset.x;
        oldCanvasCenter.y = windowPos.y + (windowSize.y - 40) * 0.5f + m_CanvasOffset.y;
        
        // 计算鼠标相对于画布中心的向量
        ImVec2 mouseToCenter;
        mouseToCenter.x = mousePos.x - oldCanvasCenter.x;
        mouseToCenter.y = mousePos.y - oldCanvasCenter.y;
        
        // 计算缩放因子
        float scaleFactor = (ioWheel.MouseWheel > 0) ? 1.1f : 0.9f;
        float oldZoom = m_CanvasZoom;
        m_CanvasZoom *= scaleFactor;
        m_CanvasZoom = std::max(0.1f, std::min(m_CanvasZoom, 8.0f));  // 限制在 10% - 800%
        
        // 计算实际应用的缩放因子（可能被边界限制）
        float actualScaleFactor = m_CanvasZoom / oldZoom;
        
        // 以鼠标为中心进行缩放
        // 核心思想：鼠标相对于画布中心的向量也会随着缩放而缩放
        // 新的画布中心 = 鼠标位置 - 缩放后的向量
        ImVec2 newMouseToCenter;
        newMouseToCenter.x = mouseToCenter.x * actualScaleFactor;
        newMouseToCenter.y = mouseToCenter.y * actualScaleFactor;
        
        ImVec2 newCanvasCenter;
        newCanvasCenter.x = mousePos.x - newMouseToCenter.x;
        newCanvasCenter.y = mousePos.y - newMouseToCenter.y;
        
        // 更新偏移量
        m_CanvasOffset.x = newCanvasCenter.x - (windowPos.x + windowSize.x * 0.5f);
        m_CanvasOffset.y = newCanvasCenter.y - (windowPos.y + (windowSize.y - 40) * 0.5f);
    }
    
    // Ctrl + 鼠标滚轮左右移动画布（在所有模式下都可用）
    bool shouldPanCanvas = ImGui::IsWindowHovered() && ioWheel.MouseWheel != 0.0f && 
                          ioWheel.KeyCtrl && !ioWheel.KeyShift;
    
    if (shouldPanCanvas) {
        // 滚轮向上：向右移动（画布向左）
        // 滚轮向下：向左移动（画布向右）
        float panAmount = ioWheel.MouseWheel * 50.0f;  // 每次滚动移动50像素
        m_CanvasOffset.x += panAmount;
    }
    
    // 单纯的鼠标滚轮上下移动画布（没有任何修饰键）
    bool shouldScrollCanvas = ImGui::IsWindowHovered() && ioWheel.MouseWheel != 0.0f && 
                             !ioWheel.KeyCtrl && !ioWheel.KeyAlt && !ioWheel.KeyShift;
    
    if (shouldScrollCanvas) {
        // 滚轮向上：画布向上移动（偏移量增加）
        // 滚轮向下：画布向下移动（偏移量减少）
        float scrollAmount = ioWheel.MouseWheel * 50.0f;  // 每次滚动移动50像素
        m_CanvasOffset.y += scrollAmount;
    }
    
    // 空格键 + 鼠标拖拽画布（PS 抓手工具 - 在所有模式下都可用）
    bool spacePressed = ImGui::IsKeyDown(ImGuiKey_Space);
    if (ImGui::IsWindowHovered() && spacePressed && !m_SelectionMode) {
        // 设置抓手光标（小手）
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        
        // 开始拖拽
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_IsDraggingCanvas = true;
            m_CanvasDragStart = ImGui::GetMousePos();
        }
    }
    
    // 拖拽画布
    if (m_IsDraggingCanvas) {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && spacePressed) {
            ImVec2 currentPos = ImGui::GetMousePos();
            ImVec2 delta(currentPos.x - m_CanvasDragStart.x, currentPos.y - m_CanvasDragStart.y);
            // 向上拖 → 画布向上移动（偏移量增加）
            // 向下拖 → 画布向下移动（偏移量减少）
            m_CanvasOffset.x += delta.x;
            m_CanvasOffset.y += delta.y;
            m_CanvasDragStart = currentPos;
            
            // 拖拽时也显示抓手光标
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        
        // 结束拖拽（松开鼠标或松开空格键）
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || !spacePressed) {
            m_IsDraggingCanvas = false;
        }
    }

    // 如果画布未应用，但有图片，显示提示并继续显示图片预览
    if (!canvasApplied) {
        if (imageList.empty()) {
            // 没有图片也没有应用画布，显示完整提示
            RenderEmptyState("请在右侧设置画布尺寸并点击\"应用画布\"", ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::End();
            return;
        }
        // 如果有图片，继续显示图片预览（带一个小提示）
        // 画布应用提示会显示在画布上方
    }

    // 画布已应用或有图片，显示画布
    if (currentIndex >= 0 && currentIndex < static_cast<int>(imageList.size())) {
        auto& info = imageList[currentIndex];

        // 检测是否切换了图片
        bool imageChanged = (m_CurrentImagePath != info.filePath);
        
        // 如果切换了图片，先保存旧图片的变换状态
        if (imageChanged && m_LastImageIndex >= 0 && m_LastImageIndex < static_cast<int>(imageList.size())) {
            SaveTransformState(imageList[m_LastImageIndex]);
        }
        
        // 加载图片
        if (imageChanged) {
            LoadCurrentImage(info.filePath);
            // 恢复这张图片的变换状态（如果有的话）
            RestoreTransformState(info);
            m_LastImageIndex = currentIndex;
        }

        if (m_CurrentImage.IsValid()) {
            // 如果画布未应用，显示提示信息
            if (!canvasApplied) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
                ImGui::SetCursorPos(ImVec2(10, 10));
                ImGui::Text("请应用画布以进行批处理");
                ImGui::PopStyleColor();
            }
            
            RenderCanvasStage(m_CurrentImage, config, shouldEraseSelection);
        } else {
            // 加载失败，但仍然显示空画布
            ImageData emptyImage;
            RenderCanvasStage(emptyImage, config, false);
        }
    } else {
        // 没有选中图片，显示空画布
        ImageData emptyImage;
        RenderCanvasStage(emptyImage, config, false);
    }
    
    // 显示变换模式提示
    if (m_TransformMode) {
        ImGui::SetCursorPos(ImVec2(10, 10));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
        ImGui::Text("变换模式 | Enter: 确认 | ESC: 取消 | Shift: 等比例");
        ImGui::PopStyleColor();
    }
    
    // 显示选区模式提示
    if (m_SelectionMode) {
        ImGui::SetCursorPos(ImVec2(10, 10));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
        if (m_HasSelection) {
            ImGui::Text("选区模式 | Backspace/Delete: 擦除 | ESC: 取消选区 | Ctrl+Z: 撤销 | Ctrl+Y: 重做 | M: 退出");
        } else {
            ImGui::Text("选区模式 | 拖拽鼠标创建选区 | Shift: 正方形 | Alt: 中心扩展 | Ctrl+Z: 撤销 | M: 退出");
        }
        ImGui::PopStyleColor();
    }

    // 同步状态回 MainUI（只在状态真正改变时才同步）
    if (m_TransformMode != inputTransformMode || m_SelectionMode != inputSelectionMode) {
        transformMode = m_TransformMode;
        selectionMode = m_SelectionMode;
    }
    
    // 保存输出状态供下一帧使用
    lastOutputTransformMode = m_TransformMode;
    lastOutputSelectionMode = m_SelectionMode;

    ImGui::End();
}

void PreviewPanel::RenderCanvasStage(const ImageData& image, ProcessConfig& config, bool shouldEraseSelection) {
    try {
        // 避免未使用参数警告
        (void)image;
        
        ImVec2 windowSize = ImGui::GetWindowSize();
        
        // 验证窗口有效
        if (windowSize.x <= 0 || windowSize.y <= 0) {
            std::cerr << "Invalid window size: " << windowSize.x << "x" << windowSize.y << std::endl;
            return;
        }
        
        // 画布舞台背景（深灰色棋盘格）
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (!drawList) {
            std::cerr << "Failed to get draw list" << std::endl;
            return;
        }

        ImVec2 windowPos = ImGui::GetWindowPos();
    
    // 绘制棋盘格背景
    const float gridSize = 16.0f;
    const ImU32 color1 = IM_COL32(30, 30, 30, 255);
    const ImU32 color2 = IM_COL32(40, 40, 40, 255);
    
    for (float y = 0; y < windowSize.y; y += gridSize) {
        for (float x = 0; x < windowSize.x; x += gridSize) {
            int gridX = static_cast<int>(x / gridSize);
            int gridY = static_cast<int>(y / gridSize);
            ImU32 color = ((gridX + gridY) % 2 == 0) ? color1 : color2;
            
            drawList->AddRectFilled(
                ImVec2(windowPos.x + x, windowPos.y + y),
                ImVec2(windowPos.x + x + gridSize, windowPos.y + y + gridSize),
                color
            );
        }
    }

    // 计算画布显示尺寸（应用缩放，保持画布比例，居中显示）
    float canvasAspect = static_cast<float>(config.canvas.width) / config.canvas.height;
    float padding = 0.0f;
    
    float maxWidth = windowSize.x - padding * 2;
    float maxHeight = windowSize.y - padding * 2 - 40; // 留出底部标签空间
    
    // 应用缩放
    float canvasDisplayWidth = maxWidth * m_CanvasZoom;
    float canvasDisplayHeight = canvasDisplayWidth / canvasAspect;

    if (canvasDisplayHeight > maxHeight * m_CanvasZoom) {
        canvasDisplayHeight = maxHeight * m_CanvasZoom;
        canvasDisplayWidth = canvasDisplayHeight * canvasAspect;
    }

    // 画布位置（居中 + 偏移）
    float canvasX = windowPos.x + (windowSize.x - canvasDisplayWidth) * 0.5f + m_CanvasOffset.x;
    float canvasY = windowPos.y + (windowSize.y - canvasDisplayHeight - 40) * 0.5f + m_CanvasOffset.y;
    
    ImVec2 canvasMin(canvasX, canvasY);
    ImVec2 canvasMax(canvasX + canvasDisplayWidth, canvasY + canvasDisplayHeight);

    // 变换模式下：Ctrl + Shift + 滚轮缩放图片本身
    ImGuiIO& ioZoom = ImGui::GetIO();
    if (ImGui::IsWindowHovered() && ioZoom.MouseWheel != 0.0f && ioZoom.KeyCtrl && ioZoom.KeyShift && m_TransformMode) {
        float scaleFactor = (ioZoom.MouseWheel > 0) ? 1.1f : 0.9f;
        
        // ✅ PS 模型：以鼠标为中心缩放矩形
        ImVec2 mousePos = ImGui::GetMousePos();
        
        // 将鼠标位置转换为画布逻辑坐标
        double mouseLogicalX = (mousePos.x - canvasX) / (canvasDisplayWidth / config.canvas.width);
        double mouseLogicalY = (mousePos.y - canvasY) / (canvasDisplayHeight / config.canvas.height);
        
        // 计算鼠标相对于矩形中心的向量
        double centerX = m_TransformRect.GetCenterX();
        double centerY = m_TransformRect.GetCenterY();
        double vecX = mouseLogicalX - centerX;
        double vecY = mouseLogicalY - centerY;
        
        // 缩放矩形（保持宽高比）
        double oldWidth = m_TransformRect.GetWidth();
        double oldHeight = m_TransformRect.GetHeight();
        double newWidth = oldWidth * scaleFactor;
        double newHeight = oldHeight * scaleFactor;
        
        // 新的中心位置（使鼠标点保持不动）
        double newCenterX = mouseLogicalX - vecX * scaleFactor;
        double newCenterY = mouseLogicalY - vecY * scaleFactor;
        
        // 更新矩形
        m_TransformRect.left = newCenterX - newWidth * 0.5;
        m_TransformRect.right = newCenterX + newWidth * 0.5;
        m_TransformRect.top = newCenterY - newHeight * 0.5;
        m_TransformRect.bottom = newCenterY + newHeight * 0.5;
    }

    // 绘制画布背景
    uint8_t r = config.canvas.background.r;
    uint8_t g = config.canvas.background.g;
    uint8_t b = config.canvas.background.b;
    
    drawList->AddRectFilled(canvasMin, canvasMax, IM_COL32(r, g, b, 255));

    // 渲染图片纹理（根据 ScaleMode 计算图片在画布中的位置和尺寸）
    if (m_TextureID != 0 && m_TextureWidth > 0 && m_TextureHeight > 0) {
        // 计算图片在画布中的实际尺寸和位置
        float imageWidth = static_cast<float>(m_TextureWidth);
        float imageHeight = static_cast<float>(m_TextureHeight);
        
        // TODO: 如果有自定义边界（擦除后），使用实际可见区域的尺寸（旧擦除工具功能，已删除）
        // if (m_HasCustomBounds) {
        //     imageWidth = static_cast<float>(m_ImageBoundsMaxX - m_ImageBoundsMinX + 1);
        //     imageHeight = static_cast<float>(m_ImageBoundsMaxY - m_ImageBoundsMinY + 1);
        // }
        
        float imageAspect = imageWidth / imageHeight;
        
        float targetWidth = 0.0f;
        float targetHeight = 0.0f;
        
        // 根据 ScaleMode 计算基础目标尺寸（未变换）
        switch (config.scaleMode) {
            case ScaleMode::Fit: {
                // 适应：保持比例，完整显示
                if (imageAspect > canvasAspect) {
                    targetWidth = static_cast<float>(config.canvas.width);
                    targetHeight = targetWidth / imageAspect;
                } else {
                    targetHeight = static_cast<float>(config.canvas.height);
                    targetWidth = targetHeight * imageAspect;
                }
                break;
            }
            case ScaleMode::Fill: {
                // 填充：保持比例，填满画布（可能裁剪）
                if (imageAspect > canvasAspect) {
                    targetHeight = static_cast<float>(config.canvas.height);
                    targetWidth = targetHeight * imageAspect;
                } else {
                    targetWidth = static_cast<float>(config.canvas.width);
                    targetHeight = targetWidth / imageAspect;
                }
                break;
            }
            case ScaleMode::Stretch: {
                // 拉伸：填满画布，不保持比例
                targetWidth = static_cast<float>(config.canvas.width);
                targetHeight = static_cast<float>(config.canvas.height);
                break;
            }
            case ScaleMode::None:
            default: {
                // 原始尺寸
                targetWidth = imageWidth;
                targetHeight = imageHeight;
                break;
            }
        }
        
        // ✅ 保存原始图层边界（未变换的尺寸和位置）- 用于选区收缩
        float originalImageX, originalImageY, originalImageWidth, originalImageHeight;
        originalImageX = (config.canvas.width - targetWidth) * 0.5f;
        originalImageY = (config.canvas.height - targetHeight) * 0.5f;
        originalImageWidth = targetWidth;
        originalImageHeight = targetHeight;
        
        // ✅ PS 模型：如果有有效的变换矩形，始终使用它（无论是否在变换模式）
        float imageX, imageY;
        if (m_TransformRect.GetWidth() > 0 && m_TransformRect.GetHeight() > 0) {
            // 使用 PS 矩形模型（保持变换状态）
            imageX = static_cast<float>(m_TransformRect.left);
            imageY = static_cast<float>(m_TransformRect.top);
            targetWidth = static_cast<float>(m_TransformRect.GetWidth());
            targetHeight = static_cast<float>(m_TransformRect.GetHeight());
        } else {
            // 默认居中对齐（未变换状态）
            imageX = (config.canvas.width - targetWidth) * 0.5f;
            imageY = (config.canvas.height - targetHeight) * 0.5f;
            
            // ✅ 初始化变换矩形（无论是否在变换模式，都要初始化以便后续使用）
            m_TransformRect.left = imageX;
            m_TransformRect.top = imageY;
            m_TransformRect.right = imageX + targetWidth;
            m_TransformRect.bottom = imageY + targetHeight;
        }
        
        // 转换为显示坐标
        float scale = canvasDisplayWidth / config.canvas.width;
        float displayImageX = canvasX + imageX * scale;
        float displayImageY = canvasY + imageY * scale;
        float displayImageWidth = targetWidth * scale;
        float displayImageHeight = targetHeight * scale;
        
        ImVec2 imageMin(displayImageX, displayImageY);
        ImVec2 imageMax(displayImageX + displayImageWidth, displayImageY + displayImageHeight);
        
        // 保存原始图片边界（用于变换控制和越界检测）
        ImVec2 originalImageMin = imageMin;
        ImVec2 originalImageMax = imageMax;
        
        // ✅ 计算图层像素边界（画布逻辑坐标）供选区越界检测使用
        // 关键理解：
        // 图层像素边界 = 图层在画布中的原始范围（不随变换改变）
        // 在 Photoshop 中，这总是（0,0）到（canvas.width, canvas.height）
        // Ctrl+T 变换后，图片会超出这个范围，触发粉色警告线
        // 粉色警告线应该显示在画布边界（即原始图层范围）上
        
        // ✅ 图层边界始终是画布范围，不随图片变换而改变
        ImVec2 layerPixelBounds_Pos(0.0f, 0.0f);  // 图层永远从(0,0)开始
        ImVec2 layerPixelBounds_Size(static_cast<float>(config.canvas.width), 
                                      static_cast<float>(config.canvas.height));
        
        // 裁剪到画布范围内（仅用于渲染）
        ImVec2 clippedImageMin = imageMin;
        ImVec2 clippedImageMax = imageMax;
        clippedImageMin.x = std::max(clippedImageMin.x, canvasMin.x);
        clippedImageMin.y = std::max(clippedImageMin.y, canvasMin.y);
        clippedImageMax.x = std::min(clippedImageMax.x, canvasMax.x);
        clippedImageMax.y = std::min(clippedImageMax.y, canvasMax.y);
        
        // 计算对应的 UV 坐标
        float uvMinX = (clippedImageMin.x - displayImageX) / displayImageWidth;
        float uvMinY = (clippedImageMin.y - displayImageY) / displayImageHeight;
        float uvMaxX = (clippedImageMax.x - displayImageX) / displayImageWidth;
        float uvMaxY = (clippedImageMax.y - displayImageY) / displayImageHeight;
        
        // TODO: 如果有自定义边界，调整 UV 坐标以只显示可见区域（旧擦除工具功能，已删除）
        // if (m_HasCustomBounds) {
        //     float fullWidth = static_cast<float>(m_TextureWidth);
        //     float fullHeight = static_cast<float>(m_TextureHeight);
        //     float boundsWidth = static_cast<float>(m_ImageBoundsMaxX - m_ImageBoundsMinX + 1);
        //     float boundsHeight = static_cast<float>(m_ImageBoundsMaxY - m_ImageBoundsMinY + 1);
        //     
        //     // 将 UV 坐标映射到实际边界区域
        //     float uOffset = m_ImageBoundsMinX / fullWidth;
        //     float vOffset = m_ImageBoundsMinY / fullHeight;
        //     float uScale = boundsWidth / fullWidth;
        //     float vScale = boundsHeight / fullHeight;
        //     
        //     uvMinX = uOffset + uvMinX * uScale;
        //     uvMinY = vOffset + uvMinY * vScale;
        //     uvMaxX = uOffset + uvMaxX * uScale;
        //     uvMaxY = vOffset + uvMaxY * vScale;
        // }
        
        ImGui::GetWindowDrawList()->AddImage(
            (void*)(intptr_t)m_TextureID,
            clippedImageMin,
            clippedImageMax,
            ImVec2(uvMinX, uvMinY),
            ImVec2(uvMaxX, uvMaxY)
        );
        
        // 变换模式：显示控制点和对齐辅助线
        if (m_TransformMode) {
            // ✅ PS 风格：如果有有效内容边界，变换框只包含有效内容
            ImVec2 transformMin = originalImageMin;
            ImVec2 transformMax = originalImageMax;
            
            if (m_ValidContentBounds.isValid) {
                // 计算有效内容在当前变换矩形中的位置（画布逻辑坐标）
                float validLeft = static_cast<float>(m_TransformRect.left) + 
                                  m_ValidContentBounds.startX * static_cast<float>(m_TransformRect.GetWidth());
                float validTop = static_cast<float>(m_TransformRect.top) + 
                                 m_ValidContentBounds.startY * static_cast<float>(m_TransformRect.GetHeight());
                float validRight = static_cast<float>(m_TransformRect.left) + 
                                   m_ValidContentBounds.endX * static_cast<float>(m_TransformRect.GetWidth());
                float validBottom = static_cast<float>(m_TransformRect.top) + 
                                    m_ValidContentBounds.endY * static_cast<float>(m_TransformRect.GetHeight());
                
                // 转换为屏幕坐标
                transformMin.x = canvasX + validLeft * scale;
                transformMin.y = canvasY + validTop * scale;
                transformMax.x = canvasX + validRight * scale;
                transformMax.y = canvasY + validBottom * scale;
                
                printf("\n=== Transform Controls with Valid Bounds ===\n");
                printf("Valid bounds (normalized): (%.4f, %.4f) to (%.4f, %.4f)\n",
                       m_ValidContentBounds.startX, m_ValidContentBounds.startY,
                       m_ValidContentBounds.endX, m_ValidContentBounds.endY);
                printf("Valid bounds (canvas logical): (%.2f, %.2f) to (%.2f, %.2f)\n",
                       validLeft, validTop, validRight, validBottom);
                printf("Valid bounds (screen): (%.2f, %.2f) to (%.2f, %.2f)\n",
                       transformMin.x, transformMin.y, transformMax.x, transformMax.y);
                printf("============================================\n\n");
            }
            
            RenderTransformControls(transformMin, transformMax);
            
            // 更新并渲染对齐辅助线（仅在拖拽图片时）
            if (m_IsDraggingImage) {
                // 计算图片中心在画布逻辑坐标系中的位置（使用 PS 矩形模型）
                ImVec2 imageCenter;
                imageCenter.x = static_cast<float>(m_TransformRect.GetCenterX());
                imageCenter.y = static_cast<float>(m_TransformRect.GetCenterY());
                
                ImVec2 canvasLogicalSize(static_cast<float>(config.canvas.width), static_cast<float>(config.canvas.height));
                
                // 更新对齐状态并获取吸附偏移量
                ImVec2 snapOffset = m_GuideLineManager.Update(imageCenter, canvasLogicalSize, 8.0f);
                
                // 应用吸附偏移（如果有）
                if (snapOffset.x != 0.0f || snapOffset.y != 0.0f) {
                    m_TransformRect.left += snapOffset.x;
                    m_TransformRect.right += snapOffset.x;
                    m_TransformRect.top += snapOffset.y;
                    m_TransformRect.bottom += snapOffset.y;
                }
                
                // 渲染对齐辅助线
                m_GuideLineManager.Render(drawList, canvasMin, canvasMax, canvasLogicalSize);
            }
        }
        
        // ✅ 选区模式：处理输入和渲染选区
        if (m_SelectionMode) {
            // 处理选区输入（鼠标交互）
            if (ImGui::IsWindowHovered()) {
                ImVec2 mousePos = ImGui::GetMousePos();
                bool isMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
                bool isMouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
                ImGuiIO& io = ImGui::GetIO();
                bool shiftPressed = io.KeyShift;
                bool altPressed = io.KeyAlt;
                
                // 转换为画布逻辑坐标
                ImVec2 logicalMousePos = ScreenToCanvas(mousePos, config);
                
                // 调试：验证坐标转换
                static int coordDebugCount = 0;
                if (coordDebugCount++ % 60 == 0) {
                    printf("\n=== ScreenToCanvas DEBUG ===\n");
                    printf("Input screen mousePos: (%.2f, %.2f)\n", mousePos.x, mousePos.y);
                    printf("Output logical mousePos: (%.2f, %.2f)\n", logicalMousePos.x, logicalMousePos.y);
                    printf("Canvas bounds in logical: (0, 0) to (%d, %d)\n", config.canvas.width, config.canvas.height);
                    printf("============================\n");
                }
                
                // 更新选区系统
                m_SelectionSystem.Update(logicalMousePos, isMouseDown, isMouseClicked, 
                                          shiftPressed, altPressed);
                
                // ✅ PS 风格：鼠标松开时自动收缩选区到图层范围（结算阶段）
                // 这是核心功能：拖拽时自由，松手后理智
                static bool wasMouseDown = false;
                if (wasMouseDown && !isMouseDown) {
                    // 鼠标刚松开，执行选区收缩
                    // ✅ 关键：选区应该收缩到画布边界（图层边界），而不是图片的显示尺寸
                    // 在 Photoshop 中，图层边界 = 画布边界 = (0, 0, canvas.width, canvas.height)
                    SelectionRect layerBounds;
                    layerBounds.x = 0.0f;  // 画布左上角
                    layerBounds.y = 0.0f;  // 画布左上角
                    layerBounds.width = static_cast<float>(config.canvas.width);   // 画布宽度
                    layerBounds.height = static_cast<float>(config.canvas.height); // 画布高度
                    layerBounds.active = true;
                    
                    // 调试输出
                    printf("\n=== ClampSelectionToLayer DEBUG ===\n");
                    printf("Canvas/Layer bounds (logical coords):\n");
                    printf("  x=%.2f, y=%.2f, width=%.2f, height=%.2f\n",
                           layerBounds.x, layerBounds.y, layerBounds.width, layerBounds.height);
                    printf("  LEFT=%.2f RIGHT=%.2f TOP=%.2f BOTTOM=%.2f\n",
                           layerBounds.x, layerBounds.x + layerBounds.width,
                           layerBounds.y, layerBounds.y + layerBounds.height);
                    printf("\nImage display bounds (for comparison):\n");
                    printf("  x=%.2f, y=%.2f, width=%.2f, height=%.2f\n",
                           imageX, imageY, targetWidth, targetHeight);
                    printf("===================================\n\n");
                    
                    m_SelectionSystem.ClampSelectionToLayer(layerBounds);
                }
                wasMouseDown = isMouseDown;
                
                // 同步选区状态
                m_HasSelection = m_SelectionSystem.HasActiveSelection();
                
                // ✅ 检测选区越界并显示粉色警告线（符合 Photoshop 规范）
                if (m_SelectionSystem.HasActiveSelection()) {
                    SelectionRect selection = m_SelectionSystem.GetSelection();
                    SelectionRect norm = selection.GetNormalized();
                    
                    // 只有当选区有一定大小时才检测越界
                    if (norm.width > 3.0f || norm.height > 3.0f) {
                        // 选区位置和尺寸（画布逻辑坐标）
                        ImVec2 selectionPos(norm.x, norm.y);
                        ImVec2 selectionSize(norm.width, norm.height);
                        
                        // 调试输出：详细对比逻辑坐标vs屏幕显示
                        static int debugCounter = 0;
                        if (debugCounter++ % 60 == 0) {
                            printf("\n=== DEBUG: Selection vs Screen Coordinate ===\n");
                            printf("Selection (LOGICAL canvas coords - 画布逻辑坐标):\n");
                            printf("  norm.x=%.2f, norm.y=%.2f, width=%.2f, height=%.2f\n", 
                                   norm.x, norm.y, norm.width, norm.height);
                            printf("  LEFT=%.2f RIGHT=%.2f TOP=%.2f BOTTOM=%.2f\n",
                                   norm.x, norm.x + norm.width, norm.y, norm.y + norm.height);
                            
                            printf("\nSelection (SCREEN display coords - 屏幕显示坐标):\n");
                            ImVec2 screenMin = CanvasToScreen(ImVec2(norm.x, norm.y), config);
                            ImVec2 screenMax = CanvasToScreen(ImVec2(norm.x + norm.width, norm.y + norm.height), config);
                            printf("  screenMin: (%.2f, %.2f), screenMax: (%.2f, %.2f)\n",
                                   screenMin.x, screenMin.y, screenMax.x, screenMax.y);
                            printf("  screen width=%.2f, height=%.2f\n",
                                   screenMax.x - screenMin.x, screenMax.y - screenMin.y);
                            
                            printf("\nCanvas (LOGICAL coords):\n");
                            printf("  width=%d, height=%d\n", config.canvas.width, config.canvas.height);
                            
                            printf("\nCanvas (SCREEN display - canvas boundaries):\n");
                            printf("  canvasMin: (%.2f, %.2f), canvasMax: (%.2f, %.2f)\n",
                                   canvasMin.x, canvasMin.y, canvasMax.x, canvasMax.y);
                            printf("  screen width=%.2f, height=%.2f\n",
                                   canvasMax.x - canvasMin.x, canvasMax.y - canvasMin.y);
                            
                            printf("\nTransform state:\n");
                            printf("  scale: %.4f, m_CanvasZoom: %.4f\n", scale, m_CanvasZoom);
                            printf("  Image pos(logical): (%.2f, %.2f), size: (%.2f, %.2f)\n",
                                   imageX, imageY, targetWidth, targetHeight);
                            printf("===========================================\n\n");
                        }
                        
                        // ✅ 直接使用渲染时计算好的图层像素边界
                        m_OutOfBoundsRenderer.Update(selectionPos, selectionSize, 
                                                      layerPixelBounds_Pos, layerPixelBounds_Size);
                    } else {
                        // 选区太小，清除越界状态
                        m_OutOfBoundsRenderer.Clear();
                    }
                } else {
                    // 没有选区，清除越界状态
                    m_OutOfBoundsRenderer.Clear();
                }
            }
            
            // ✅ 渲染选区（蚂蚁线和遮罩）
            if (m_SelectionSystem.HasActiveSelection()) {
                float deltaTime = ImGui::GetIO().DeltaTime;
                
                // ✅ 创建图层边界（画布逻辑坐标）用于选区自动收缩
                // 根据 RENAME_FIX.md 规范，图层边界始终是画布范围
                SelectionRect layerBounds;
                layerBounds.x = 0.0f;
                layerBounds.y = 0.0f;
                layerBounds.width = static_cast<float>(config.canvas.width);
                layerBounds.height = static_cast<float>(config.canvas.height);
                layerBounds.active = true;
                
                // 渲染选区（使用正确的 scale，而不是 m_CanvasZoom）
                // scale = 画布逻辑坐标到屏幕坐标的缩放比例
                m_SelectionSystem.Render(drawList, canvasMin, canvasMax, deltaTime, scale, &layerBounds);
                
                // ✅ 渲染越界警告线（必须在蚂蚁线之后，确保覆盖在上面）
                // 注意：这里传递的是 scale，而不是 m_CanvasZoom
                m_OutOfBoundsRenderer.Render(drawList, canvasMin, canvasMax, scale);
            }
            
            // TODO: 检测是否需要擦除选区内容
            // if (shouldEraseSelection && m_HasSelection) {
            //     EraseSelection(config);
            // }
        }
    }

    // 底部尺寸标签（显示缩放比例）
    char sizeText[64];
    snprintf(sizeText, sizeof(sizeText), "%d×%d (%.0f%%)", config.canvas.width, config.canvas.height, m_CanvasZoom * 100.0f);
    float textWidth = ImGui::CalcTextSize(sizeText).x;
    float labelWidth = textWidth + 24;  // 文字宽度 + 左右边距
    
    ImVec2 labelPos(canvasX + canvasDisplayWidth * 0.5f - labelWidth * 0.5f, canvasY + canvasDisplayHeight + 8);
    ImGui::SetCursorScreenPos(labelPos);
    
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::BeginChild("##SizeLabel", ImVec2(labelWidth, 24), true, ImGuiWindowFlags_NoScrollbar);
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetCursorPosX((labelWidth - textWidth) * 0.5f);
    ImGui::SetCursorPosY(4);
    ImGui::TextUnformatted(sizeText);
    ImGui::PopStyleColor();
    
    ImGui::EndChild();
    ImGui::PopStyleColor();
    } catch (const std::exception& e) {
        std::cerr << "Exception in RenderCanvasStage: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in RenderCanvasStage" << std::endl;
    }
}

void PreviewPanel::RenderCropOverlay(Rect& cropRegion, const ImVec2& canvasPos, const ImVec2& canvasSize) {
    if (!cropRegion.IsValid()) {
        // 初始化裁剪框（居中，60% 大小）
        cropRegion.width = static_cast<int>(canvasSize.x * 0.6f);
        cropRegion.height = static_cast<int>(canvasSize.y * 0.6f);
        cropRegion.x = static_cast<int>((canvasSize.x - cropRegion.width) * 0.5f);
        cropRegion.y = static_cast<int>((canvasSize.y - cropRegion.height) * 0.5f);
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // 计算裁剪框屏幕坐标
    ImVec2 cropMin(canvasPos.x + cropRegion.x, canvasPos.y + cropRegion.y);
    ImVec2 cropMax(cropMin.x + cropRegion.width, cropMin.y + cropRegion.height);

    // 绘制半透明遮罩（裁剪区域外）
    ImVec2 canvasMax(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y);
    
    drawList->AddRectFilled(canvasPos, ImVec2(canvasMax.x, cropMin.y), IM_COL32(0, 0, 0, 120));
    drawList->AddRectFilled(ImVec2(canvasPos.x, cropMax.y), canvasMax, IM_COL32(0, 0, 0, 120));
    drawList->AddRectFilled(ImVec2(canvasPos.x, cropMin.y), ImVec2(cropMin.x, cropMax.y), IM_COL32(0, 0, 0, 120));
    drawList->AddRectFilled(ImVec2(cropMax.x, cropMin.y), ImVec2(canvasMax.x, cropMax.y), IM_COL32(0, 0, 0, 120));

    // 绘制裁剪框边框（黄色）
    drawList->AddRect(cropMin, cropMax, IM_COL32(255, 200, 0, 255), 0.0f, 0, 2.0f);

    // 绘制角点
    float cornerSize = 8.0f;
    ImU32 cornerColor = IM_COL32(255, 200, 0, 255);
    
    drawList->AddRectFilled(
        ImVec2(cropMin.x - cornerSize * 0.5f, cropMin.y - cornerSize * 0.5f),
        ImVec2(cropMin.x + cornerSize * 0.5f, cropMin.y + cornerSize * 0.5f),
        cornerColor
    );
    drawList->AddRectFilled(
        ImVec2(cropMax.x - cornerSize * 0.5f, cropMin.y - cornerSize * 0.5f),
        ImVec2(cropMax.x + cornerSize * 0.5f, cropMin.y + cornerSize * 0.5f),
        cornerColor
    );
    drawList->AddRectFilled(
        ImVec2(cropMin.x - cornerSize * 0.5f, cropMax.y - cornerSize * 0.5f),
        ImVec2(cropMin.x + cornerSize * 0.5f, cropMax.y + cornerSize * 0.5f),
        cornerColor
    );
    drawList->AddRectFilled(
        ImVec2(cropMax.x - cornerSize * 0.5f, cropMax.y - cornerSize * 0.5f),
        ImVec2(cropMax.x + cornerSize * 0.5f, cropMax.y + cornerSize * 0.5f),
        cornerColor
    );

    // TODO: 实现拖拽交互
}

// 屏幕坐标转画布逻辑坐标
ImVec2 PreviewPanel::ScreenToCanvas(const ImVec2& screenPos, const ProcessConfig& config) const {
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    // ✅ 必须与 RenderCanvasStage 中的计算逻辑完全一致
    float canvasAspect = static_cast<float>(config.canvas.width) / config.canvas.height;
    float padding = 0.0f;
    
    float maxWidth = windowSize.x - padding * 2;
    float maxHeight = windowSize.y - padding * 2 - 40; // 留出底部标签空间
    
    // 应用缩放
    float canvasDisplayWidth = maxWidth * m_CanvasZoom;
    float canvasDisplayHeight = canvasDisplayWidth / canvasAspect;

    // ✅ 关键：如果高度超出，需要重新计算宽度
    if (canvasDisplayHeight > maxHeight * m_CanvasZoom) {
        canvasDisplayHeight = maxHeight * m_CanvasZoom;
        canvasDisplayWidth = canvasDisplayHeight * canvasAspect;
    }
    
    // 画布位置（居中 + 偏移）
    float canvasX = windowPos.x + (windowSize.x - canvasDisplayWidth) * 0.5f + m_CanvasOffset.x;
    float canvasY = windowPos.y + (windowSize.y - canvasDisplayHeight - 40) * 0.5f + m_CanvasOffset.y;
    
    // ✅ 计算画布逻辑坐标到屏幕坐标的缩放比例
    float scale = canvasDisplayWidth / config.canvas.width;
    
    // 转换为画布逻辑坐标
    float logicalX = (screenPos.x - canvasX) / scale;
    float logicalY = (screenPos.y - canvasY) / scale;
    
    return ImVec2(logicalX, logicalY);
}

// 画布逻辑坐标转屏幕坐标
ImVec2 PreviewPanel::CanvasToScreen(const ImVec2& canvasPos, const ProcessConfig& config) const {
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    // ✅ 必须与 RenderCanvasStage 中的计算逻辑完全一致
    float canvasAspect = static_cast<float>(config.canvas.width) / config.canvas.height;
    float padding = 0.0f;
    
    float maxWidth = windowSize.x - padding * 2;
    float maxHeight = windowSize.y - padding * 2 - 40; // 留出底部标签空间
    
    // 应用缩放
    float canvasDisplayWidth = maxWidth * m_CanvasZoom;
    float canvasDisplayHeight = canvasDisplayWidth / canvasAspect;

    // ✅ 关键：如果高度超出，需要重新计算宽度
    if (canvasDisplayHeight > maxHeight * m_CanvasZoom) {
        canvasDisplayHeight = maxHeight * m_CanvasZoom;
        canvasDisplayWidth = canvasDisplayHeight * canvasAspect;
    }
    
    // 画布位置（居中 + 偏移）
    float canvasX = windowPos.x + (windowSize.x - canvasDisplayWidth) * 0.5f + m_CanvasOffset.x;
    float canvasY = windowPos.y + (windowSize.y - canvasDisplayHeight - 40) * 0.5f + m_CanvasOffset.y;
    
    // ✅ 计算画布逻辑坐标到屏幕坐标的缩放比例
    float scale = canvasDisplayWidth / config.canvas.width;
    
    // 转换为屏幕坐标
    float screenX = canvasX + canvasPos.x * scale;
    float screenY = canvasY + canvasPos.y * scale;
    
    return ImVec2(screenX, screenY);
}

void PreviewPanel::RenderEmptyState(const char* message, const ImVec4& color) {
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    // 深色棋盘格背景
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    
    const float gridSize = 16.0f;
    const ImU32 color1 = IM_COL32(30, 30, 30, 255);
    const ImU32 color2 = IM_COL32(40, 40, 40, 255);
    
    for (float y = 0; y < windowSize.y; y += gridSize) {
        for (float x = 0; x < windowSize.x; x += gridSize) {
            int gridX = static_cast<int>(x / gridSize);
            int gridY = static_cast<int>(y / gridSize);
            ImU32 gridColor = ((gridX + gridY) % 2 == 0) ? color1 : color2;
            
            drawList->AddRectFilled(
                ImVec2(windowPos.x + x, windowPos.y + y),
                ImVec2(windowPos.x + x + gridSize, windowPos.y + gridSize),
                gridColor
            );
        }
    }
    
    // 居中文本
    ImGui::SetCursorPosY(windowSize.y * 0.5f - 20);
    
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    float textWidth = ImGui::CalcTextSize(message).x;
    ImGui::SetCursorPosX((windowSize.x - textWidth) * 0.5f);
    ImGui::TextUnformatted(message);
    ImGui::PopStyleColor();
}

bool PreviewPanel::LoadCurrentImage(const std::string& filePath) {
    try {
        if (filePath.empty()) {
            std::cerr << "Error: Empty file path" << std::endl;
            return false;
        }

        // ✅ 1. 先保存当前图片到缓存（如果有修改或有历史记录）
        if (!m_CurrentImagePath.empty() && m_CurrentImage.IsValid()) {
            ImageCache cache;
            cache.imageData = m_CurrentImage;
            cache.modified = m_ImageModified;
            cache.history = m_ImageHistory;  // ✅ 保存历史记录
            cache.validBounds = m_ValidContentBounds;  // ✅ 保存有效内容边界
            m_ImageCache[m_CurrentImagePath] = cache;
            printf("[LoadImage] Saved image to cache: %s (modified=%d, history_count=%zu)\n", 
                   m_CurrentImagePath.c_str(), m_ImageModified, m_ImageHistory.GetHistoryCount());
        }

        // ✅ 2. 检查缓存中是否有这张图片
        auto it = m_ImageCache.find(filePath);
        if (it != m_ImageCache.end()) {
            // 从缓存加载
            printf("[LoadImage] Loading from cache: %s (modified=%d, history_count=%zu)\n", 
                   filePath.c_str(), it->second.modified, it->second.history.GetHistoryCount());
            
            m_CurrentImage = it->second.imageData;
            m_CurrentImagePath = filePath;
            m_ImageModified = it->second.modified;
            m_ImageHistory = it->second.history;  // ✅ 恢复历史记录
            m_ValidContentBounds = it->second.validBounds;  // ✅ 恢复有效内容边界
            
            // 更新纹理
            if (!CreateTexture(m_CurrentImage)) {
                std::cerr << "Failed to create texture from cache" << std::endl;
                return false;
            }
            
            return true;
        }

        // ✅ 3. 缓存中没有，从磁盘加载
        printf("[LoadImage] Loading from disk: %s\n", filePath.c_str());
        
        ImageData tempImage;
        if (!ImageLoader::Load(filePath, tempImage)) {
            std::cerr << "Failed to load image: " << filePath << std::endl;
            return false;
        }

        // 先创建纹理，成功后再更新当前图片
        if (!CreateTexture(tempImage)) {
            std::cerr << "Failed to create texture for: " << filePath << std::endl;
            return false;
        }

        // 成功后才更新
        m_CurrentImage = tempImage;
        m_CurrentImagePath = filePath;
        m_ImageModified = false;  // 新加载的图片未修改
        
        // 清空历史记录（新加载的图片）
        m_ImageHistory.Clear();
        m_ValidContentBounds.Reset();  // ✅ 清空有效内容边界
        printf("[LoadImage] Loaded new image from disk, history cleared.\n");
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception in LoadCurrentImage: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception in LoadCurrentImage" << std::endl;
        return false;
    }
}

bool PreviewPanel::CreateTexture(const ImageData& imageData) {
    try {
        // 释放旧纹理
        ReleaseTexture();
        
        if (!imageData.IsValid()) {
            std::cerr << "Invalid image data for texture" << std::endl;
            return false;
        }
        
        // 创建 OpenGL 纹理
        glGenTextures(1, &m_TextureID);
        if (m_TextureID == 0) {
            std::cerr << "Failed to generate OpenGL texture" << std::endl;
            return false;
        }

        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        
        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // 上传纹理数据
        GLenum format = GL_RGB;
        if (imageData.channels == 4) {
            format = GL_RGBA;
        } else if (imageData.channels == 1) {
            format = GL_RED;
        } else if (imageData.channels != 3) {
            std::cerr << "Unsupported channel count: " << imageData.channels << std::endl;
            glDeleteTextures(1, &m_TextureID);
            m_TextureID = 0;
            return false;
        }
        
        if (imageData.pixels.empty()) {
            std::cerr << "Image pixel data is empty" << std::endl;
            glDeleteTextures(1, &m_TextureID);
            m_TextureID = 0;
            return false;
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, format, imageData.width, imageData.height,
                     0, format, GL_UNSIGNED_BYTE, imageData.pixels.data());
        
        GLenum glError = glGetError();
        if (glError != GL_NO_ERROR) {
            std::cerr << "OpenGL error during texture upload: " << glError << std::endl;
            glDeleteTextures(1, &m_TextureID);
            m_TextureID = 0;
            return false;
        }
        
        m_TextureWidth = imageData.width;
        m_TextureHeight = imageData.height;
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception in CreateTexture: " << e.what() << std::endl;
        ReleaseTexture();
        return false;
    } catch (...) {
        std::cerr << "Unknown exception in CreateTexture" << std::endl;
        ReleaseTexture();
        return false;
    }
}

void PreviewPanel::ReleaseTexture() {
    if (m_TextureID != 0) {
        glDeleteTextures(1, &m_TextureID);
        m_TextureID = 0;
        m_TextureWidth = 0;
        m_TextureHeight = 0;
    }
}

void PreviewPanel::ResetTransform() {
    // 重置 PS 矩形模型
    m_TransformRect = TransformRect();
    m_DragStartRect = TransformRect();
    m_DragStartAspect = 1.0;
    
    // 兼容旧代码
    m_TransformScale = ImVec2(1.0f, 1.0f);
    m_TransformRotation = 0.0f;
    m_TransformPosition = ImVec2(0.0f, 0.0f);
    
    m_DraggingHandle = TransformHandle::None;
    // m_HasCustomBounds = false;  // TODO: 旧擦除工具变量，已删除
    m_IsDraggingImage = false;
    
    // 重置画布视图
    m_CanvasZoom = 1.0f;
    m_CanvasOffset = ImVec2(0.0f, 0.0f);
    
    // 重置 TransformManager
    m_TransformManager.Reset();
    m_GuideLineManager.Clear();
}

void PreviewPanel::SaveTransformState(ImageInfo& imageInfo) {
    // ✅ 保存 PS 矩形模型的变换状态
    imageInfo.transformState.scaleX = static_cast<float>(m_TransformRect.left);
    imageInfo.transformState.scaleY = static_cast<float>(m_TransformRect.top);
    imageInfo.transformState.positionX = static_cast<float>(m_TransformRect.right);
    imageInfo.transformState.positionY = static_cast<float>(m_TransformRect.bottom);
    imageInfo.transformState.rotation = 0.0f;  // 暂未使用
    
    // 检查是否有自定义变换（不是默认值）
    imageInfo.transformState.hasTransform = 
        (m_TransformRect.GetWidth() > 0 && m_TransformRect.GetHeight() > 0);
}

void PreviewPanel::SaveCurrentTransformState(ImageInfo& imageInfo) {
    // 公共接口，直接调用私有方法
    SaveTransformState(imageInfo);
}

bool PreviewPanel::GetCachedImageData(const std::string& filePath, ImageData& outImageData) const {
    // 查找缓存
    auto it = m_ImageCache.find(filePath);
    if (it != m_ImageCache.end() && it->second.modified) {
        // 找到缓存且已修改
        outImageData = it->second.imageData;
        return true;
    }
    
    // 检查当前图片（可能还没保存到缓存）
    if (m_CurrentImagePath == filePath && m_ImageModified && m_CurrentImage.IsValid()) {
        outImageData = m_CurrentImage;
        return true;
    }
    
    return false;
}

void PreviewPanel::RestoreTransformState(const ImageInfo& imageInfo) {
    if (imageInfo.transformState.hasTransform) {
        // ✅ 恢复 PS 矩形模型的变换状态
        m_TransformRect.left = static_cast<double>(imageInfo.transformState.scaleX);
        m_TransformRect.top = static_cast<double>(imageInfo.transformState.scaleY);
        m_TransformRect.right = static_cast<double>(imageInfo.transformState.positionX);
        m_TransformRect.bottom = static_cast<double>(imageInfo.transformState.positionY);
    } else {
        // 没有保存的变换状态，重置矩形（会在渲染时自动初始化）
        m_TransformRect = TransformRect();
    }
}

void PreviewPanel::RenderTransformControls(const ImVec2& imageMin, const ImVec2& imageMax) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // 计算8个控制点的位置
    ImVec2 center = ImVec2((imageMin.x + imageMax.x) * 0.5f, (imageMin.y + imageMax.y) * 0.5f);
    ImVec2 size = ImVec2(imageMax.x - imageMin.x, imageMax.y - imageMin.y);
    
    ImVec2 handles[9];
    handles[0] = imageMin;  // TopLeft
    handles[1] = ImVec2(center.x, imageMin.y);  // TopCenter
    handles[2] = ImVec2(imageMax.x, imageMin.y);  // TopRight
    handles[3] = ImVec2(imageMax.x, center.y);  // MiddleRight
    handles[4] = imageMax;  // BottomRight
    handles[5] = ImVec2(center.x, imageMax.y);  // BottomCenter
    handles[6] = ImVec2(imageMin.x, imageMax.y);  // BottomLeft
    handles[7] = ImVec2(imageMin.x, center.y);  // MiddleLeft
    handles[8] = center;  // Center
    
    // 绘制变换边框
    drawList->AddRect(imageMin, imageMax, IM_COL32(0, 150, 255, 255), 0.0f, 0, 2.0f);
    
    // 绘制对角线
    drawList->AddLine(imageMin, imageMax, IM_COL32(0, 150, 255, 128), 1.0f);
    drawList->AddLine(ImVec2(imageMax.x, imageMin.y), ImVec2(imageMin.x, imageMax.y), IM_COL32(0, 150, 255, 128), 1.0f);
    
    // 绘制控制点
    const float handleSize = 8.0f;
    const ImU32 handleColor = IM_COL32(255, 255, 255, 255);
    const ImU32 handleBorderColor = IM_COL32(0, 150, 255, 255);
    
    for (int i = 0; i < 9; i++) {
        // 中心点用圆形，其他用方形
        if (i == 8) {
            drawList->AddCircleFilled(handles[i], handleSize * 0.8f, handleColor);
            drawList->AddCircle(handles[i], handleSize * 0.8f, handleBorderColor, 0, 2.0f);
        } else {
            ImVec2 handleMin(handles[i].x - handleSize * 0.5f, handles[i].y - handleSize * 0.5f);
            ImVec2 handleMax(handles[i].x + handleSize * 0.5f, handles[i].y + handleSize * 0.5f);
            drawList->AddRectFilled(handleMin, handleMax, handleColor);
            drawList->AddRect(handleMin, handleMax, handleBorderColor, 0.0f, 0, 2.0f);
        }
    }
    
    // 处理拖拽
    HandleTransformInput(imageMin, imageMax);
}

void PreviewPanel::HandleTransformInput(const ImVec2& imageMin, const ImVec2& imageMax) {
    // 如果正在拖拽画布（空格键 + 拖拽），跳过变换输入处理
    if (m_IsDraggingCanvas) {
        return;
    }
    
    // 如果按住空格键，也跳过变换输入处理（优先使用抓手工具）
    if (ImGui::IsKeyDown(ImGuiKey_Space)) {
        return;
    }
    
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 center = ImVec2((imageMin.x + imageMax.x) * 0.5f, (imageMin.y + imageMax.y) * 0.5f);
    
    // 计算8个控制点的位置
    ImVec2 handles[9];
    handles[0] = imageMin;  // TopLeft
    handles[1] = ImVec2(center.x, imageMin.y);  // TopCenter
    handles[2] = ImVec2(imageMax.x, imageMin.y);  // TopRight
    handles[3] = ImVec2(imageMax.x, center.y);  // MiddleRight
    handles[4] = imageMax;  // BottomRight
    handles[5] = ImVec2(center.x, imageMax.y);  // BottomCenter
    handles[6] = ImVec2(imageMin.x, imageMax.y);  // BottomLeft
    handles[7] = ImVec2(imageMin.x, center.y);  // MiddleLeft
    handles[8] = center;  // Center
    
    const float handleHitRadius = 12.0f;
    
    // 开始拖拽
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        m_DraggingHandle = TransformHandle::None;
        
        // 首先检测是否点击了控制点
        bool hitHandle = false;
        for (int i = 0; i < 9; i++) {
            float dist = sqrtf(powf(mousePos.x - handles[i].x, 2) + powf(mousePos.y - handles[i].y, 2));
            if (dist < handleHitRadius) {
                m_DraggingHandle = static_cast<TransformHandle>(i);
                m_DragStartPos = mousePos;
                
                // ✅ PS 模型：缓存初始矩形状态（画布逻辑坐标）
                m_DragStartRect = m_TransformRect;
                m_DragStartAspect = m_DragStartRect.GetWidth() / m_DragStartRect.GetHeight();
                
                hitHandle = true;
                break;
            }
        }
        
        // 如果没有点击控制点，检测是否点击了图片区域
        if (!hitHandle) {
            // 检测鼠标是否在图片范围内
            if (mousePos.x >= imageMin.x && mousePos.x <= imageMax.x &&
                mousePos.y >= imageMin.y && mousePos.y <= imageMax.y) {
                // 点击图片区域，进入移动模式
                m_DraggingHandle = TransformHandle::Center;
                m_DragStartPos = mousePos;
                m_DragStartRect = m_TransformRect;
            }
        }
    }
    
    // 拖拽中
    if (m_DraggingHandle != TransformHandle::None && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        ImVec2 delta = ImVec2(mousePos.x - m_DragStartPos.x, mousePos.y - m_DragStartPos.y);
        
        if (m_DraggingHandle == TransformHandle::Center) {
            // 移动图片：将屏幕坐标的 delta 转换为画布逻辑坐标的 delta
            double logicalDeltaX = static_cast<double>(delta.x) / m_CanvasZoom;
            double logicalDeltaY = static_cast<double>(delta.y) / m_CanvasZoom;
            
            // ✅ PS 模型：直接平移矩形的四条边
            m_TransformRect.left = m_DragStartRect.left + logicalDeltaX;
            m_TransformRect.right = m_DragStartRect.right + logicalDeltaX;
            m_TransformRect.top = m_DragStartRect.top + logicalDeltaY;
            m_TransformRect.bottom = m_DragStartRect.bottom + logicalDeltaY;
            
            m_IsDraggingImage = true;
        } else {
            // ✅ PS 核心：几何重建，而不是 scale 矩阵
            // 每一帧从初始状态 + 当前鼠标位移重新计算
            
            // 转换拖拽距离到画布逻辑坐标
            double logicalDeltaX = static_cast<double>(delta.x) / m_CanvasZoom;
            double logicalDeltaY = static_cast<double>(delta.y) / m_CanvasZoom;
            
            // 获取初始状态
            double T = m_DragStartRect.top;
            double B = m_DragStartRect.bottom;
            double L = m_DragStartRect.left;
            double R = m_DragStartRect.right;
            double H = B - T;
            double W = R - L;
            double CenterX = (L + R) * 0.5;
            double CenterY = (T + B) * 0.5;
            double Aspect = m_DragStartAspect;
            
            // 判断是边缘中点还是角点
            bool isEdgeCenter = (m_DraggingHandle == TransformHandle::TopCenter ||
                                m_DraggingHandle == TransformHandle::BottomCenter ||
                                m_DraggingHandle == TransformHandle::MiddleLeft ||
                                m_DraggingHandle == TransformHandle::MiddleRight);
            
            if (isEdgeCenter) {
                // ✅ 边缘中点：PS 工业级实现
                
                if (m_DraggingHandle == TransformHandle::BottomCenter) {
                    // 拖拽下边缘中点：上边缘固定，等比例缩放
                    
                    // 1. 上边缘固定
                    m_TransformRect.top = T;
                    
                    // 2. 新高度由拖拽直接决定
                    double newHeight = H + logicalDeltaY;
                    newHeight = std::max(newHeight, 1.0);
                    
                    // 3. 等比例约束
                    double newWidth = newHeight * Aspect;
                    
                    // 4. 水平方向保持居中
                    m_TransformRect.left = CenterX - newWidth * 0.5;
                    m_TransformRect.right = CenterX + newWidth * 0.5;
                    
                    // 5. 新下边缘
                    m_TransformRect.bottom = m_TransformRect.top + newHeight;
                    
                } else if (m_DraggingHandle == TransformHandle::TopCenter) {
                    // 拖拽上边缘中点：下边缘固定，等比例缩放
                    
                    // 1. 下边缘固定
                    m_TransformRect.bottom = B;
                    
                    // 2. 新高度由拖拽直接决定
                    double newHeight = H - logicalDeltaY;
                    newHeight = std::max(newHeight, 1.0);
                    
                    // 3. 等比例约束
                    double newWidth = newHeight * Aspect;
                    
                    // 4. 水平方向保持居中
                    m_TransformRect.left = CenterX - newWidth * 0.5;
                    m_TransformRect.right = CenterX + newWidth * 0.5;
                    
                    // 5. 新上边缘
                    m_TransformRect.top = m_TransformRect.bottom - newHeight;
                    
                } else if (m_DraggingHandle == TransformHandle::MiddleRight) {
                    // 拖拽右边缘中点：左边缘固定，等比例缩放
                    
                    // 1. 左边缘固定
                    m_TransformRect.left = L;
                    
                    // 2. 新宽度由拖拽直接决定
                    double newWidth = W + logicalDeltaX;
                    newWidth = std::max(newWidth, 1.0);
                    
                    // 3. 等比例约束
                    double newHeight = newWidth / Aspect;
                    
                    // 4. 垂直方向保持居中
                    m_TransformRect.top = CenterY - newHeight * 0.5;
                    m_TransformRect.bottom = CenterY + newHeight * 0.5;
                    
                    // 5. 新右边缘
                    m_TransformRect.right = m_TransformRect.left + newWidth;
                    
                } else if (m_DraggingHandle == TransformHandle::MiddleLeft) {
                    // 拖拽左边缘中点：右边缘固定，等比例缩放
                    
                    // 1. 右边缘固定
                    m_TransformRect.right = R;
                    
                    // 2. 新宽度由拖拽直接决定
                    double newWidth = W - logicalDeltaX;
                    newWidth = std::max(newWidth, 1.0);
                    
                    // 3. 等比例约束
                    double newHeight = newWidth / Aspect;
                    
                    // 4. 垂直方向保持居中
                    m_TransformRect.top = CenterY - newHeight * 0.5;
                    m_TransformRect.bottom = CenterY + newHeight * 0.5;
                    
                    // 5. 新左边缘
                    m_TransformRect.left = m_TransformRect.right - newWidth;
                }
                
            } else {
                // 角点：等比例缩放（对角点固定）
                
                // 确定固定的锚点
                double anchorX, anchorY;
                
                switch (m_DraggingHandle) {
                    case TransformHandle::TopLeft:
                        anchorX = R; anchorY = B;  // 右下角固定
                        break;
                    case TransformHandle::TopRight:
                        anchorX = L; anchorY = B;  // 左下角固定
                        break;
                    case TransformHandle::BottomRight:
                        anchorX = L; anchorY = T;  // 左上角固定
                        break;
                    case TransformHandle::BottomLeft:
                        anchorX = R; anchorY = T;  // 右上角固定
                        break;
                    default:
                        anchorX = CenterX; anchorY = CenterY;
                        break;
                }
                
                // 计算拖拽点到锚点的距离变化
                // 屏幕坐标系中的距离
                ImVec2 anchorScreen;
                anchorScreen.x = (imageMin.x + imageMax.x) * 0.5f + static_cast<float>((anchorX - CenterX) * m_CanvasZoom);
                anchorScreen.y = (imageMin.y + imageMax.y) * 0.5f + static_cast<float>((anchorY - CenterY) * m_CanvasZoom);
                
                double startDist = sqrt(pow(m_DragStartPos.x - anchorScreen.x, 2) + pow(m_DragStartPos.y - anchorScreen.y, 2));
                double currentDist = sqrt(pow(mousePos.x - anchorScreen.x, 2) + pow(mousePos.y - anchorScreen.y, 2));
                
                double scaleFactor = 1.0;
                if (startDist > 1.0) {
                    scaleFactor = currentDist / startDist;
                }
                
                // 等比例缩放
                double newWidth = W * scaleFactor;
                double newHeight = H * scaleFactor;
                newWidth = std::max(newWidth, 1.0);
                newHeight = std::max(newHeight, 1.0);
                
                // 根据锚点重新计算矩形
                switch (m_DraggingHandle) {
                    case TransformHandle::TopLeft:
                        m_TransformRect.right = R;
                        m_TransformRect.bottom = B;
                        m_TransformRect.left = R - newWidth;
                        m_TransformRect.top = B - newHeight;
                        break;
                    case TransformHandle::TopRight:
                        m_TransformRect.left = L;
                        m_TransformRect.bottom = B;
                        m_TransformRect.right = L + newWidth;
                        m_TransformRect.top = B - newHeight;
                        break;
                    case TransformHandle::BottomRight:
                        m_TransformRect.left = L;
                        m_TransformRect.top = T;
                        m_TransformRect.right = L + newWidth;
                        m_TransformRect.bottom = T + newHeight;
                        break;
                    case TransformHandle::BottomLeft:
                        m_TransformRect.right = R;
                        m_TransformRect.top = T;
                        m_TransformRect.left = R - newWidth;
                        m_TransformRect.bottom = T + newHeight;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    
    // 结束拖拽
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        m_DraggingHandle = TransformHandle::None;
        m_IsDraggingImage = false;
        m_GuideLineManager.Clear();
    }
    
    // 更新鼠标光标
    if (m_DraggingHandle == TransformHandle::None) {
        for (int i = 0; i < 9; i++) {
            float dist = sqrtf(powf(mousePos.x - handles[i].x, 2) + powf(mousePos.y - handles[i].y, 2));
            if (dist < handleHitRadius) {
                if (i == 8) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                } else {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                }
                break;
            }
        }
    }
}

bool PreviewPanel::DeleteSelectionContent(const ProcessConfig& config) {
    // ✅ PS 风格：Delete / Backspace 删除选区内容
    
    // 1. 检查是否有有效选区
    if (!m_SelectionSystem.HasActiveSelection()) {
        printf("[DeleteSelection] No active selection, operation ignored.\n");
        return false;
    }
    
    // 2. 检查是否有有效的图像数据
    if (!m_CurrentImage.IsValid() || m_CurrentImage.pixels.empty()) {
        printf("[DeleteSelection] No valid image data.\n");
        return false;
    }
    
    // ✅ 3. 在执行删除前保存当前图像状态到历史记录
    m_ImageHistory.Push(m_CurrentImage, "Delete Selection");
    
    // 4. 获取选区（画布逻辑坐标）
    SelectionRect selection = m_SelectionSystem.GetSelection();
    SelectionRect norm = selection.GetNormalized();
    
    printf("\n=== Delete Selection Content ===\n");
    printf("Selection bounds (canvas logical coords):\n");
    printf("  x=%.2f, y=%.2f, width=%.2f, height=%.2f\n", 
           norm.x, norm.y, norm.width, norm.height);
    
    // ✅ 关键修复：将画布坐标转换为图片像素坐标
    // 获取图片在画布中的变换信息
    float imageX = static_cast<float>(m_TransformRect.left);
    float imageY = static_cast<float>(m_TransformRect.top);
    float imageWidth = static_cast<float>(m_TransformRect.GetWidth());
    float imageHeight = static_cast<float>(m_TransformRect.GetHeight());
    
    printf("Image transform (canvas logical coords):\n");
    printf("  pos: (%.2f, %.2f), size: (%.2f, %.2f)\n", 
           imageX, imageY, imageWidth, imageHeight);
    printf("Image original size: %d x %d\n", m_CurrentImage.width, m_CurrentImage.height);
    
    // 计算选区相对于图片的坐标（画布坐标 -> 图片坐标）
    // 选区在画布中的位置 - 图片在画布中的位置 = 选区相对于图片的位置
    float selectionInImageX = norm.x - imageX;
    float selectionInImageY = norm.y - imageY;
    
    // 将画布坐标系的尺寸转换为图片像素坐标系
    // 缩放比例 = 图片原始尺寸 / 图片在画布中的显示尺寸
    float scaleX = m_CurrentImage.width / imageWidth;
    float scaleY = m_CurrentImage.height / imageHeight;
    
    printf("Scale factors: scaleX=%.4f, scaleY=%.4f\n", scaleX, scaleY);
    
    // 转换为图片像素坐标
    float pixelX = selectionInImageX * scaleX;
    float pixelY = selectionInImageY * scaleY;
    float pixelWidth = norm.width * scaleX;
    float pixelHeight = norm.height * scaleY;
    
    printf("Selection in image pixel coords:\n");
    printf("  x=%.2f, y=%.2f, width=%.2f, height=%.2f\n", 
           pixelX, pixelY, pixelWidth, pixelHeight);
    
    // 4. 计算删除区域（选区 ∩ 图像边界）
    int deleteLeft = std::max(0, static_cast<int>(std::floor(pixelX)));
    int deleteTop = std::max(0, static_cast<int>(std::floor(pixelY)));
    int deleteRight = std::min(m_CurrentImage.width, static_cast<int>(std::ceil(pixelX + pixelWidth)));
    int deleteBottom = std::min(m_CurrentImage.height, static_cast<int>(std::ceil(pixelY + pixelHeight)));
    
    printf("Image size: %d x %d\n", m_CurrentImage.width, m_CurrentImage.height);
    printf("Canvas size: %d x %d\n", config.canvas.width, config.canvas.height);
    
    // 检查是否有有效的删除区域
    if (deleteRight <= deleteLeft || deleteBottom <= deleteTop) {
        printf("[DeleteSelection] Selection is outside image bounds.\n");
        return false;
    }
    
    printf("Delete area (pixel coords):\n");
    printf("  left=%d, top=%d, right=%d, bottom=%d\n", 
           deleteLeft, deleteTop, deleteRight, deleteBottom);
    printf("  width=%d, height=%d\n", 
           deleteRight - deleteLeft, deleteBottom - deleteTop);
    
    // 5. 确保图像有 Alpha 通道
    int channels = m_CurrentImage.channels;
    if (channels < 4) {
        printf("[DeleteSelection] Converting image to RGBA format...\n");
        
        // 转换为 RGBA 格式
        std::vector<uint8_t> newPixels;
        newPixels.reserve(m_CurrentImage.width * m_CurrentImage.height * 4);
        
        for (int y = 0; y < m_CurrentImage.height; y++) {
            for (int x = 0; x < m_CurrentImage.width; x++) {
                int srcIdx = (y * m_CurrentImage.width + x) * channels;
                
                if (channels == 3) {
                    // RGB -> RGBA
                    newPixels.push_back(m_CurrentImage.pixels[srcIdx + 0]); // R
                    newPixels.push_back(m_CurrentImage.pixels[srcIdx + 1]); // G
                    newPixels.push_back(m_CurrentImage.pixels[srcIdx + 2]); // B
                    newPixels.push_back(255); // A (不透明)
                } else if (channels == 1) {
                    // Grayscale -> RGBA
                    uint8_t gray = m_CurrentImage.pixels[srcIdx];
                    newPixels.push_back(gray); // R
                    newPixels.push_back(gray); // G
                    newPixels.push_back(gray); // B
                    newPixels.push_back(255);  // A (不透明)
                }
            }
        }
        
        m_CurrentImage.pixels = std::move(newPixels);
        m_CurrentImage.channels = 4;
        channels = 4;
    }
    
    // 6. 删除选区内的像素（设置 Alpha = 0）
    int pixelsDeleted = 0;
    for (int y = deleteTop; y < deleteBottom; y++) {
        for (int x = deleteLeft; x < deleteRight; x++) {
            // 计算像素索引
            int pixelIdx = (y * m_CurrentImage.width + x) * channels;
            
            // ✅ PS 行为：将 Alpha 设置为 0（完全透明）
            // RGB 值保留（虽然不可见，但符合 PS 行为）
            m_CurrentImage.pixels[pixelIdx + 3] = 0;  // Alpha = 0
            pixelsDeleted++;
        }
    }
    
    printf("Deleted %d pixels (set Alpha=0)\n", pixelsDeleted);
    
    // 7. 更新纹理
    if (!CreateTexture(m_CurrentImage)) {
        printf("[DeleteSelection] Failed to update texture.\n");
        return false;
    }
    
    // ✅ 8. 标记图像为已修改并同步缓存
    m_ImageModified = true;
    
    // ✅ 同步更新缓存（确保缓存与当前状态一致）
    if (!m_CurrentImagePath.empty()) {
        ImageCache cache;
        cache.imageData = m_CurrentImage;
        cache.modified = true;
        cache.history = m_ImageHistory;  // ✅ 保存历史记录
        cache.validBounds = m_ValidContentBounds;  // ✅ 保存有效内容边界
        m_ImageCache[m_CurrentImagePath] = cache;
        printf("Cache updated for: %s\n", m_CurrentImagePath.c_str());
    }
    
    printf("Texture updated successfully.\n");
    printf("Image marked as modified.\n");
    
    // ✅ 9. 计算剩余有效像素的边界（非透明区域）
    int minX = m_CurrentImage.width;
    int minY = m_CurrentImage.height;
    int maxX = -1;
    int maxY = -1;
    
    for (int y = 0; y < m_CurrentImage.height; y++) {
        for (int x = 0; x < m_CurrentImage.width; x++) {
            int pixelIdx = (y * m_CurrentImage.width + x) * channels;
            uint8_t alpha = m_CurrentImage.pixels[pixelIdx + 3];
            
            if (alpha > 0) {  // 有效像素（非完全透明）
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
            }
        }
    }
    
    // ✅ 10. 保存有效内容边界（归一化坐标）
    // 不改变 m_TransformRect，而是记录有效内容的相对位置
    if (maxX >= minX && maxY >= minY) {
        printf("\n=== Save Valid Content Bounds ===\n");
        printf("Valid pixel bounds: (%d, %d) to (%d, %d)\n", minX, minY, maxX, maxY);
        printf("Valid size: %d x %d\n", maxX - minX + 1, maxY - minY + 1);
        
        // 计算有效内容在图片中的相对位置（归一化坐标 0-1）
        m_ValidContentBounds.startX = static_cast<float>(minX) / m_CurrentImage.width;
        m_ValidContentBounds.startY = static_cast<float>(minY) / m_CurrentImage.height;
        m_ValidContentBounds.endX = static_cast<float>(maxX + 1) / m_CurrentImage.width;
        m_ValidContentBounds.endY = static_cast<float>(maxY + 1) / m_CurrentImage.height;
        m_ValidContentBounds.isValid = true;
        
        printf("Valid content normalized coords: (%.4f, %.4f) to (%.4f, %.4f)\n",
               m_ValidContentBounds.startX, m_ValidContentBounds.startY,
               m_ValidContentBounds.endX, m_ValidContentBounds.endY);
        printf("Note: Transform rect NOT changed. Valid bounds will be used for Ctrl+T.\n");
        printf("==================================\n\n");
    } else {
        // 没有有效像素（全部透明），清除有效边界
        m_ValidContentBounds.Reset();
        printf("\n[Warning] No valid pixels found after deletion.\n\n");
    }
    
    printf("================================\n\n");
    
    return true;
}

bool PreviewPanel::Undo() {
    ImageData restoredImage;
    std::string description;
    
    if (!m_ImageHistory.Undo(restoredImage, description)) {
        printf("[Undo] Cannot undo: no history available\n");
        return false;
    }
    
    printf("\n=== Undo Operation ===\n");
    printf("Restoring: '%s'\n", description.c_str());
    
    // 恢复图像数据
    m_CurrentImage = restoredImage;
    
    // 更新纹理
    if (!CreateTexture(m_CurrentImage)) {
        printf("[Undo] Failed to update texture.\n");
        return false;
    }
    
    // ✅ 标记图像为已修改（撤销也是一种修改）
    m_ImageModified = true;
    
    // ✅ 同步更新缓存（确保缓存与当前状态一致）
    if (!m_CurrentImagePath.empty()) {
        ImageCache cache;
        cache.imageData = m_CurrentImage;
        cache.modified = true;
        cache.history = m_ImageHistory;  // ✅ 保存历史记录
        cache.validBounds = m_ValidContentBounds;  // ✅ 保存有效内容边界
        m_ImageCache[m_CurrentImagePath] = cache;
        printf("[Undo] Updated cache for: %s\n", m_CurrentImagePath.c_str());
    }
    
    printf("Undo completed successfully.\n");
    printf("======================\n\n");
    
    return true;
}

bool PreviewPanel::Redo() {
    ImageData restoredImage;
    std::string description;
    
    if (!m_ImageHistory.Redo(restoredImage, description)) {
        printf("[Redo] Cannot redo: already at latest state\n");
        return false;
    }
    
    printf("\n=== Redo Operation ===\n");
    printf("Restoring: '%s'\n", description.c_str());
    
    // 恢复图像数据
    m_CurrentImage = restoredImage;
    
    // 更新纹理
    if (!CreateTexture(m_CurrentImage)) {
        printf("[Redo] Failed to update texture.\n");
        return false;
    }
    
    // ✅ 标记图像为已修改（重做也是一种修改）
    m_ImageModified = true;
    
    // ✅ 同步更新缓存（确保缓存与当前状态一致）
    if (!m_CurrentImagePath.empty()) {
        ImageCache cache;
        cache.imageData = m_CurrentImage;
        cache.modified = true;
        cache.history = m_ImageHistory;  // ✅ 保存历史记录
        cache.validBounds = m_ValidContentBounds;  // ✅ 保存有效内容边界
        m_ImageCache[m_CurrentImagePath] = cache;
        printf("[Redo] Updated cache for: %s\n", m_CurrentImagePath.c_str());
    }
    
    printf("Redo completed successfully.\n");
    printf("======================\n\n");
    
    return true;
}

