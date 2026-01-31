#include "ImageProcessor.h"
#include <algorithm>
#include <cmath>
#include <cstring>

ImageData ImageProcessor::CreateCanvas(const Canvas& canvas) {
    ImageData result;
    result.width = canvas.width;
    result.height = canvas.height;
    result.channels = 4; // RGBA

    size_t totalPixels = canvas.width * canvas.height;
    result.pixels.resize(totalPixels * 4);

    // 填充背景色
    for (size_t i = 0; i < totalPixels; ++i) {
        size_t offset = i * 4;
        result.pixels[offset + 0] = canvas.background.r;
        result.pixels[offset + 1] = canvas.background.g;
        result.pixels[offset + 2] = canvas.background.b;
        result.pixels[offset + 3] = canvas.background.a;
    }

    return result;
}

ImageData ImageProcessor::Crop(const ImageData& source, const Rect& region) {
    if (!source.IsValid() || !region.IsValid()) {
        return ImageData();
    }

    // 裁剪区域边界检查
    int x = std::max(0, region.x);
    int y = std::max(0, region.y);
    int w = std::min(region.width, source.width - x);
    int h = std::min(region.height, source.height - y);

    if (w <= 0 || h <= 0) {
        return ImageData();
    }

    ImageData result;
    result.width = w;
    result.height = h;
    result.channels = source.channels;
    result.pixels.resize(w * h * source.channels);

    // 逐行复制
    for (int row = 0; row < h; ++row) {
        size_t srcOffset = ((y + row) * source.width + x) * source.channels;
        size_t dstOffset = row * w * source.channels;
        std::memcpy(
            result.pixels.data() + dstOffset,
            source.pixels.data() + srcOffset,
            w * source.channels
        );
    }

    return result;
}

ImageData ImageProcessor::Resize(const ImageData& source, int targetWidth, int targetHeight) {
    if (!source.IsValid() || targetWidth <= 0 || targetHeight <= 0) {
        return ImageData();
    }

    ImageData result;
    result.width = targetWidth;
    result.height = targetHeight;
    result.channels = source.channels;
    result.pixels.resize(targetWidth * targetHeight * source.channels);

    // 双线性插值
    float xRatio = static_cast<float>(source.width) / targetWidth;
    float yRatio = static_cast<float>(source.height) / targetHeight;

    for (int y = 0; y < targetHeight; ++y) {
        for (int x = 0; x < targetWidth; ++x) {
            float srcX = x * xRatio;
            float srcY = y * yRatio;

            int x1 = static_cast<int>(srcX);
            int y1 = static_cast<int>(srcY);
            int x2 = std::min(x1 + 1, source.width - 1);
            int y2 = std::min(y1 + 1, source.height - 1);

            float dx = srcX - x1;
            float dy = srcY - y1;

            for (int c = 0; c < source.channels; ++c) {
                // 获取四个邻近像素
                uint8_t p11 = source.pixels[(y1 * source.width + x1) * source.channels + c];
                uint8_t p21 = source.pixels[(y1 * source.width + x2) * source.channels + c];
                uint8_t p12 = source.pixels[(y2 * source.width + x1) * source.channels + c];
                uint8_t p22 = source.pixels[(y2 * source.width + x2) * source.channels + c];

                // 双线性插值
                float value = p11 * (1 - dx) * (1 - dy) +
                              p21 * dx * (1 - dy) +
                              p12 * (1 - dx) * dy +
                              p22 * dx * dy;

                result.pixels[(y * targetWidth + x) * source.channels + c] = 
                    static_cast<uint8_t>(value);
            }
        }
    }

    return result;
}

void ImageProcessor::DrawToCanvas(ImageData& canvas, const ImageLayer& layer) {
    if (!canvas.IsValid() || !layer.image.IsValid()) {
        return;
    }

    const ImageData& img = layer.image;
    const Rect& pos = layer.position;

    // 计算实际绘制区域（裁剪到画布范围内）
    int startX = std::max(0, pos.x);
    int startY = std::max(0, pos.y);
    int endX = std::min(canvas.width, pos.x + pos.width);
    int endY = std::min(canvas.height, pos.y + pos.height);

    if (startX >= endX || startY >= endY) {
        return;
    }

    // 绘制
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            // 计算源图像坐标
            int srcX = x - pos.x;
            int srcY = y - pos.y;

            if (srcX < 0 || srcX >= img.width || srcY < 0 || srcY >= img.height) {
                continue;
            }

            size_t canvasOffset = (y * canvas.width + x) * canvas.channels;
            size_t imgOffset = (srcY * img.width + srcX) * img.channels;

            BlendPixel(
                canvas.pixels.data() + canvasOffset,
                img.pixels.data() + imgOffset,
                img.channels
            );
        }
    }
}

void ImageProcessor::BlendPixel(uint8_t* dest, const uint8_t* src, int channels) {
    if (channels == 4) {
        // Alpha 混合
        float alpha = src[3] / 255.0f;
        float invAlpha = 1.0f - alpha;

        dest[0] = static_cast<uint8_t>(src[0] * alpha + dest[0] * invAlpha);
        dest[1] = static_cast<uint8_t>(src[1] * alpha + dest[1] * invAlpha);
        dest[2] = static_cast<uint8_t>(src[2] * alpha + dest[2] * invAlpha);
        dest[3] = 255; // 画布始终不透明
    } else {
        // 直接覆盖
        for (int i = 0; i < channels; ++i) {
            dest[i] = src[i];
        }
    }
}

Rect ImageProcessor::CalculatePosition(int imageWidth, int imageHeight,
                                        const Canvas& canvas, Alignment alignment) {
    Rect result;
    result.width = imageWidth;
    result.height = imageHeight;

    switch (alignment) {
        case Alignment::TopLeft:
            result.x = 0;
            result.y = 0;
            break;
        case Alignment::TopCenter:
            result.x = (canvas.width - imageWidth) / 2;
            result.y = 0;
            break;
        case Alignment::TopRight:
            result.x = canvas.width - imageWidth;
            result.y = 0;
            break;
        case Alignment::MiddleLeft:
            result.x = 0;
            result.y = (canvas.height - imageHeight) / 2;
            break;
        case Alignment::MiddleCenter:
            result.x = (canvas.width - imageWidth) / 2;
            result.y = (canvas.height - imageHeight) / 2;
            break;
        case Alignment::MiddleRight:
            result.x = canvas.width - imageWidth;
            result.y = (canvas.height - imageHeight) / 2;
            break;
        case Alignment::BottomLeft:
            result.x = 0;
            result.y = canvas.height - imageHeight;
            break;
        case Alignment::BottomCenter:
            result.x = (canvas.width - imageWidth) / 2;
            result.y = canvas.height - imageHeight;
            break;
        case Alignment::BottomRight:
            result.x = canvas.width - imageWidth;
            result.y = canvas.height - imageHeight;
            break;
    }

    return result;
}

std::pair<int, int> ImageProcessor::CalculateScaledSize(int sourceWidth, int sourceHeight,
                                                         int targetWidth, int targetHeight,
                                                         ScaleMode mode) {
    switch (mode) {
        case ScaleMode::None:
            return {sourceWidth, sourceHeight};

        case ScaleMode::Fit: {
            // 适应（保持比例，完整显示）
            float scaleX = static_cast<float>(targetWidth) / sourceWidth;
            float scaleY = static_cast<float>(targetHeight) / sourceHeight;
            float scale = std::min(scaleX, scaleY);
            return {
                static_cast<int>(sourceWidth * scale),
                static_cast<int>(sourceHeight * scale)
            };
        }

        case ScaleMode::Fill: {
            // 填充（保持比例，可能裁剪）
            float scaleX = static_cast<float>(targetWidth) / sourceWidth;
            float scaleY = static_cast<float>(targetHeight) / sourceHeight;
            float scale = std::max(scaleX, scaleY);
            return {
                static_cast<int>(sourceWidth * scale),
                static_cast<int>(sourceHeight * scale)
            };
        }

        case ScaleMode::Stretch:
            // 拉伸（不保持比例）
            return {targetWidth, targetHeight};

        case ScaleMode::FixedWidth: {
            // 固定宽度
            float scale = static_cast<float>(targetWidth) / sourceWidth;
            return {
                targetWidth,
                static_cast<int>(sourceHeight * scale)
            };
        }

        case ScaleMode::FixedHeight: {
            // 固定高度
            float scale = static_cast<float>(targetHeight) / sourceHeight;
            return {
                static_cast<int>(sourceWidth * scale),
                targetHeight
            };
        }
    }

    return {sourceWidth, sourceHeight};
}

ImageData ImageProcessor::Process(const ImageData& source, const ProcessConfig& config,
                                   const ImageTransformState* transformState) {
    if (!source.IsValid()) {
        return ImageData();
    }

    ImageData processed = source;

    // 1. 裁剪
    if (config.crop.enabled && config.crop.region.IsValid()) {
        processed = Crop(processed, config.crop.region);
        if (!processed.IsValid()) {
            return ImageData();
        }
    }

    // 2. 如果有用户变换，应用变换（PS 矩形模型）
    if (transformState && transformState->hasTransform) {
        // ✅ PS 矩形模型：transformState 中存储的是矩形的四条边
        // scaleX = left, scaleY = top, positionX = right, positionY = bottom
        double rectLeft = transformState->scaleX;
        double rectTop = transformState->scaleY;
        double rectRight = transformState->positionX;
        double rectBottom = transformState->positionY;
        
        double rectWidth = rectRight - rectLeft;
        double rectHeight = rectBottom - rectTop;
        
        // 如果矩形有效，使用用户的变换
        if (rectWidth > 0 && rectHeight > 0) {
            // 缩放图片到用户指定的尺寸
            int targetWidth = static_cast<int>(rectWidth);
            int targetHeight = static_cast<int>(rectHeight);
            
            if (targetWidth > 0 && targetHeight > 0) {
                processed = Resize(processed, targetWidth, targetHeight);
                if (!processed.IsValid()) {
                    return ImageData();
                }
            }
            
            // 创建画布
            ImageData canvas = CreateCanvas(config.canvas);
            
            // 使用用户指定的位置（矩形的左上角）
            Rect position;
            position.x = static_cast<int>(rectLeft);
            position.y = static_cast<int>(rectTop);
            position.width = processed.width;
            position.height = processed.height;
            
            // 绘制到画布
            ImageLayer layer;
            layer.image = processed;
            layer.position = position;
            DrawToCanvas(canvas, layer);
            
            return canvas;
        }
    }
    
    // 没有用户变换或矩形无效，使用默认的缩放模式
    {
        // 2. 计算缩放尺寸
        auto [scaledWidth, scaledHeight] = CalculateScaledSize(
            processed.width, processed.height,
            config.canvas.width, config.canvas.height,
            config.scaleMode
        );

        // 3. 缩放
        if (scaledWidth != processed.width || scaledHeight != processed.height) {
            processed = Resize(processed, scaledWidth, scaledHeight);
            if (!processed.IsValid()) {
                return ImageData();
            }
        }

        // 4. 创建画布
        ImageData canvas = CreateCanvas(config.canvas);

        // 5. 计算位置
        Rect position = CalculatePosition(
            processed.width, processed.height,
            config.canvas, config.alignment
        );

        // 6. 绘制到画布
        ImageLayer layer;
        layer.image = processed;
        layer.position = position;
        DrawToCanvas(canvas, layer);

        return canvas;
    }
}
