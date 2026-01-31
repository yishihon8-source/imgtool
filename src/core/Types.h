#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief 颜色结构（RGBA）
 */
struct Color {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;

    Color() = default;
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}

    static Color White() { return Color(255, 255, 255, 255); }
    static Color Black() { return Color(0, 0, 0, 255); }
    static Color Transparent() { return Color(0, 0, 0, 0); }
};

/**
 * @brief 矩形区域
 */
struct Rect {
    int x = 0;      // 左上角 X
    int y = 0;      // 左上角 Y
    int width = 0;  // 宽度
    int height = 0; // 高度

    Rect() = default;
    Rect(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}

    bool IsValid() const { return width > 0 && height > 0; }
    int Right() const { return x + width; }
    int Bottom() const { return y + height; }
};

/**
 * @brief 画布配置
 */
struct Canvas {
    int width = 1024;
    int height = 1024;
    Color background = Color::White();

    Canvas() = default;
    Canvas(int w, int h, const Color& bg = Color::White())
        : width(w), height(h), background(bg) {}
};

/**
 * @brief 图像数据
 */
struct ImageData {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<uint8_t> pixels;

    bool IsValid() const {
        return width > 0 && height > 0 && channels > 0 && !pixels.empty();
    }

    size_t GetSize() const {
        return static_cast<size_t>(width) * height * channels;
    }
};

/**
 * @brief 图层（图像在画布中的位置）
 */
struct ImageLayer {
    ImageData image;
    Rect position;      // 在画布中的位置
    float scale = 1.0f; // 缩放比例

    ImageLayer() = default;
};

/**
 * @brief 裁剪参数
 */
struct CropParams {
    bool enabled = false;
    Rect region;              // 裁剪区域
    bool keepAspectRatio = false;
    float aspectRatio = 1.0f; // 宽高比（宽/高）

    CropParams() = default;
};

/**
 * @brief 缩放模式
 */
enum class ScaleMode {
    None,           // 不缩放
    Fit,            // 适应（保持比例）
    Fill,           // 填充（可能裁剪）
    Stretch,        // 拉伸（不保持比例）
    FixedWidth,     // 固定宽度
    FixedHeight     // 固定高度
};

/**
 * @brief 对齐方式
 */
enum class Alignment {
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    MiddleCenter,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

/**
 * @brief 输出格式
 */
enum class OutputFormat {
    JPG,
    PNG
};

/**
 * @brief 处理配置
 */
struct ProcessConfig {
    Canvas canvas;
    CropParams crop;
    ScaleMode scaleMode = ScaleMode::Fit;
    Alignment alignment = Alignment::MiddleCenter;
    OutputFormat format = OutputFormat::PNG;
    int jpgQuality = 95; // 1-100

    ProcessConfig() = default;
};

/**
 * @brief 图片变换状态（用于保存每张图片的变换）
 */
struct ImageTransformState {
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float positionX = 0.0f;
    float positionY = 0.0f;
    float rotation = 0.0f;
    bool hasTransform = false;  // 是否有自定义变换
    
    ImageTransformState() = default;
};

/**
 * @brief 图片信息
 */
struct ImageInfo {
    std::string filePath;
    std::string fileName;
    int width = 0;
    int height = 0;
    int channels = 0;
    size_t fileSize = 0;
    
    // 保存每张图片的变换状态
    ImageTransformState transformState;

    bool IsValid() const {
        return !filePath.empty() && width > 0 && height > 0;
    }
};
