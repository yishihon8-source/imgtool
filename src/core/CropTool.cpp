#include "CropTool.h"
#include <algorithm>

CropTool::CropTool() = default;
CropTool::~CropTool() = default;

void CropTool::Activate(int canvasWidth, int canvasHeight) {
    m_State = CropToolState::Active;
    ResetCropRect(canvasWidth, canvasHeight);
}

void CropTool::Deactivate() {
    m_State = CropToolState::Inactive;
    m_DraggingHandle = CropHandle::None;
}

void CropTool::ResetCropRect(int canvasWidth, int canvasHeight) {
    // ✅ Photoshop 行为：默认裁剪框覆盖整个画布
    m_CropRect.left = 0.0;
    m_CropRect.top = 0.0;
    m_CropRect.right = static_cast<double>(canvasWidth);
    m_CropRect.bottom = static_cast<double>(canvasHeight);
}

bool CropTool::ApplyCrop(ProcessConfig& config) {
    if (!m_CropRect.IsValid()) {
        return false;
    }
    
    // 归一化裁剪矩形
    CropRect normalized = m_CropRect.GetNormalized();
    
    // 收缩到画布边界内
    normalized.ClampTo(0, 0, config.canvas.width, config.canvas.height);
    
    // 计算新的画布尺寸
    int newWidth = static_cast<int>(normalized.GetWidth());
    int newHeight = static_cast<int>(normalized.GetHeight());
    
    if (newWidth <= 0 || newHeight <= 0) {
        return false;
    }
    
    // ✅ 保存裁剪信息（偏移量和尺寸）
    // 这样可以在单张图片上应用裁剪，而不影响全局配置
    m_CropOffsetX = static_cast<int>(normalized.left);
    m_CropOffsetY = static_cast<int>(normalized.top);
    m_CropWidth = newWidth;
    m_CropHeight = newHeight;
    
    return true;
}

void CropTool::ClampToCanvas(int canvasWidth, int canvasHeight) {
    m_CropRect.ClampTo(0, 0, canvasWidth, canvasHeight);
}

