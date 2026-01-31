#pragma once

#include "core/Types.h"

/**
 * @brief 控制面板
 * 
 * 职责：
 * - 画布配置
 * - 裁剪参数
 * - 缩放模式
 * - 输出设置
 */
class ControlPanel {
public:
    ControlPanel() = default;
    ~ControlPanel() = default;

    /**
     * @brief 渲染面板
     * @param config 处理配置（输入输出）
     */
    void Render(ProcessConfig& config);

private:
    /**
     * @brief 渲染图层尺寸分组
     */
    void RenderTransformSection(Canvas& canvas);

    /**
     * @brief 渲染处理模式分组
     */
    void RenderAdjustmentsSection(ScaleMode& mode);

    /**
     * @brief 渲染水印分组
     */
    void RenderWatermarkSection();

    /**
     * @brief 渲染导出分组
     */
    void RenderExportSection(OutputFormat& format, int& jpgQuality);

    // 保留旧方法作为备用
    void RenderCanvasSection(Canvas& canvas);
    void RenderImageSection(CropParams& crop);
    void RenderLayoutSection(ScaleMode& mode, Alignment& alignment);
};
