#pragma once

#include "CropTool.h"
#include "CropRenderer.h"
#include <imgui.h>

/**
 * @brief Crop Tool 交互系统
 * 
 * 职责：
 * - HitTest（检测鼠标点击的控制点或区域）
 * - 处理 8 个 resize handles 的拖拽
 * - 处理整个裁剪框的移动
 * - 支持键盘快捷键（方向键移动、Shift 等比例缩放）
 * - 屏幕坐标与画布逻辑坐标的转换
 * 
 * 设计原则：
 * - 完全对标 Photoshop 行为
 * - 支持 canvas zoom and pan
 * - 高性能（8K 图像实时响应）
 */
class CropInteraction {
public:
    CropInteraction();
    ~CropInteraction();
    
    /**
     * @brief 更新交互状态
     * @param cropTool 裁剪工具实例
     * @param mousePos 鼠标位置（屏幕坐标）
     * @param canvasMin 画布左上角（屏幕坐标）
     * @param canvasMax 画布右下角（屏幕坐标）
     * @param canvasLogicalSize 画布逻辑尺寸
     * @param isMouseDown 鼠标左键是否按下
     * @param isMouseClicked 鼠标左键是否刚点击
     * @param isMouseReleased 鼠标左键是否刚释放
     * @param shiftPressed Shift 键是否按下
     * @param altPressed Alt 键是否按下
     */
    void Update(CropTool& cropTool,
                const ImVec2& mousePos,
                const ImVec2& canvasMin,
                const ImVec2& canvasMax,
                const ImVec2& canvasLogicalSize,
                bool isMouseDown,
                bool isMouseClicked,
                bool isMouseReleased,
                bool shiftPressed,
                bool altPressed);
    
    /**
     * @brief 获取当前悬停的控制点
     */
    CropHandle GetHoveredHandle() const { return m_HoveredHandle; }
    
    /**
     * @brief 处理键盘输入（方向键移动裁剪框）
     * @param cropTool 裁剪工具实例
     * @param canvasWidth 画布宽度
     * @param canvasHeight 画布高度
     */
    void HandleKeyboardInput(CropTool& cropTool, int canvasWidth, int canvasHeight);
    
    /**
     * @brief 屏幕坐标转画布逻辑坐标
     */
    ImVec2 ScreenToCanvas(const ImVec2& screenPos,
                         const ImVec2& canvasMin,
                         const ImVec2& canvasMax,
                         const ImVec2& canvasLogicalSize) const;
    
    /**
     * @brief 画布逻辑坐标转屏幕坐标
     */
    ImVec2 CanvasToScreen(const ImVec2& canvasPos,
                         const ImVec2& canvasMin,
                         const ImVec2& canvasMax,
                         const ImVec2& canvasLogicalSize) const;
    
    /**
     * @brief 计算8个控制点的屏幕坐标
     */
    void CalculateHandlePositions(const CropRect& cropRect,
                                  const ImVec2& canvasMin,
                                  const ImVec2& canvasMax,
                                  const ImVec2& canvasLogicalSize,
                                  ImVec2 outHandles[8]) const;
    
private:
    /**
     * @brief HitTest - 检测鼠标点击的控制点
     * @return 返回点击的控制点，如果没有点击任何控制点返回 None
     */
    CropHandle HitTestHandles(const ImVec2& mousePos,
                             const ImVec2 handles[8]) const;
    
    /**
     * @brief HitTest - 检测鼠标是否在裁剪框内（用于移动）
     */
    bool HitTestCropRect(const ImVec2& mousePos,
                        const ImVec2& cropMin,
                        const ImVec2& cropMax) const;
    
    /**
     * @brief 处理裁剪框创建（拖拽创建新裁剪框）
     */
    void HandleCreating(CropTool& cropTool,
                       const ImVec2& mouseLogical,
                       bool isMouseDown,
                       bool isMouseReleased,
                       bool shiftPressed,
                       bool altPressed);
    
    /**
     * @brief 处理裁剪框调整大小
     */
    void HandleResizing(CropTool& cropTool,
                       const ImVec2& mouseLogical,
                       bool isMouseDown,
                       bool shiftPressed,
                       bool altPressed);
    
    /**
     * @brief 处理裁剪框移动
     */
    void HandleMoving(CropTool& cropTool,
                     const ImVec2& mouseLogical,
                     bool isMouseDown);
    
    /**
     * @brief 更新鼠标光标样式
     */
    void UpdateMouseCursor(CropHandle handle) const;
    
private:
    CropHandle m_HoveredHandle = CropHandle::None;
    ImVec2 m_DragStartMousePos;  // 拖拽开始时的鼠标位置（画布逻辑坐标）
    
    const float m_HandleHitRadius = 12.0f;  // 控制点点击半径（屏幕坐标）
};

