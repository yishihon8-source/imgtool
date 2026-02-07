#pragma once

#include "Types.h"
#include <vector>
#include <memory>
#include <cstdint>
#include <cstring>

/**
 * @file TileImage.h
 * @brief Photoshop 级 Tile-based 图像系统
 * 
 * 性能目标：
 * - 支持 8K 图像 + 1000+ 历史记录无卡顿
 * - 内存降低 90%+
 * - Undo/Redo 性能提升 1000 倍
 */

// Tile 大小配置（推荐 64x64 或 128x128）
constexpr int TILE_SIZE = 64;
constexpr int TILE_PIXEL_COUNT = TILE_SIZE * TILE_SIZE;

/**
 * @brief Tile 结构（64x64 像素块）
 * 
 * 使用引用计数实现 Copy-on-Write 优化
 */
struct Tile {
    uint8_t* pixels;  // RGBA 数据（TILE_SIZE * TILE_SIZE * 4 bytes）
    int refCount;     // 引用计数（Copy-on-Write）
    
    Tile();
    ~Tile();
    
    // 禁止拷贝（使用 Clone 显式复制）
    Tile(const Tile&) = delete;
    Tile& operator=(const Tile&) = delete;
    
    /**
     * @brief 克隆 Tile（深拷贝）
     */
    Tile* Clone() const;
    
    /**
     * @brief 增加引用计数
     */
    void AddRef();
    
    /**
     * @brief 减少引用计数（为 0 时自动释放）
     */
    void Release();
    
    /**
     * @brief 获取像素（相对于 Tile 的坐标）
     * @param x 0 到 TILE_SIZE-1
     * @param y 0 到 TILE_SIZE-1
     * @param channel 0=R, 1=G, 2=B, 3=A
     */
    uint8_t GetPixel(int x, int y, int channel) const;
    
    /**
     * @brief 设置像素
     */
    void SetPixel(int x, int y, int channel, uint8_t value);
    
    /**
     * @brief 从 ImageData 填充 Tile
     * @param imageData 源图像
     * @param tileX Tile 在图像中的 X 索引
     * @param tileY Tile 在图像中的 Y 索引
     * @param imageWidth 图像宽度
     * @param imageHeight 图像高度
     */
    void FillFromImageData(const ImageData& imageData, int tileX, int tileY, 
                           int imageWidth, int imageHeight);
};

/**
 * @brief Tile Pool（避免频繁 new/delete）
 */
class TilePool {
public:
    static TilePool& Instance();
    
    /**
     * @brief 分配一个 Tile
     */
    Tile* Allocate();
    
    /**
     * @brief 回收一个 Tile
     */
    void Recycle(Tile* tile);
    
    /**
     * @brief 清空池（释放所有缓存）
     */
    void Clear();
    
    /**
     * @brief 获取池中缓存的 Tile 数量
     */
    size_t GetPoolSize() const { return m_FreeTiles.size(); }

private:
    TilePool() = default;
    ~TilePool();
    
    std::vector<Tile*> m_FreeTiles;
};

/**
 * @brief Tile-based 图像表示
 * 
 * 将图像分割为 64x64 的小块，每个块独立存储
 */
class TileImage {
public:
    TileImage();
    ~TileImage();
    
    /**
     * @brief 从 ImageData 创建 TileImage
     */
    bool CreateFromImageData(const ImageData& imageData);
    
    /**
     * @brief 转换为 ImageData
     */
    ImageData ToImageData() const;
    
    /**
     * @brief 获取像素（图像坐标）
     */
    uint8_t GetPixel(int x, int y, int channel) const;
    
    /**
     * @brief 设置像素（图像坐标）
     * 
     * 注意：会触发 Copy-on-Write
     */
    void SetPixel(int x, int y, int channel, uint8_t value);
    
    /**
     * @brief 获取 Tile（只读）
     */
    const Tile* GetTile(int tileX, int tileY) const;
    
    /**
     * @brief 获取 Tile（可写，触发 Copy-on-Write）
     */
    Tile* GetTileForWrite(int tileX, int tileY);
    
    /**
     * @brief 从像素坐标计算 Tile 索引
     */
    void PixelToTile(int pixelX, int pixelY, int& tileX, int& tileY) const;
    
    /**
     * @brief 计算选区影响的 Tile 范围
     * @param selectionX 选区左上角 X
     * @param selectionY 选区左上角 Y
     * @param selectionWidth 选区宽度
     * @param selectionHeight 选区高度
     * @param outStartTileX 输出：起始 Tile X
     * @param outStartTileY 输出：起始 Tile Y
     * @param outEndTileX 输出：结束 Tile X（包含）
     * @param outEndTileY 输出：结束 Tile Y（包含）
     */
    void GetAffectedTiles(int selectionX, int selectionY, 
                          int selectionWidth, int selectionHeight,
                          int& outStartTileX, int& outStartTileY,
                          int& outEndTileX, int& outEndTileY) const;
    
    /**
     * @brief 获取 Tile 索引（一维）
     */
    int GetTileIndex(int tileX, int tileY) const;
    
    /**
     * @brief 克隆 TileImage（深拷贝）
     */
    TileImage* Clone() const;
    
    /**
     * @brief 清空
     */
    void Clear();
    
    // Getters
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    int GetTileCountX() const { return m_TileCountX; }
    int GetTileCountY() const { return m_TileCountY; }
    bool IsValid() const { return m_Width > 0 && m_Height > 0 && !m_Tiles.empty(); }

private:
    int m_Width;
    int m_Height;
    int m_TileCountX;
    int m_TileCountY;
    std::vector<Tile*> m_Tiles;  // Tile 数组（tileCountX * tileCountY）
};

/**
 * @brief Tile Delta（历史记录中的增量）
 * 
 * 只保存修改的 Tile，而不是整个图像
 */
struct TileDelta {
    int tileIndex;           // Tile 索引
    Tile* previousTile;      // 修改前的 Tile（用于 Undo）
    
    TileDelta() : tileIndex(-1), previousTile(nullptr) {}
    TileDelta(int index, Tile* tile) : tileIndex(index), previousTile(tile) {}
    
    ~TileDelta() {
        if (previousTile) {
            previousTile->Release();
        }
    }
    
    // 禁止拷贝（使用移动语义）
    TileDelta(const TileDelta&) = delete;
    TileDelta& operator=(const TileDelta&) = delete;
    
    // 移动构造
    TileDelta(TileDelta&& other) noexcept
        : tileIndex(other.tileIndex), previousTile(other.previousTile) {
        other.tileIndex = -1;
        other.previousTile = nullptr;
    }
    
    TileDelta& operator=(TileDelta&& other) noexcept {
        if (this != &other) {
            if (previousTile) {
                previousTile->Release();
            }
            tileIndex = other.tileIndex;
            previousTile = other.previousTile;
            other.tileIndex = -1;
            other.previousTile = nullptr;
        }
        return *this;
    }
};

/**
 * @brief 历史记录状态（Tile 级）
 */
struct TileHistoryState {
    std::vector<TileDelta> deltas;  // 修改的 Tile 列表
    std::string description;        // 操作描述
    
    TileHistoryState() = default;
    TileHistoryState(const std::string& desc) : description(desc) {}
    
    // 禁止拷贝
    TileHistoryState(const TileHistoryState&) = delete;
    TileHistoryState& operator=(const TileHistoryState&) = delete;
    
    // 移动构造
    TileHistoryState(TileHistoryState&& other) noexcept
        : deltas(std::move(other.deltas)), description(std::move(other.description)) {}
    
    TileHistoryState& operator=(TileHistoryState&& other) noexcept {
        if (this != &other) {
            deltas = std::move(other.deltas);
            description = std::move(other.description);
        }
        return *this;
    }
};

