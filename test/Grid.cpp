// SpatialGrid.cpp
#include "Map.h"
#include <algorithm>

SpatialGrid::SpatialGrid(float cellSize, float worldMinX, float worldMinY,
    float worldMaxX, float worldMaxY)
    : m_cellSize(cellSize)
    , m_minX(worldMinX)
    , m_minY(worldMinY)
    , m_worldWidth(worldMaxX - worldMinX)
    , m_worldHeight(worldMaxY - worldMinY) {

    // 计算网格维度
    m_cellsX = static_cast<int>(std::ceil(m_worldWidth / m_cellSize));
    m_cellsY = static_cast<int>(std::ceil(m_worldHeight / m_cellSize));

    // 初始化所有单元格
    m_cells.resize(m_cellsX * m_cellsY);
    for (int y = 0; y < m_cellsY; ++y) {
        for (int x = 0; x < m_cellsX; ++x) {
            int index = y * m_cellsX + x;
            m_cells[index].x = x;
            m_cells[index].y = y;
        }
    }
}

int SpatialGrid::WorldToGridX(float worldX) const {
    int gridX = static_cast<int>((worldX - m_minX) / m_cellSize);
    return std::clamp(gridX, 0, m_cellsX - 1);
}

int SpatialGrid::WorldToGridY(float worldY) const {
    int gridY = static_cast<int>((worldY - m_minY) / m_cellSize);
    return std::clamp(gridY, 0, m_cellsY - 1);
}

int SpatialGrid::WorldToGridIndex(float worldX, float worldY) const {
    int gridX = WorldToGridX(worldX);
    int gridY = WorldToGridY(worldY);
    return gridY * m_cellsX + gridX;
}

void SpatialGrid::BuildFromMap(Map& map) {
    // 清空所有单元格
    for (auto& cell : m_cells) {
        cell.tiles.clear();
    }

    // 只处理中间层（固体砖块）
    const auto& midgroundTiles = map.GetTiles(MapLayer::MIDGROUND);

    for (const auto& tile : midgroundTiles) {
        if (tile.tileInfo.isSolid) {
            // 计算砖块覆盖的网格范围
            int minX = WorldToGridX(tile.posX);
            int maxX = WorldToGridX(tile.posX + tile.width);
            int minY = WorldToGridY(tile.posY);
            int maxY = WorldToGridY(tile.posY + tile.height);

            // 将砖块添加到它覆盖的所有单元格中
            for (int y = minY; y <= maxY; ++y) {
                for (int x = minX; x <= maxX; ++x) {
                    int index = y * m_cellsX + x;
                    m_cells[index].tiles.push_back(const_cast<MapTile*>(&tile));
                }
            }
        }
    }
}

void SpatialGrid::GetTilesInArea(float x, float y, float width, float height,
    std::vector<MapTile*>& result) const {
    result.clear();

    // 计算查询区域的网格范围
    int minX = WorldToGridX(x);
    int maxX = WorldToGridX(x + width);
    int minY = WorldToGridY(y);
    int maxY = WorldToGridY(y + height);

    // 去重集合
    std::unordered_set<MapTile*> uniqueTiles;

    // 收集覆盖区域内的所有单元格中的砖块
    for (int gridY = minY; gridY <= maxY; ++gridY) {
        for (int gridX = minX; gridX <= maxX; ++gridX) {
            int index = gridY * m_cellsX + gridX;

            for (auto tile : m_cells[index].tiles) {
                if (uniqueTiles.find(tile) == uniqueTiles.end()) {
                    uniqueTiles.insert(tile);
                    result.push_back(tile);
                }
            }
        }
    }
}