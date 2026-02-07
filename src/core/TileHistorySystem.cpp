#include "TileHistorySystem.h"
#include "utils/Logger.h"
#include <algorithm>

TileHistorySystem::TileHistorySystem(size_t maxHistorySize)
    : m_CurrentIndex(-1)
    , m_MaxHistorySize(maxHistorySize)
    , m_CurrentTileImage(nullptr)
    , m_CurrentEdit(nullptr) {
    m_Stats = {0.0, 0.0, 0, 0};
}

TileHistorySystem::~TileHistorySystem() {
    Clear();
}

void TileHistorySystem::BeginEdit(TileImage* tileImage, const std::string& description) {
    if (!tileImage || !tileImage->IsValid()) {
        Logger::Error("[TileHistorySystem] BeginEdit: Invalid TileImage");
        return;
    }
    
    if (m_CurrentEdit) {
        Logger::Warning("[TileHistorySystem] BeginEdit: Previous edit not ended, auto-ending");
        EndEdit();
    }
    
    m_CurrentTileImage = tileImage;
    m_CurrentEdit = std::make_unique<TileHistoryState>(description);
    m_ModifiedTileIndices.clear();
    
    Logger::Debug("[TileHistorySystem] BeginEdit: " + description);
}

void TileHistorySystem::RecordTileModification(int tileX, int tileY) {
    if (!m_CurrentEdit || !m_CurrentTileImage) {
        Logger::Error("[TileHistorySystem] RecordTileModification: No active edit");
        return;
    }
    
    int tileIndex = m_CurrentTileImage->GetTileIndex(tileX, tileY);
    
    // 检查是否已经记录过这个 Tile
    if (std::find(m_ModifiedTileIndices.begin(), m_ModifiedTileIndices.end(), tileIndex) 
        != m_ModifiedTileIndices.end()) {
        return;  // 已记录，跳过
    }
    
    // 保存修改前的 Tile
    const Tile* originalTile = m_CurrentTileImage->GetTile(tileX, tileY);
    if (!originalTile) {
        Logger::Error("[TileHistorySystem] RecordTileModification: Tile not found");
        return;
    }
    
    Tile* clonedTile = originalTile->Clone();
    m_CurrentEdit->deltas.emplace_back(tileIndex, clonedTile);
    m_ModifiedTileIndices.push_back(tileIndex);
    
    Logger::Debug("[TileHistorySystem] Recorded Tile modification: (" + 
                  std::to_string(tileX) + ", " + std::to_string(tileY) + ")");
}

void TileHistorySystem::EndEdit() {
    if (!m_CurrentEdit) {
        Logger::Warning("[TileHistorySystem] EndEdit: No active edit");
        return;
    }
    
    // 如果没有修改任何 Tile，不保存历史记录
    if (m_CurrentEdit->deltas.empty()) {
        Logger::Debug("[TileHistorySystem] EndEdit: No tiles modified, discarding");
        m_CurrentEdit.reset();
        m_CurrentTileImage = nullptr;
        m_ModifiedTileIndices.clear();
        return;
    }
    
    // 如果当前不在历史记录末尾，清除后续的重做记录
    if (m_CurrentIndex < static_cast<int>(m_History.size()) - 1) {
        m_History.erase(m_History.begin() + m_CurrentIndex + 1, m_History.end());
    }
    
    // 添加到历史记录
    m_History.push_back(std::move(m_CurrentEdit));
    m_CurrentIndex = static_cast<int>(m_History.size()) - 1;
    
    // 如果超过最大历史记录数，删除最旧的记录
    if (m_History.size() > m_MaxHistorySize) {
        m_History.erase(m_History.begin());
        m_CurrentIndex--;
    }
    
    // 更新统计信息
    size_t tilesModified = m_ModifiedTileIndices.size();
    m_Stats.totalMemoryUsed += tilesModified * TILE_PIXEL_COUNT * 4;
    
    if (m_History.size() > 0) {
        size_t totalTiles = 0;
        for (const auto& state : m_History) {
            totalTiles += state->deltas.size();
        }
        m_Stats.averageTilesPerEdit = totalTiles / m_History.size();
    }
    
    Logger::Info("[TileHistorySystem] EndEdit: Saved " + std::to_string(tilesModified) + 
                 " tiles (index=" + std::to_string(m_CurrentIndex) + 
                 ", total=" + std::to_string(m_History.size()) + ")");
    
    m_CurrentEdit.reset();
    m_CurrentTileImage = nullptr;
    m_ModifiedTileIndices.clear();
}

bool TileHistorySystem::Undo() {
    if (!CanUndo()) {
        Logger::Debug("[TileHistorySystem] Undo: No history available");
        return false;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 获取当前状态
    TileHistoryState* state = m_History[m_CurrentIndex].get();
    
    // 恢复所有修改的 Tile
    // 注意：这里需要外部提供 TileImage 引用
    // 由于架构限制，我们需要在 PreviewPanel 中处理实际的恢复逻辑
    
    m_CurrentIndex--;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_Stats.lastUndoTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    Logger::Info("[TileHistorySystem] Undo: '" + state->description + 
                 "' (time=" + std::to_string(m_Stats.lastUndoTime) + "ms, " +
                 "tiles=" + std::to_string(state->deltas.size()) + ")");
    
    return true;
}

bool TileHistorySystem::Redo() {
    if (!CanRedo()) {
        Logger::Debug("[TileHistorySystem] Redo: No redo available");
        return false;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    m_CurrentIndex++;
    
    TileHistoryState* state = m_History[m_CurrentIndex].get();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_Stats.lastRedoTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    Logger::Info("[TileHistorySystem] Redo: '" + state->description + 
                 "' (time=" + std::to_string(m_Stats.lastRedoTime) + "ms)");
    
    return true;
}

bool TileHistorySystem::CanUndo() const {
    return m_CurrentIndex >= 0;
}

bool TileHistorySystem::CanRedo() const {
    return m_CurrentIndex < static_cast<int>(m_History.size()) - 1;
}

void TileHistorySystem::Clear() {
    m_History.clear();
    m_CurrentIndex = -1;
    m_CurrentEdit.reset();
    m_CurrentTileImage = nullptr;
    m_ModifiedTileIndices.clear();
    m_Stats = {0.0, 0.0, 0, 0};
    
    Logger::Info("[TileHistorySystem] Cleared all history");
}

std::vector<std::string> TileHistorySystem::GetHistoryDescriptions() const {
    std::vector<std::string> descriptions;
    descriptions.reserve(m_History.size());
    
    for (const auto& state : m_History) {
        descriptions.push_back(state->description);
    }
    
    return descriptions;
}

