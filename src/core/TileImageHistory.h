#pragma once

#include "TileImage.h"
#include "TileHistorySystem.h"
#include "Types.h"
#include <memory>

/**
 * @brief Tile-based 图像历史记录适配器
 * 
 * 职责：
 * - 封装 TileImage 和 TileHistorySystem
 * - 提供与 ImageHistory 兼容的接口
 * - 自动管理 Tile 修改跟踪
 * 
 * 使用方式：
 * 1. 从 ImageData 创建：CreateFromImageData()
 * 2. 修改前调用：BeginEdit()
 * 3. 修改像素：SetPixel() 或 DeleteSelection()
 * 4. 修改后调用：EndEdit()
 * 5. 撤销/重做：Undo() / Redo()
 */
class TileImageHistory {
public:
    TileImageHistory(size_t maxHistorySize = 50);
    ~TileImageHistory();

    /**
     * @brief 从 ImageData 创建
     */
    bool CreateFromImageData(const ImageData& imageData);

    /**
     * @brief 转换为 ImageData
     */
    ImageData ToImageData() const;

    /**
     * @brief 开始编辑
     * @param description 操作描述
     */
    void BeginEdit(const std::string& description);

    /**
     * @brief 结束编辑
     */
    void EndEdit();

    /**
     * @brief 设置像素（自动跟踪 Tile 修改）
     */
    void SetPixel(int x, int y, int channel, uint8_t value);

    /**
     * @brief 获取像素
     */
    uint8_t GetPixel(int x, int y, int channel) const;

    /**
     * @brief 删除选区内容（Tile 级优化）
     * @param selectionX 选区左上角 X
     * @param selectionY 选区左上角 Y
     * @param selectionWidth 选区宽度
     * @param selectionHeight 选区高度
     */
    void DeleteSelection(int selectionX, int selectionY, 
                        int selectionWidth, int selectionHeight);

    /**
     * @brief 撤销
     */
    bool Undo();

    /**
     * @brief 重做
     */
    bool Redo();

    /**
     * @brief 是否可以撤销
     */
    bool CanUndo() const;

    /**
     * @brief 是否可以重做
     */
    bool CanRedo() const;

    /**
     * @brief 清空历史记录
     */
    void Clear();

    /**
     * @brief 获取图像尺寸
     */
    int GetWidth() const;
    int GetHeight() const;

    /**
     * @brief 是否有效
     */
    bool IsValid() const;

    /**
     * @brief 获取性能统计
     */
    TileHistorySystem::PerformanceStats GetPerformanceStats() const;

private:
    std::unique_ptr<TileImage> m_TileImage;
    std::unique_ptr<TileHistorySystem> m_HistorySystem;
    
    // 用于 Undo/Redo 的快照栈
    struct Snapshot {
        std::unique_ptr<TileImage> image;
        std::string description;
    };
    std::vector<Snapshot> m_Snapshots;
    int m_CurrentSnapshotIndex;
    size_t m_MaxHistorySize;
};

