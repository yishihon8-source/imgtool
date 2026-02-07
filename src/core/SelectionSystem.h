#pragma once

#include "SelectionHistory.h"
#include "SelectionMath.h"
#include <imgui.h>

/**
 * @file SelectionSystem.h
 * @brief 选区系统 - Photoshop 级选框工具实现
 * 
 * 架构（遵循 RENAME_FIX.md 第十节）：
 * - SelectionModel: 选区数据（本文件的 SelectionRect）
 * - SelectionController: 交互与状态更新（本文件的 SelectionSystem）
 * - SelectionMath: 几何判断与交集计算（独立模块 SelectionMath.h）
 * - SelectionRenderer: 蚂蚁线与遮罩渲染（RenderMarchingAnts / RenderMask）
 * - OutOfBoundsRenderer: 越界粉色线渲染（独立模块）
 */

/**
 * @brief 选区矩形数据模型
 * 
 * 职责：存储选区几何数据（Model 层）
 * 禁止：不应包含越界逻辑或渲染逻辑
 */
struct SelectionRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    bool active = false;

    // ✅ 获取标准化矩形（处理负宽高）
    // 标准化后：width >= 0, height >= 0, (x,y) 为左上角
    SelectionRect GetNormalized() const {
        SelectionRect normalized;
        normalized.x = (width >= 0) ? x : (x + width);
        normalized.y = (height >= 0) ? y : (y + height);
        normalized.width = (width >= 0) ? width : -width;
        normalized.height = (height >= 0) ? height : -height;
        normalized.active = active;
        return normalized;
    }

    // ✅ 判断点是否在选区内（使用标准化矩形）
    bool Contains(float px, float py) const {
        if (!active) return false;
        SelectionRect norm = GetNormalized();
        return px >= norm.x && px <= (norm.x + norm.width) &&
               py >= norm.y && py <= (norm.y + norm.height);
    }

    // ✅ 清空选区
    void Clear() {
        x = y = width = height = 0.0f;
        active = false;
    }

    // ✅ 判断选区是否有效
    bool IsValid() const {
        return active && (width != 0.0f || height != 0.0f);
    }
};

/**
 * @brief 选区系统控制器
 * 
 * 职责：
 * - 处理鼠标交互（拖拽、创建、移动）
 * - 管理选区状态生命周期
 * - 触发历史记录保存
 * - 协调选区渲染（通过调用专门的渲染方法）
 * 
 * 禁止：
 * - 不负责越界判断（由 SelectionMath / OutOfBoundsRenderer 负责）
 * - 不负责坐标系转换细节（由 SelectionMath 负责）
 */
class SelectionSystem {
public:
    SelectionSystem();

    // ========== 交互与更新 ==========

    /**
     * @brief 更新选区（处理鼠标交互）
     * @param mousePos 鼠标位置（画布逻辑坐标）
     * @param isMouseDown 鼠标是否按下
     * @param isMouseClicked 鼠标是否刚点击（本帧新按下）
     * @param shiftPressed Shift 键是否按下（影响选区形状）
     * @param altPressed Alt 键是否按下（影响选区方向）
     */
    void Update(const ImVec2& mousePos, bool isMouseDown, bool isMouseClicked, 
                bool shiftPressed, bool altPressed);

    // ========== 渲染 ==========

    /**
     * @brief 渲染选区视觉元素
     * @param drawList ImGui 绘制列表
     * @param canvasMin 画布屏幕坐标最小值
     * @param canvasMax 画布屏幕坐标最大值
     * @param deltaTime 帧时间差（用于蚂蚁线动画）
     * @param canvasZoom 画布缩放比例（用于坐标转换）
     * @param layerBounds 图层像素边界（画布逻辑坐标，可选）
     * 
     * 渲染顺序（自下而上）：
     * 1. 半透明遮罩（选区外区域）
     * 2. 蚂蚁线（选区边界）
     * 注：越界粉色线由 OutOfBoundsRenderer 单独处理
     * 
     * ✅ PS 风格选区自动收缩（遵循 RENAME_FIX.md）：
     * - 如果提供了 layerBounds，最终显示的选区会自动裁剪到图层范围内
     * - 用户可以自由拖拽，但显示的选区永远不会超出图层
     */
    void Render(ImDrawList* drawList, const ImVec2& canvasMin, const ImVec2& canvasMax, 
                float deltaTime, float canvasZoom = 1.0f, 
                const SelectionRect* layerBounds = nullptr);

    // ========== 数据访问 ==========

    // 获取当前选区（模型数据）
    const SelectionRect& GetSelection() const { return m_Selection; }

    // 清空选区
    void ClearSelection();

    // 判断是否有活动选区
    bool HasActiveSelection() const { return m_Selection.active; }

    // ========== 历史记录 ==========

    void Undo();
    void Redo();
    bool CanUndo() const { return m_History.CanUndo(); }
    bool CanRedo() const { return m_History.CanRedo(); }

    // ========== 工具函数 ==========

    // 应用吸附偏移到选区
    void ApplySnapOffset(const ImVec2& offset);

    // ✅ PS 风格：结束拖拽后自动收缩选区到图层范围
    // 这个方法应该在鼠标松开时调用（结算阶段）
    void ClampSelectionToLayer(const SelectionRect& layerBounds);

    // ✅ 方向键移动选区（PS 风格）
    // @param offsetX 水平偏移量（画布逻辑坐标）
    // @param offsetY 垂直偏移量（画布逻辑坐标）
    void MoveSelectionByOffset(float offsetX, float offsetY);

private:
    // ========== 数据 ==========
    SelectionRect m_Selection;      // 选区数据模型
    ImVec2 m_AnchorPoint;           // 拖拽起点
    ImVec2 m_MoveOffset;            // 移动选区时的偏移
    bool m_IsCreating;              // 是否正在创建选区
    bool m_IsMoving;                // 是否正在移动选区
    float m_DashOffset;             // 蚂蚁线动画偏移（用于实时更新效果）
    SelectionHistory m_History;     // 历史记录系统

    // ========== 业务逻辑（Controller 职责）==========

    // 创建新选区（处理不同修饰键组合）
    void CreateSelection(const ImVec2& mousePos, bool shiftPressed, bool altPressed);

    // 移动选区
    void MoveSelection(const ImVec2& mousePos);

    // 提交选区到历史记录（MouseUp 时调用）
    void CommitSelection();

    // ========== 渲染工具（Renderer 职责）==========

    /**
     * @brief 渲染蚂蚁线（选区边界动画）
     * 
     * 蚂蚁线是 Photoshop 标志性的选区标记：
     * - 交替的黑白虚线
     * - 以 dashOffset 进行逐帧滚动动画
     * - 在选区四条边上绘制
     */
    void RenderMarchingAnts(ImDrawList* drawList, const SelectionRect& rect, float dashOffset);

    /**
     * @brief 渲染半透明遮罩（选区外区域）
     * 
     * 半透明遮罩帮助用户识别选区范围：
     * - 选区内：无遮罩
     * - 选区外：半透明黑色覆盖
     */
    void RenderMask(ImDrawList* drawList, const ImVec2& canvasMin, const ImVec2& canvasMax, 
                   const SelectionRect& rect);
};

