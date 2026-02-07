#include "TileImage.h"
#include "utils/Logger.h"
#include <algorithm>
#include <cmath>

// ========== Tile 实现 ==========

Tile::Tile() : pixels(nullptr), refCount(1) {
    pixels = new uint8_t[TILE_PIXEL_COUNT * 4];
    std::memset(pixels, 0, TILE_PIXEL_COUNT * 4);
}

Tile::~Tile() {
    delete[] pixels;
}

Tile* Tile::Clone() const {
    Tile* newTile = TilePool::Instance().Allocate();
    std::memcpy(newTile->pixels, pixels, TILE_PIXEL_COUNT * 4);
    return newTile;
}

void Tile::AddRef() {
    refCount++;
}

void Tile::Release() {
    refCount--;
    if (refCount <= 0) {
        TilePool::Instance().Recycle(this);
    }
}

uint8_t Tile::GetPixel(int x, int y, int channel) const {
    if (x < 0 || x >= TILE_SIZE || y < 0 || y >= TILE_SIZE || channel < 0 || channel >= 4) {
        return 0;
    }
    int index = (y * TILE_SIZE + x) * 4 + channel;
    return pixels[index];
}

void Tile::SetPixel(int x, int y, int channel, uint8_t value) {
    if (x < 0 || x >= TILE_SIZE || y < 0 || y >= TILE_SIZE || channel < 0 || channel >= 4) {
        return;
    }
    int index = (y * TILE_SIZE + x) * 4 + channel;
    pixels[index] = value;
}

void Tile::FillFromImageData(const ImageData& imageData, int tileX, int tileY, 
                              int imageWidth, int imageHeight) {
    int startX = tileX * TILE_SIZE;
    int startY = tileY * TILE_SIZE;
    
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            int imageX = startX + x;
            int imageY = startY + y;
            
            if (imageX >= imageWidth || imageY >= imageHeight) {
                // 超出图像范围，填充透明
                SetPixel(x, y, 0, 0);
                SetPixel(x, y, 1, 0);
                SetPixel(x, y, 2, 0);
                SetPixel(x, y, 3, 0);
            } else {
                // 从图像数据复制
                int imageIndex = (imageY * imageWidth + imageX) * imageData.channels;
                
                if (imageData.channels == 4) {
                    SetPixel(x, y, 0, imageData.pixels[imageIndex + 0]);
                    SetPixel(x, y, 1, imageData.pixels[imageIndex + 1]);
                    SetPixel(x, y, 2, imageData.pixels[imageIndex + 2]);
                    SetPixel(x, y, 3, imageData.pixels[imageIndex + 3]);
                } else if (imageData.channels == 3) {
                    SetPixel(x, y, 0, imageData.pixels[imageIndex + 0]);
                    SetPixel(x, y, 1, imageData.pixels[imageIndex + 1]);
                    SetPixel(x, y, 2, imageData.pixels[imageIndex + 2]);
                    SetPixel(x, y, 3, 255);  // 不透明
                } else if (imageData.channels == 1) {
                    uint8_t gray = imageData.pixels[imageIndex];
                    SetPixel(x, y, 0, gray);
                    SetPixel(x, y, 1, gray);
                    SetPixel(x, y, 2, gray);
                    SetPixel(x, y, 3, 255);
                }
            }
        }
    }
}

// ========== TilePool 实现 ==========

TilePool& TilePool::Instance() {
    static TilePool instance;
    return instance;
}

Tile* TilePool::Allocate() {
    if (!m_FreeTiles.empty()) {
        Tile* tile = m_FreeTiles.back();
        m_FreeTiles.pop_back();
        tile->refCount = 1;
        return tile;
    }
    return new Tile();
}

void TilePool::Recycle(Tile* tile) {
    if (!tile) return;
    
    // 限制池大小（避免占用过多内存）
    const size_t MAX_POOL_SIZE = 1000;
    if (m_FreeTiles.size() < MAX_POOL_SIZE) {
        m_FreeTiles.push_back(tile);
    } else {
        delete tile;
    }
}

void TilePool::Clear() {
    for (Tile* tile : m_FreeTiles) {
        delete tile;
    }
    m_FreeTiles.clear();
}

TilePool::~TilePool() {
    Clear();
}

// ========== TileImage 实现 ==========

TileImage::TileImage() 
    : m_Width(0), m_Height(0), m_TileCountX(0), m_TileCountY(0) {
}

TileImage::~TileImage() {
    Clear();
}

bool TileImage::CreateFromImageData(const ImageData& imageData) {
    if (!imageData.IsValid()) {
        Logger::Error("[TileImage] Invalid ImageData");
        return false;
    }
    
    Clear();
    
    m_Width = imageData.width;
    m_Height = imageData.height;
    
    // 计算需要的 Tile 数量
    m_TileCountX = (m_Width + TILE_SIZE - 1) / TILE_SIZE;
    m_TileCountY = (m_Height + TILE_SIZE - 1) / TILE_SIZE;
    
    int totalTiles = m_TileCountX * m_TileCountY;
    m_Tiles.resize(totalTiles, nullptr);
    
    Logger::Info("[TileImage] Creating TileImage: " + 
                 std::to_string(m_Width) + "x" + std::to_string(m_Height) + 
                 " -> " + std::to_string(m_TileCountX) + "x" + std::to_string(m_TileCountY) + 
                 " tiles (" + std::to_string(totalTiles) + " total)");
    
    // 填充所有 Tile
    for (int tileY = 0; tileY < m_TileCountY; ++tileY) {
        for (int tileX = 0; tileX < m_TileCountX; ++tileX) {
            Tile* tile = TilePool::Instance().Allocate();
            tile->FillFromImageData(imageData, tileX, tileY, m_Width, m_Height);
            
            int index = GetTileIndex(tileX, tileY);
            m_Tiles[index] = tile;
        }
    }
    
    return true;
}

ImageData TileImage::ToImageData() const {
    ImageData result;
    result.width = m_Width;
    result.height = m_Height;
    result.channels = 4;  // RGBA
    result.pixels.resize(m_Width * m_Height * 4);
    
    for (int y = 0; y < m_Height; ++y) {
        for (int x = 0; x < m_Width; ++x) {
            int tileX, tileY;
            PixelToTile(x, y, tileX, tileY);
            
            const Tile* tile = GetTile(tileX, tileY);
            if (!tile) continue;
            
            int localX = x % TILE_SIZE;
            int localY = y % TILE_SIZE;
            
            int imageIndex = (y * m_Width + x) * 4;
            result.pixels[imageIndex + 0] = tile->GetPixel(localX, localY, 0);
            result.pixels[imageIndex + 1] = tile->GetPixel(localX, localY, 1);
            result.pixels[imageIndex + 2] = tile->GetPixel(localX, localY, 2);
            result.pixels[imageIndex + 3] = tile->GetPixel(localX, localY, 3);
        }
    }
    
    return result;
}

uint8_t TileImage::GetPixel(int x, int y, int channel) const {
    if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) {
        return 0;
    }
    
    int tileX, tileY;
    PixelToTile(x, y, tileX, tileY);
    
    const Tile* tile = GetTile(tileX, tileY);
    if (!tile) return 0;
    
    int localX = x % TILE_SIZE;
    int localY = y % TILE_SIZE;
    
    return tile->GetPixel(localX, localY, channel);
}

void TileImage::SetPixel(int x, int y, int channel, uint8_t value) {
    if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) {
        return;
    }
    
    int tileX, tileY;
    PixelToTile(x, y, tileX, tileY);
    
    Tile* tile = GetTileForWrite(tileX, tileY);
    if (!tile) return;
    
    int localX = x % TILE_SIZE;
    int localY = y % TILE_SIZE;
    
    tile->SetPixel(localX, localY, channel, value);
}

const Tile* TileImage::GetTile(int tileX, int tileY) const {
    if (tileX < 0 || tileX >= m_TileCountX || tileY < 0 || tileY >= m_TileCountY) {
        return nullptr;
    }
    
    int index = GetTileIndex(tileX, tileY);
    return m_Tiles[index];
}

Tile* TileImage::GetTileForWrite(int tileX, int tileY) {
    if (tileX < 0 || tileX >= m_TileCountX || tileY < 0 || tileY >= m_TileCountY) {
        return nullptr;
    }
    
    int index = GetTileIndex(tileX, tileY);
    Tile* tile = m_Tiles[index];
    
    if (!tile) return nullptr;
    
    // Copy-on-Write：如果引用计数 > 1，需要克隆
    if (tile->refCount > 1) {
        Tile* newTile = tile->Clone();
        tile->Release();
        m_Tiles[index] = newTile;
        return newTile;
    }
    
    return tile;
}

void TileImage::PixelToTile(int pixelX, int pixelY, int& tileX, int& tileY) const {
    tileX = pixelX / TILE_SIZE;
    tileY = pixelY / TILE_SIZE;
}

void TileImage::GetAffectedTiles(int selectionX, int selectionY, 
                                  int selectionWidth, int selectionHeight,
                                  int& outStartTileX, int& outStartTileY,
                                  int& outEndTileX, int& outEndTileY) const {
    // 计算选区的边界
    int left = std::max(0, selectionX);
    int top = std::max(0, selectionY);
    int right = std::min(m_Width - 1, selectionX + selectionWidth - 1);
    int bottom = std::min(m_Height - 1, selectionY + selectionHeight - 1);
    
    // 转换为 Tile 坐标
    outStartTileX = left / TILE_SIZE;
    outStartTileY = top / TILE_SIZE;
    outEndTileX = right / TILE_SIZE;
    outEndTileY = bottom / TILE_SIZE;
    
    // 边界检查
    outStartTileX = std::max(0, std::min(outStartTileX, m_TileCountX - 1));
    outStartTileY = std::max(0, std::min(outStartTileY, m_TileCountY - 1));
    outEndTileX = std::max(0, std::min(outEndTileX, m_TileCountX - 1));
    outEndTileY = std::max(0, std::min(outEndTileY, m_TileCountY - 1));
}

int TileImage::GetTileIndex(int tileX, int tileY) const {
    return tileY * m_TileCountX + tileX;
}

TileImage* TileImage::Clone() const {
    TileImage* newImage = new TileImage();
    newImage->m_Width = m_Width;
    newImage->m_Height = m_Height;
    newImage->m_TileCountX = m_TileCountX;
    newImage->m_TileCountY = m_TileCountY;
    newImage->m_Tiles.resize(m_Tiles.size());
    
    for (size_t i = 0; i < m_Tiles.size(); ++i) {
        if (m_Tiles[i]) {
            newImage->m_Tiles[i] = m_Tiles[i]->Clone();
        }
    }
    
    return newImage;
}

void TileImage::Clear() {
    for (Tile* tile : m_Tiles) {
        if (tile) {
            tile->Release();
        }
    }
    m_Tiles.clear();
    m_Width = 0;
    m_Height = 0;
    m_TileCountX = 0;
    m_TileCountY = 0;
}

