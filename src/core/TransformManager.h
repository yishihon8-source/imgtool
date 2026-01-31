#pragma once

#include <imgui.h>

/**
 * @brief 变换管理器
 * 
 * 职责：
 * - 管理图片的缩放、平移、旋转
 * - 实现以鼠标为中心的缩放（PS 级体验）
 * - 提供坐标变换计算
 */
class TransformManager {
public:
    TransformManager();
    ~TransformManager() = default;

    /**
     * @brief 重置变换
     */
    void Reset();

    /**
     * @brief 以指定点为中心进行缩放
     * @param mouseScreenPos 鼠标屏幕坐标
     * @param canvasMin 画布左上角屏幕坐标
     * @param canvasSize 画布显示尺寸（像素）
     * @param canvasLogicalSize 画布逻辑尺寸（配置中的尺寸）
     * @param scaleFactor 缩放因子（>1 放大，<1 缩小）
     */
    void ZoomAtPoint(const ImVec2& mouseScreenPos,
                     const ImVec2& canvasMin,
                     const ImVec2& canvasSize,
                     const ImVec2& canvasLogicalSize,
                     float scaleFactor);

    /**
     * @brief 平移图片
     * @param delta 平移增量（屏幕坐标）
     */
    void Translate(const ImVec2& delta);

    /**
     * @brief 设置缩放
     */
    void SetScale(const ImVec2& scale) { m_Scale = scale; }

    /**
     * @brief 设置位置
     */
    void SetPosition(const ImVec2& position) { m_Position = position; }

    /**
     * @brief 设置旋转
     */
    void SetRotation(float rotation) { m_Rotation = rotation; }

    /**
     * @brief 获取缩放
     */
    ImVec2 GetScale() const { return m_Scale; }

    /**
     * @brief 获取位置
     */
    ImVec2 GetPosition() const { return m_Position; }

    /**
     * @brief 获取旋转
     */
    float GetRotation() const { return m_Rotation; }

    /**
     * @brief 限制缩放范围
     */
    void ClampScale(float minScale = 0.01f, float maxScale = 20.0f);

private:
    ImVec2 m_Scale;      // 缩放比例
    ImVec2 m_Position;   // 位置偏移（画布逻辑坐标）
    float m_Rotation;    // 旋转角度（弧度）
};











