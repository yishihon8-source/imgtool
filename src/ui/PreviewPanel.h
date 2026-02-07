#pragma once

#include "core/Types.h"
#include "core/TransformManager.h"
#include "core/GuideLineManager.h"
#include "core/SelectionSystem.h"
#include "core/SelectionMath.h"
#include "core/OutOfBoundsRenderer.h"
#include "core/ImageHistory.h"
#include "core/TileImageHistory.h"
#include <imgui.h>
#include <vector>
#include <map>

/**
 * @brief 预览面板
 * 
 * 职责：
 * - 显示当前选中的图片
 * - 可视化裁剪框
 * - 实时预览处理效果
 * - Ctrl+滚轮缩放（PS级体验）
 * - 智能对齐辅助线
 */
class PreviewPanel {
public:
    PreviewPanel();
    ~PreviewPanel();

    /**
     * @brief 渲染面板
     * @param imageList 图片列表（非const，需要保存变换状态）
     * @param currentIndex 当前选中的索引
     * @param config 处理配置
     * @param canvasApplied 画布是否已应用
     * @param transformMode 是否启用变换模式（输入输出）
     * @param selectionMode 是否启用选区模式（输入输出）
     */
    void Render(std::vector<ImageInfo>& imageList, 
                int currentIndex,
                ProcessConfig& config,
                bool canvasApplied,
                bool& transformMode,
                bool& selectionMode);
    
    /**
     * @brief 保存当前图片的变换状态（供外部调用）
     * @param imageInfo 要保存到的图片信息
     */
    void SaveCurrentTransformState(ImageInfo& imageInfo);
    
    /**
     * @brief 获取缓存的图片数据（如果有修改）
     * @param filePath 图片路径
     * @param outImageData 输出图片数据
     * @return 如果有缓存且已修改返回 true
     */
    bool GetCachedImageData(const std::string& filePath, ImageData& outImageData) const;
    
    /**
     * @brief 获取当前图片的历史记录（只读）
     * @return 历史记录引用
     */
    const ImageHistory& GetImageHistory() const { return m_ImageHistory; }
    
    /**
     * @brief 撤销操作（公共接口）
     */
    bool Undo();
    
    /**
     * @brief 重做操作（公共接口）
     */
    bool Redo();
    
    /**
     * @brief 清除所有缓存（用于状态恢复）
     */
    void ClearCache();

private:
    /**
     * @brief 渲染画布舞台
     */
    void RenderCanvasStage(const ImageData& image, ProcessConfig& config, bool shouldEraseSelection);

    /**
     * @brief 渲染裁剪覆盖层
     */
    void RenderCropOverlay(Rect& cropRegion, const ImVec2& canvasPos, const ImVec2& canvasSize);

    /**
     * @brief 渲染空状态
     */
    void RenderEmptyState(const char* message, const ImVec4& color);

    /**
     * @brief 加载当前图片
     */
    bool LoadCurrentImage(const std::string& filePath);
    
    /**
     * @brief 创建 OpenGL 纹理
     */
    bool CreateTexture(const ImageData& imageData);
    
    /**
     * @brief 释放纹理
     */
    void ReleaseTexture();

private:
    ImageData m_CurrentImage;
    std::string m_CurrentImagePath;
    bool m_ImageModified = false;  // ✅ 标记图像是否被修改过
    
    // OpenGL 纹理
    unsigned int m_TextureID = 0;
    int m_TextureWidth = 0;
    int m_TextureHeight = 0;
    
    // 裁剪框拖拽状态
    bool m_IsDraggingCrop = false;
    bool m_IsResizingCrop = false;
    
    // 画布缩放和偏移（用于视图变换）
    float m_CanvasZoom = 1.0f;
    ImVec2 m_CanvasOffset = ImVec2(0.0f, 0.0f);  // 画布偏移（屏幕坐标）
    
    // 画布拖拽状态（空格键 + 鼠标拖拽）
    bool m_IsDraggingCanvas = false;
    ImVec2 m_CanvasDragStart;
    
    // 自由变换模式 (Ctrl+T) - Photoshop 工业级实现
    bool m_TransformMode = false;
    TransformManager m_TransformManager;                // 变换管理器（保留用于其他功能）
    
    // ✅ PS 正确模型：直接存储矩形的四条边（画布逻辑坐标系）
    struct TransformRect {
        double top = 0.0;
        double bottom = 0.0;
        double left = 0.0;
        double right = 0.0;
        
        double GetWidth() const { return right - left; }
        double GetHeight() const { return bottom - top; }
        double GetCenterX() const { return (left + right) * 0.5; }
        double GetCenterY() const { return (top + bottom) * 0.5; }
    };
    
    TransformRect m_TransformRect;           // 当前变换矩形
    TransformRect m_DragStartRect;           // 拖拽开始时的矩形（用于每帧重新计算）
    double m_DragStartAspect = 1.0;          // 拖拽开始时的宽高比
    
    // 兼容旧代码（用于保存/恢复状态）
    ImVec2 m_TransformScale = ImVec2(1.0f, 1.0f);
    float m_TransformRotation = 0.0f;
    ImVec2 m_TransformPosition = ImVec2(0.0f, 0.0f);
    
    // 对齐辅助线管理器
    GuideLineManager m_GuideLineManager;
    
    // 矩形选框工具（Rectangular Marquee Tool）
    bool m_SelectionMode = false;                       // 是否在选框模式
    SelectionSystem m_SelectionSystem;                  // 选区系统
    bool m_HasSelection = false;                        // 是否有活动选区
    OutOfBoundsRenderer m_OutOfBoundsRenderer;          // 选区越界警告线渲染器
    
    // 图像历史记录（撤销/重做）
    ImageHistory m_ImageHistory;                        // 历史记录管理器（旧系统）
    std::unique_ptr<TileImageHistory> m_TileImageHistory;  // Tile-based 历史记录（新系统）
    bool m_UseTileBasedHistory = true;                  // 是否使用 Tile-based 系统（默认启用）
    
    // ✅ 有效内容边界（用于删除后的变换框收缩）
    // 存储图片中非透明像素的边界（归一化坐标 0-1）
    struct ValidContentBounds {
        float startX = 0.0f;  // 有效内容起始 X（归一化）
        float startY = 0.0f;  // 有效内容起始 Y（归一化）
        float endX = 1.0f;    // 有效内容结束 X（归一化）
        float endY = 1.0f;    // 有效内容结束 Y（归一化）
        bool isValid = false; // 是否有有效边界
        
        void Reset() {
            startX = 0.0f;
            startY = 0.0f;
            endX = 1.0f;
            endY = 1.0f;
            isValid = false;
        }
    };
    ValidContentBounds m_ValidContentBounds;  // 当前图片的有效内容边界
    
    // ✅ 图片缓存系统（保存每张图片的修改状态和历史记录）
    struct ImageCache {
        ImageData imageData;
        bool modified = false;
        ImageHistory history;  // ✅ 保存每张图片的历史记录（旧系统）
        std::unique_ptr<TileImageHistory> tileHistory;  // ✅ Tile-based 历史记录（新系统）
        ValidContentBounds validBounds;  // ✅ 保存有效内容边界
    };
    std::map<std::string, ImageCache> m_ImageCache;  // 路径 -> 缓存数据
    
    // 变换控制点拖拽状态
    enum class TransformHandle {
        None = -1,
        TopLeft = 0,
        TopCenter = 1,
        TopRight = 2,
        MiddleRight = 3,
        BottomRight = 4,
        BottomCenter = 5,
        BottomLeft = 6,
        MiddleLeft = 7,
        Center = 8  // 中心点用于移动
    };
    
    TransformHandle m_DraggingHandle = TransformHandle::None;
    ImVec2 m_DragStartPos;
    ImVec2 m_DragStartScale;
    ImVec2 m_DragStartPosition;
    ImVec2 m_DragStartImageSize;  // 拖拽开始时图片的显示尺寸（屏幕坐标）
    
    // 拖拽状态（用于对齐辅助线）
    bool m_IsDraggingImage = false;
    
    // 辅助方法
    void RenderTransformControls(const ImVec2& imageMin, const ImVec2& imageMax);
    void HandleTransformInput(const ImVec2& imageMin, const ImVec2& imageMax);
    void ResetTransform();
    void SaveTransformState(ImageInfo& imageInfo);      // 保存变换状态
    void RestoreTransformState(const ImageInfo& imageInfo);  // 恢复变换状态
    
    // 选区相关辅助方法
    ImVec2 ScreenToCanvas(const ImVec2& screenPos, const ProcessConfig& config) const;  // 屏幕坐标转画布逻辑坐标
    ImVec2 CanvasToScreen(const ImVec2& canvasPos, const ProcessConfig& config) const;  // 画布逻辑坐标转屏幕坐标
    
    /**
     * @brief 删除选区内容（PS 风格）
     * @param config 处理配置
     * @return 成功返回 true
     * 
     * 行为：
     * - 如果没有选区，不执行任何操作
     * - 将选区内的像素 Alpha 设置为 0（透明）
     * - 不影响选区外的像素
     * - 删除后选区仍然保留
     */
    bool DeleteSelectionContent(const ProcessConfig& config);
    
    // 当前图片索引（用于跟踪切换）
    int m_LastImageIndex = -1;
};
