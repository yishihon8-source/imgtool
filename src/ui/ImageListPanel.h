#pragma once

#include "core/Types.h"
#include <vector>
#include <functional>
#include <map>

/**
 * @brief 图片列表面板
 * 
 * 职责：
 * - 显示已导入的图片列表
 * - 显示缩略图和基本信息
 * - 支持选择切换
 */
class ImageListPanel {
public:
    ImageListPanel() = default;
    ~ImageListPanel();

    /**
     * @brief 渲染面板
     * @param imageList 图片列表
     * @param currentIndex 当前选中的索引（输入输出）
     * @param onAddFiles 添加文件的回调函数
     * @param onOpenFolder 打开文件夹的回调函数
     * @param onClearAll 清空所有素材的回调函数
     */
    void Render(std::vector<ImageInfo>& imageList, int& currentIndex, 
                std::function<void()> onAddFiles = nullptr,
                std::function<void()> onOpenFolder = nullptr,
                std::function<void()> onClearAll = nullptr);

private:
    /**
     * @brief 渲染单个图片项
     * @return 是否被点击
     */
    bool RenderImageItem(const ImageInfo& info, int index, bool isSelected);

    /**
     * @brief 格式化文件大小
     */
    std::string FormatFileSize(size_t bytes);
    
    /**
     * @brief 获取或创建缩略图纹理
     */
    unsigned int GetThumbnailTexture(const std::string& filePath);
    
    /**
     * @brief 清理所有缩略图纹理
     */
    void ClearThumbnails();

private:
    // 缩略图缓存 <文件路径, 纹理ID>
    std::map<std::string, unsigned int> m_ThumbnailCache;
    
    // 重命名状态
    int m_RenamingIndex = -1;
    char m_RenameBuffer[256] = {0};
    bool m_DoubleClickTriggered = false;  // 标记是否由双击触发
    bool m_RightClickTriggered = false;   // 标记是否由右键触发
    bool m_OpenRenameDialog = false;      // 标记需要打开重命名对话框
    bool m_OpenDeleteDialog = false;      // 标记需要打开删除对话框
    
    // 拖拽状态
    int m_DraggedIndex = -1;
    
    // 双击检测
    int m_LastClickedIndex = -1;
    float m_LastClickTime = 0.0f;
};
