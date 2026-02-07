#include "TileImageHistory.h"
#include "utils/Logger.h"
#include <chrono>

TileImageHistory::TileImageHistory(size_t maxHistorySize)
    : m_CurrentSnapshotIndex(-1)
    , m_MaxHistorySize(maxHistorySize) {
    m_TileImage = std::make_unique<TileImage>();
    m_HistorySystem = std::make_unique<TileHistorySystem>(maxHistorySize);
}

TileImageHistory::~TileImageHistory() = default;

bool TileImageHistory::CreateFromImageData(const ImageData& imageData) {
    if (!m_TileImage->CreateFromImageData(imageData)) {
        Logger::Error("[TileImageHistory] Failed to create TileImage from ImageData");
        return false;
    }
    
    // 清空历史记录
    m_Snapshots.clear();
    m_CurrentSnapshotIndex = -1;
    m_HistorySystem->Clear();
    
    Logger::Info("[TileImageHistory] Created from ImageData: " + 
                 std::to_string(imageData.width) + "x" + std::to_string(imageData.height));
    
    return true;
}

ImageData TileImageHistory::ToImageData() const {
    return m_TileImage->ToImageData();
}

void TileImageHistory::BeginEdit(const std::string& description) {
    // 保存当前状态快照
    Snapshot snapshot;
    snapshot.image = std::unique_ptr<TileImage>(m_TileImage->Clone());
    snapshot.description = description;
    
    // 如果当前不在历史记录末尾，清除后续的重做记录
    if (m_CurrentSnapshotIndex < static_cast<int>(m_Snapshots.size()) - 1) {
        m_Snapshots.erase(m_Snapshots.begin() + m_CurrentSnapshotIndex + 1, m_Snapshots.end());
    }
    
    // 添加快照
    m_Snapshots.push_back(std::move(snapshot));
    m_CurrentSnapshotIndex = static_cast<int>(m_Snapshots.size()) - 1;
    
    // 限制历史记录数量
    if (m_Snapshots.size() > m_MaxHistorySize) {
        m_Snapshots.erase(m_Snapshots.begin());
        m_CurrentSnapshotIndex--;
    }
    
    Logger::Debug("[TileImageHistory] BeginEdit: " + description + 
                  " (snapshot index=" + std::to_string(m_CurrentSnapshotIndex) + ")");
}

void TileImageHistory::EndEdit() {
    // 在简化版本中，EndEdit 不需要做任何事情
    // 因为我们在 BeginEdit 时已经保存了快照
    Logger::Debug("[TileImageHistory] EndEdit");
}

void TileImageHistory::SetPixel(int x, int y, int channel, uint8_t value) {
    m_TileImage->SetPixel(x, y, channel, value);
}

uint8_t TileImageHistory::GetPixel(int x, int y, int channel) const {
    return m_TileImage->GetPixel(x, y, channel);
}

void TileImageHistory::DeleteSelection(int selectionX, int selectionY, 
                                       int selectionWidth, int selectionHeight) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 计算受影响的 Tile 范围
    int startTileX, startTileY, endTileX, endTileY;
    m_TileImage->GetAffectedTiles(selectionX, selectionY, selectionWidth, selectionHeight,
                                   startTileX, startTileY, endTileX, endTileY);
    
    int tilesAffected = (endTileX - startTileX + 1) * (endTileY - startTileY + 1);
    
    Logger::Info("[TileImageHistory] DeleteSelection: Affecting " + std::to_string(tilesAffected) + 
                 " tiles (from (" + std::to_string(startTileX) + "," + std::to_string(startTileY) + 
                 ") to (" + std::to_string(endTileX) + "," + std::to_string(endTileY) + "))");
    
    // 遍历选区内的所有像素，设置 Alpha 为 0
    int pixelsModified = 0;
    for (int y = selectionY; y < selectionY + selectionHeight; ++y) {
        for (int x = selectionX; x < selectionX + selectionWidth; ++x) {
            if (x >= 0 && x < m_TileImage->GetWidth() && 
                y >= 0 && y < m_TileImage->GetHeight()) {
                m_TileImage->SetPixel(x, y, 3, 0);  // Alpha = 0
                pixelsModified++;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    Logger::Info("[TileImageHistory] DeleteSelection completed: " + 
                 std::to_string(pixelsModified) + " pixels in " + 
                 std::to_string(elapsedMs) + "ms (" + 
                 std::to_string(tilesAffected) + " tiles affected)");
}

bool TileImageHistory::Undo() {
    if (!CanUndo()) {
        Logger::Debug("[TileImageHistory] Undo: No history available");
        return false;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 恢复到上一个快照
    m_CurrentSnapshotIndex--;
    
    if (m_CurrentSnapshotIndex >= 0) {
        m_TileImage = std::unique_ptr<TileImage>(m_Snapshots[m_CurrentSnapshotIndex].image->Clone());
        
        auto endTime = std::chrono::high_resolution_clock::now();
        double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        
        Logger::Info("[TileImageHistory] Undo: '" + m_Snapshots[m_CurrentSnapshotIndex].description + 
                     "' (time=" + std::to_string(elapsedMs) + "ms)");
    } else {
        // 回到初始状态（空图像）
        m_TileImage->Clear();
        Logger::Info("[TileImageHistory] Undo: Back to initial state");
    }
    
    return true;
}

bool TileImageHistory::Redo() {
    if (!CanRedo()) {
        Logger::Debug("[TileImageHistory] Redo: No redo available");
        return false;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    m_CurrentSnapshotIndex++;
    m_TileImage = std::unique_ptr<TileImage>(m_Snapshots[m_CurrentSnapshotIndex].image->Clone());
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    Logger::Info("[TileImageHistory] Redo: '" + m_Snapshots[m_CurrentSnapshotIndex].description + 
                 "' (time=" + std::to_string(elapsedMs) + "ms)");
    
    return true;
}

bool TileImageHistory::CanUndo() const {
    return m_CurrentSnapshotIndex >= 0;
}

bool TileImageHistory::CanRedo() const {
    return m_CurrentSnapshotIndex < static_cast<int>(m_Snapshots.size()) - 1;
}

void TileImageHistory::Clear() {
    m_Snapshots.clear();
    m_CurrentSnapshotIndex = -1;
    m_HistorySystem->Clear();
    Logger::Info("[TileImageHistory] Cleared all history");
}

int TileImageHistory::GetWidth() const {
    return m_TileImage->GetWidth();
}

int TileImageHistory::GetHeight() const {
    return m_TileImage->GetHeight();
}

bool TileImageHistory::IsValid() const {
    return m_TileImage->IsValid();
}

TileHistorySystem::PerformanceStats TileImageHistory::GetPerformanceStats() const {
    return m_HistorySystem->GetPerformanceStats();
}

