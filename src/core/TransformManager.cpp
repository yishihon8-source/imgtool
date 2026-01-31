#include "TransformManager.h"
#include <algorithm>
#include <cmath>

TransformManager::TransformManager()
    : m_Scale(1.0f, 1.0f)
    , m_Position(0.0f, 0.0f)
    , m_Rotation(0.0f)
{
}

void TransformManager::Reset() {
    m_Scale = ImVec2(1.0f, 1.0f);
    m_Position = ImVec2(0.0f, 0.0f);
    m_Rotation = 0.0f;
}

void TransformManager::ZoomAtPoint(const ImVec2& mouseScreenPos,
                                   const ImVec2& canvasMin,
                                   const ImVec2& canvasSize,
                                   const ImVec2& canvasLogicalSize,
                                   float scaleFactor) {
    // 核心算法：确保鼠标指向的图像点在缩放前后保持在相同的屏幕位置
    
    // 1. 计算鼠标在画布中的相对位置（归一化坐标 0-1）
    ImVec2 mouseInCanvas;
    mouseInCanvas.x = (mouseScreenPos.x - canvasMin.x) / canvasSize.x;
    mouseInCanvas.y = (mouseScreenPos.y - canvasMin.y) / canvasSize.y;
    
    // 2. 转换为画布逻辑坐标
    ImVec2 mouseLogical;
    mouseLogical.x = mouseInCanvas.x * canvasLogicalSize.x;
    mouseLogical.y = mouseInCanvas.y * canvasLogicalSize.y;
    
    // 3. 计算图片中心在画布逻辑坐标系中的位置
    ImVec2 imageCenter;
    imageCenter.x = canvasLogicalSize.x * 0.5f + m_Position.x;
    imageCenter.y = canvasLogicalSize.y * 0.5f + m_Position.y;
    
    // 4. 计算鼠标点相对于图片中心的向量（缩放前）
    ImVec2 mouseToCenter;
    mouseToCenter.x = mouseLogical.x - imageCenter.x;
    mouseToCenter.y = mouseLogical.y - imageCenter.y;
    
    // 5. 保存旧缩放
    ImVec2 oldScale = m_Scale;
    
    // 6. 应用新缩放
    m_Scale.x *= scaleFactor;
    m_Scale.y *= scaleFactor;
    
    // 7. 限制缩放范围
    ClampScale();
    
    // 8. 计算实际应用的缩放因子（可能被限制了）
    float actualScaleFactor = m_Scale.x / oldScale.x;
    
    // 9. 缩放后，鼠标点相对于图片中心的向量也会缩放
    //    为了保持鼠标点在屏幕上的位置不变，需要调整图片位置
    //    新的图片中心位置 = 鼠标点 - 缩放后的向量
    ImVec2 newMouseToCenter;
    newMouseToCenter.x = mouseToCenter.x * actualScaleFactor;
    newMouseToCenter.y = mouseToCenter.y * actualScaleFactor;
    
    ImVec2 newImageCenter;
    newImageCenter.x = mouseLogical.x - newMouseToCenter.x;
    newImageCenter.y = mouseLogical.y - newMouseToCenter.y;
    
    // 10. 更新位置偏移
    m_Position.x = newImageCenter.x - canvasLogicalSize.x * 0.5f;
    m_Position.y = newImageCenter.y - canvasLogicalSize.y * 0.5f;
}

void TransformManager::Translate(const ImVec2& delta) {
    m_Position.x += delta.x;
    m_Position.y += delta.y;
}

void TransformManager::ClampScale(float minScale, float maxScale) {
    m_Scale.x = std::max(minScale, std::min(m_Scale.x, maxScale));
    m_Scale.y = std::max(minScale, std::min(m_Scale.y, maxScale));
}






