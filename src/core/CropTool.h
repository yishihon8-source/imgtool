#pragma once

#include "Types.h"
#include <imgui.h>

/**
 * @brief 裁剪矩形数据结构（画布逻辑坐标）
 * 
 * Photoshop 规范：
 * - 裁剪框使用画布逻辑坐标系（与图像无关）
 * - 支持超出画布边界（拖拽时自由，松手后自动收缩）
 * - 非破坏性裁剪（不修改原图，只改变画布尺寸）
 */
struct CropRect {
    double left = 0.0;
    double top = 0.0;
    double right = 0.0;
    double bottom = 0.0;
    
    double GetWidth() const { return right - left; }
    double GetHeight() const { return bottom - top; }
    double GetCenterX() const { return (left + right) * 0.5; }
    double GetCenterY() const { return (top + bottom) * 0.5; }
    
    bool IsValid() const { return GetWidth() > 0 && GetHeight() > 0; }
    
    // 归一化（确保 left < right, top < bottom）
    CropRect GetNormalized() const {
        CropRect result = *this;
        if (result.left > result.right) std::swap(result.left, result.right);
        if (result.top > result.bottom) std::swap(result.top, result.bottom);
        return result;
    }
    
    // 收缩到指定边界内
    void ClampTo(double minX, double minY, double maxX, double maxY) {
        left = std::max(minX, std::min(left, maxX));
        right = std::max(minX, std::min(right, maxX));
        top = std::max(minY, std::min(top, maxY));
        bottom = std::max(minY, std::min(bottom, maxY));
    }
};

/**
 * @brief 裁剪工具状态
 */
enum class CropToolState {
    Inactive,       // 未激活
    Creating,       // 正在创建裁剪框（拖拽中）
    Active,         // 裁剪框已创建，等待调整
    Resizing,       // 正在调整大小
    Moving          // 正在移动
};

/**
 * @brief 裁剪控制点（8个resize handles）
 */
enum class CropHandle {
    None = -1,
    TopLeft = 0,
    TopCenter = 1,
    TopRight = 2,
    MiddleRight = 3,
    BottomRight = 4,
    BottomCenter = 5,
    BottomLeft = 6,
    MiddleLeft = 7,
    Center = 8  // 用于移动整个裁剪框
};

/**
 * @brief Photoshop 级 Crop Tool 核心类
 * 
 * 职责：
 * - 管理裁剪矩形状态
 * - 提供非破坏性裁剪接口
 * - 支持画布坐标系变换
 * 
 * 设计原则：
 * - 数据与渲染分离
 * - 数据与交互分离
 * - 支持 8K 图像实时编辑
 */
class CropTool {
public:
    CropTool();
    ~CropTool();
    
    /**
     * @brief 激活裁剪工具
     * @param canvasWidth 画布宽度
     * @param canvasHeight 画布高度
     */
    void Activate(int canvasWidth, int canvasHeight);
    
    /**
     * @brief 取消激活
     */
    void Deactivate();
    
    /**
     * @brief 是否激活
     */
    bool IsActive() const { return m_State != CropToolState::Inactive; }
    
    /**
     * @brief 获取当前状态
     */
    CropToolState GetState() const { return m_State; }
    
    /**
     * @brief 设置状态
     */
    void SetState(CropToolState state) { m_State = state; }
    
    /**
     * @brief 获取裁剪矩形（画布逻辑坐标）
     */
    const CropRect& GetCropRect() const { return m_CropRect; }
    
    /**
     * @brief 设置裁剪矩形
     */
    void SetCropRect(const CropRect& rect) { m_CropRect = rect; }
    
    /**
     * @brief 获取当前拖拽的控制点
     */
    CropHandle GetDraggingHandle() const { return m_DraggingHandle; }
    
    /**
     * @brief 设置拖拽控制点
     */
    void SetDraggingHandle(CropHandle handle) { m_DraggingHandle = handle; }
    
    /**
     * @brief 获取拖拽起始矩形（用于增量计算）
     */
    const CropRect& GetDragStartRect() const { return m_DragStartRect; }
    
    /**
     * @brief 保存拖拽起始状态
     */
    void SaveDragStartState() { m_DragStartRect = m_CropRect; }
    
    /**
     * @brief 重置裁剪框到默认状态（覆盖整个画布，Photoshop 行为）
     */
    void ResetCropRect(int canvasWidth, int canvasHeight);
    
    /**
     * @brief 应用裁剪（非破坏性）
     * @param config 处理配置（会修改 canvas 尺寸）
     * @return 成功返回 true
     * 
     * 行为：
     * - 修改 config.canvas 的尺寸为裁剪后的尺寸
     * - 不修改原图数据
     * - 后续渲染会自动应用新的画布尺寸
     */
    bool ApplyCrop(ProcessConfig& config);
    
    /**
     * @brief 收缩裁剪框到画布边界内
     */
    void ClampToCanvas(int canvasWidth, int canvasHeight);
    
    /**
     * @brief 是否显示 Rule of Thirds 网格
     */
    bool IsShowingRuleOfThirds() const { return m_ShowRuleOfThirds; }
    
    /**
     * @brief 切换 Rule of Thirds 网格显示
     */
    void ToggleRuleOfThirds() { m_ShowRuleOfThirds = !m_ShowRuleOfThirds; }
    
    /**
     * @brief 获取裁剪偏移量和尺寸（用于单张图片裁剪）
     */
    void GetCropInfo(int& offsetX, int& offsetY, int& width, int& height) const {
        offsetX = m_CropOffsetX;
        offsetY = m_CropOffsetY;
        width = m_CropWidth;
        height = m_CropHeight;
    }
    
    /**
     * @brief 检查是否有有效的裁剪信息
     */
    bool HasCropInfo() const {
        return m_CropWidth > 0 && m_CropHeight > 0;
    }
    
private:
    CropToolState m_State = CropToolState::Inactive;
    CropRect m_CropRect;
    CropRect m_DragStartRect;  // 拖拽开始时的矩形（用于增量计算）
    CropHandle m_DraggingHandle = CropHandle::None;
    bool m_ShowRuleOfThirds = true;  // 默认显示三分法网格
    
    // ✅ 裁剪信息（用于单张图片裁剪，不影响全局配置）
    int m_CropOffsetX = 0;
    int m_CropOffsetY = 0;
    int m_CropWidth = 0;
    int m_CropHeight = 0;
};

