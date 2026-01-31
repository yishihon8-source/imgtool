#pragma once

#include "Types.h"

/**
 * @brief 图像处理器
 * 
 * 职责：
 * - 创建画布
 * - 图像裁剪
 * - 图像缩放
 * - 图像合成到画布
 * - 完整的处理流程
 * 
 * 注意：此类不依赖任何 UI 库
 */
class ImageProcessor {
public:
    /**
     * @brief 创建空白画布
     * @param canvas 画布配置
     * @return 画布图像数据
     */
    static ImageData CreateCanvas(const Canvas& canvas);

    /**
     * @brief 裁剪图像
     * @param source 源图像
     * @param region 裁剪区域
     * @return 裁剪后的图像
     */
    static ImageData Crop(const ImageData& source, const Rect& region);

    /**
     * @brief 缩放图像（双线性插值）
     * @param source 源图像
     * @param targetWidth 目标宽度
     * @param targetHeight 目标高度
     * @return 缩放后的图像
     */
    static ImageData Resize(const ImageData& source, int targetWidth, int targetHeight);

    /**
     * @brief 将图像绘制到画布上
     * @param canvas 画布图像
     * @param layer 图层（包含图像和位置）
     */
    static void DrawToCanvas(ImageData& canvas, const ImageLayer& layer);

    /**
     * @brief 计算图像在画布中的位置（根据对齐方式）
     * @param imageWidth 图像宽度
     * @param imageHeight 图像高度
     * @param canvas 画布配置
     * @param alignment 对齐方式
     * @return 位置矩形
     */
    static Rect CalculatePosition(int imageWidth, int imageHeight, 
                                   const Canvas& canvas, Alignment alignment);

    /**
     * @brief 计算缩放后的尺寸（根据缩放模式）
     * @param sourceWidth 源宽度
     * @param sourceHeight 源高度
     * @param targetWidth 目标宽度
     * @param targetHeight 目标高度
     * @param mode 缩放模式
     * @return 缩放后的尺寸（宽，高）
     */
    static std::pair<int, int> CalculateScaledSize(int sourceWidth, int sourceHeight,
                                                     int targetWidth, int targetHeight,
                                                     ScaleMode mode);

    /**
     * @brief 完整处理流程
     * @param source 源图像
     * @param config 处理配置
     * @param transformState 用户的变换状态（可选）
     * @return 处理后的图像
     */
    static ImageData Process(const ImageData& source, const ProcessConfig& config, 
                            const ImageTransformState* transformState = nullptr);

private:
    /**
     * @brief 混合两个像素（Alpha 混合）
     */
    static void BlendPixel(uint8_t* dest, const uint8_t* src, int channels);
};
