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

    // グリッドの寸法を計算する
    m_cellsX = static_cast<int>(std::ceil(m_worldWidth / m_cellSize));
    m_cellsY = static_cast<int>(std::ceil(m_worldHeight / m_cellSize));

    // すべてのセルを初期化する
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
    // すべてのセルを空にする
    for (auto& cell : m_cells) {
        cell.tiles.clear();
    }

    // 中間レイヤー（固体タイル）のみ処理する
    const auto& midgroundTiles = map.GetTiles(MapLayer::MIDGROUND);

    for (const auto& tile : midgroundTiles) {
        if (tile.tileInfo.isSolid) {
            // タイルが覆うグリッド範囲を計算する
            int minX = WorldToGridX(tile.posX);
            int maxX = WorldToGridX(tile.posX + tile.width);
            int minY = WorldToGridY(tile.posY);
            int maxY = WorldToGridY(tile.posY + tile.height);

            // タイルを覆っているすべてのセルに追加する
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

    // 問い合わせ範囲のグリッド領域を計算する
    int minX = WorldToGridX(x);
    int maxX = WorldToGridX(x + width);
    int minY = WorldToGridY(y);
    int maxY = WorldToGridY(y + height);

    // 重複排除用の集合
    std::unordered_set<MapTile*> uniqueTiles;

    // 対象領域内のすべてのセルからタイルを集める
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