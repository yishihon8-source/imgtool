# PreviewPanel.cpp 功能集成补丁

本文档说明如何在 PreviewPanel.cpp 中集成 Ctrl+滚轮缩放和智能对齐辅助线功能。

## 修改 1: 添加头文件引用（第1-3行之间）

在文件开头的 `#include` 部分，添加新的管理器头文件：

```cpp
#include "PreviewPanel.h"
#include "core/ImageLoader.h"
#include "core/TransformManager.h"      // 新增
#include "core/GuideLineManager.h"      // 新增
#include <imgui.h>
```

## 修改 2: 在 RenderCanvasStage 函数中添加 Ctrl+滚轮缩放（约第260行，画布背景绘制之后）

找到这段代码：
```cpp
ImVec2 canvasMin(canvasX, canvasY);
ImVec2 canvasMax(canvasX + canvasDisplayWidth, canvasY + canvasDisplayHeight);

// 绘制画布背景
uint8_t r = config.canvas.background.r;
```

在 `canvasMax` 定义之后，`// 绘制画布背景` 之前，插入：

```cpp
ImVec2 canvasMin(canvasX, canvasY);
ImVec2 canvasMax(canvasX + canvasDisplayWidth, canvasY + canvasDisplayHeight);

// 处理 Ctrl + 滚轮缩放图片（PS 级体验）
ImGuiIO& ioZoom = ImGui::GetIO();
if (ImGui::IsWindowHovered() && ioZoom.MouseWheel != 0.0f && ioZoom.KeyCtrl && m_TransformMode) {
    float scaleFactor = (ioZoom.MouseWheel > 0) ? 1.1f : 0.9f;
    
    // 使用 TransformManager 进行以鼠标为中心的缩放
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 canvasSize(canvasDisplayWidth, canvasDisplayHeight);
    ImVec2 canvasLogicalSize(static_cast<float>(config.canvas.width), static_cast<float>(config.canvas.height));
    
    m_TransformManager.ZoomAtPoint(mousePos, canvasMin, canvasSize, canvasLogicalSize, scaleFactor);
    
    // 同步到旧的变量（兼容性）
    m_TransformScale = m_TransformManager.GetScale();
    m_TransformPosition = m_TransformManager.GetPosition();
}

// 绘制画布背景
uint8_t r = config.canvas.background.r;
```

## 修改 3: 在变换模式渲染中添加对齐辅助线（约第420行）

找到这段代码：
```cpp
// 变换模式：显示控制点
if (m_TransformMode) {
    RenderTransformControls(originalImageMin, originalImageMax);
}
```

替换为：

```cpp
// 变换模式：显示控制点和对齐辅助线
if (m_TransformMode) {
    RenderTransformControls(originalImageMin, originalImageMax);
    
    // 更新并渲染对齐辅助线（仅在拖拽图片时）
    if (m_IsDraggingImage) {
        // 计算图片中心在画布逻辑坐标系中的位置
        ImVec2 imageCenter;
        imageCenter.x = config.canvas.width * 0.5f + m_TransformPosition.x;
        imageCenter.y = config.canvas.height * 0.5f + m_TransformPosition.y;
        
        ImVec2 canvasLogicalSize(static_cast<float>(config.canvas.width), static_cast<float>(config.canvas.height));
        
        // 更新对齐状态
        m_GuideLineManager.Update(imageCenter, canvasLogicalSize, 8.0f);
        
        // 渲染对齐辅助线
        m_GuideLineManager.Render(drawList, canvasMin, canvasMax, canvasLogicalSize);
    }
}
```

## 修改 4: 在 HandleTransformInput 中标记拖拽状态（约第720行）

找到这段代码：
```cpp
if (m_DraggingHandle == TransformHandle::Center) {
    // 移动
    m_TransformPosition.x = m_DragStartPosition.x + delta.x;
    m_TransformPosition.y = m_DragStartPosition.y + delta.y;
} else {
```

替换为：

```cpp
if (m_DraggingHandle == TransformHandle::Center) {
    // 移动
    m_TransformPosition.x = m_DragStartPosition.x + delta.x;
    m_TransformPosition.y = m_DragStartPosition.y + delta.y;
    m_IsDraggingImage = true;  // 标记正在拖拽图片
} else {
```

## 修改 5: 在结束拖拽时清除辅助线（约第750行）

找到这段代码：
```cpp
// 结束拖拽
if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
    m_DraggingHandle = TransformHandle::None;
}
```

替换为：

```cpp
// 结束拖拽
if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
    m_DraggingHandle = TransformHandle::None;
    m_IsDraggingImage = false;  // 停止拖拽
    m_GuideLineManager.Clear();  // 清除辅助线
}
```

## 修改 6: 在 ResetTransform 中重置管理器（约第570行）

找到这段代码：
```cpp
void PreviewPanel::ResetTransform() {
    m_TransformScale = ImVec2(1.0f, 1.0f);
    m_TransformRotation = 0.0f;
    m_TransformPosition = ImVec2(0.0f, 0.0f);
    m_DraggingHandle = TransformHandle::None;
    m_HasCustomBounds = false;  // 重置自定义边界
}
```

替换为：

```cpp
void PreviewPanel::ResetTransform() {
    m_TransformScale = ImVec2(1.0f, 1.0f);
    m_TransformRotation = 0.0f;
    m_TransformPosition = ImVec2(0.0f, 0.0f);
    m_DraggingHandle = TransformHandle::None;
    m_HasCustomBounds = false;  // 重置自定义边界
    m_IsDraggingImage = false;
    
    // 重置 TransformManager
    m_TransformManager.Reset();
    m_GuideLineManager.Clear();
}
```

## 完成

完成以上6处修改后，保存文件。接下来需要更新 CMakeLists.txt 添加新文件。






