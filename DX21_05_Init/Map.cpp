#include "Map.h"
#include "Game.h"
#include "Enemy.h"
#include <cmath>
#include <algorithm>


// Map 类实现
Map::Map(const std::string& name, float gridWidth, float gridHeight)
    : m_name(name), m_gridWidth(gridWidth), m_gridHeight(gridHeight), m_defaultSpawnId(0) {
    InitializeTileDictionary();
}

void Map::InitializeTileDictionary() {
    // 初始化瓦片类型字典
    m_tileDictionary = {
        // 空地
        {"00", {"00", "empty", "none", false, false, false, false}},

        // 地面类型
        {"G1", {"G1", "ground", "grass", true, false, false, false}},
        {"G2", {"G2", "ground", "dirt", true, false, false, false}},
        {"G3", {"G3", "ground", "stone", true, false, false, false}},

        // 墙壁类型
        {"W1", {"W1", "wall", "brick", true, false, false, false}},
        {"W2", {"W2", "wall", "stone", true, false, false, false}},

        // 平台类型
        {"P1", {"P1", "platform", "wood", true, false, false, false}},
        {"P2", {"P2", "platform", "metal", true, false, false, false}},

        // 敌人类型
        {"E1", {"E1", "enemy", "normal", false, false, false, true}},
        {"E2", {"E2", "enemy", "shield", false, false, false, true}},
        {"E3", {"E3", "enemy", "mage", false, false, false, true}},
        {"E4", {"E4", "enemy", "fast", false, false, false, true}},

        // 传送门类型
        {"DF", {"DF", "door", "forest", false, false, true, false}},
        {"DI", {"DI", "door", "ice", false, false, true, false}},
        {"DT", {"DT", "door", "test", false, false, true, false}},

        // 生成点类型
        {"S1", {"S1", "spawn", "default", false, true, false, false}},
        {"S2", {"S2", "spawn", "secondary", false, true, false, false}},

        // 装饰类型（可选）
        {"D1", {"D1", "decoration", "tree", false, false, false, false}},
        {"D2", {"D2", "decoration", "rock", false, false, false, false}}
    };
}

TileInfo Map::ParseTileCode(const std::string& code) {
    auto it = m_tileDictionary.find(code);
    if (it != m_tileDictionary.end()) {
        return it->second;
    }
    // 默认返回空地
    return m_tileDictionary.at("00");
}

void Map::LoadFromGrid(const std::vector<std::vector<std::string>>& grid, MapLayer layer) {
    ClearLayer(layer);

    auto& tiles = (layer == MapLayer::BACKGROUND) ? m_backgroundTiles :
        (layer == MapLayer::MIDGROUND) ? m_midgroundTiles : m_foregroundTiles;

    int gridRows = static_cast<int>(grid.size());
    int gridCols = gridRows > 0 ? static_cast<int>(grid[0].size()) : 0;

    float totalWidth = gridCols * m_gridWidth;
    float totalHeight = gridRows * m_gridHeight;

    float startX = -totalWidth * 0.5f;
    float startY = -totalHeight * 0.5f;

    for (int y = 0; y < gridRows; y++) {
        for (int x = 0; x < gridCols; x++) {
            std::string tileCode = grid[y][x];
            if (tileCode == "00") continue;  // 空地块跳过

            TileInfo tileInfo = ParseTileCode(tileCode);

            // 计算位置
            float tileX = startX + static_cast<float>(x) * m_gridWidth;
            float tileY = startY + static_cast<float>(gridRows - 1 - y) * m_gridHeight;

            // 处理敌人生成点
            if (tileInfo.isEnemy) {
                EnemySpawnInfo spawn;
                spawn.posX = tileX;
                spawn.posY = tileY;
                spawn.enemyType = tileCode;
                spawn.enemySubtype = std::stoi(tileCode.substr(1)); // 提取数字部分
                m_enemySpawns.push_back(spawn);
                continue; // 敌人不添加到瓦片列表
            }

            // 处理生成点
            if (tileInfo.isSpawn) {
                int spawnId = std::stoi(tileCode.substr(1)); // 提取数字部分
                AddSpawnPoint(tileX, tileY, spawnId, "Spawn_" + tileCode);
                continue; // 生成点不添加到瓦片列表
            }

            // 创建瓦片
            MapTile tile;
            tile.posX = tileX;
            tile.posY = tileY;
            tile.width = m_gridWidth;
            tile.height = m_gridHeight;
            tile.tileInfo = tileInfo;
            tile.linkedSpawnId = -1;

            // 处理传送门
            if (tileInfo.isPortal) {
                if (tileInfo.subtype == "forest") {
                    tile.targetMap = "forest";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "ice") {
                    tile.targetMap = "ice";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "test") {
                    tile.targetMap = "test";
                    tile.linkedSpawnId = 1;
                }
            }

            tiles.push_back(tile);
        }
    }
}

void Map::AddTile(float x, float y, const std::string& tileCode, MapLayer layer,
    const std::string& targetMap, int linkedSpawnId) {
    TileInfo tileInfo = ParseTileCode(tileCode);

    // 处理敌人生成
    if (tileInfo.isEnemy) {
        EnemySpawnInfo spawn;
        spawn.posX = x;
        spawn.posY = y;
        spawn.enemyType = tileCode;
        spawn.enemySubtype = std::stoi(tileCode.substr(1));
        m_enemySpawns.push_back(spawn);
        return;
    }

    // 处理生成点
    if (tileInfo.isSpawn) {
        int spawnId = std::stoi(tileCode.substr(1));
        AddSpawnPoint(x, y, spawnId, "Spawn_" + tileCode);
        return;
    }

    // 创建普通瓦片
    MapTile tile;
    tile.posX = x;
    tile.posY = y;
    tile.width = m_gridWidth;
    tile.height = m_gridHeight;
    tile.tileInfo = tileInfo;
    tile.targetMap = targetMap;
    tile.linkedSpawnId = linkedSpawnId;

    switch (layer) {
    case MapLayer::BACKGROUND:
        m_backgroundTiles.push_back(tile);
        break;
    case MapLayer::MIDGROUND:
        m_midgroundTiles.push_back(tile);
        break;
    case MapLayer::FOREGROUND:
        m_foregroundTiles.push_back(tile);
        break;
    }
}

void Map::AddSpawnPoint(float x, float y, int id, const std::string& name) {
    SpawnPoint spawn;
    spawn.posX = x;
    spawn.posY = y;
    spawn.id = id;
    spawn.name = name;
    m_spawnPoints.push_back(spawn);

    if (m_defaultSpawnId == 0) {
        m_defaultSpawnId = id;
    }
}

// 其余函数实现与之前类似，但使用新的TileInfo系统
void Map::ClearLayer(MapLayer layer) {
    switch (layer) {
    case MapLayer::BACKGROUND:
        m_backgroundTiles.clear();
        break;
    case MapLayer::MIDGROUND:
        m_midgroundTiles.clear();
        break;
    case MapLayer::FOREGROUND:
        m_foregroundTiles.clear();
        break;
    }
}

void Map::ClearAll() {
    m_backgroundTiles.clear();
    m_midgroundTiles.clear();
    m_foregroundTiles.clear();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
}

const std::vector<MapTile>& Map::GetTiles(MapLayer layer) const {
    static const std::vector<MapTile> empty;
    switch (layer) {
    case MapLayer::BACKGROUND: return m_backgroundTiles;
    case MapLayer::MIDGROUND: return m_midgroundTiles;
    case MapLayer::FOREGROUND: return m_foregroundTiles;
    default: return empty;
    }
}

std::vector<MapTile>& Map::GetSolidTiles() {
    static std::vector<MapTile> solidTiles;
    solidTiles.clear();

    for (const auto& tile : m_midgroundTiles) {
        if (tile.tileInfo.isSolid) {
            solidTiles.push_back(tile);
        }
    }
    return solidTiles;
}

bool Map::GetSpawnPoint(int spawnId, float& x, float& y) const {
    for (const auto& spawn : m_spawnPoints) {
        if (spawn.id == spawnId) {
            x = spawn.posX;
            y = spawn.posY;
            return true;
        }
    }
    return false;
}

bool Map::GetDefaultSpawnPoint(float& x, float& y) const {
    return GetSpawnPoint(m_defaultSpawnId, x, y);
}

bool Map::CheckPortalCollision(float x, float y, float width, float height,
    std::string& targetMap, int& portalId, int& linkedSpawnId) const {
    for (const auto& tile : m_midgroundTiles) {
        if (tile.tileInfo.isPortal) {
            if (CheckCollision(x, y, width, height,
                tile.posX, tile.posY, tile.width, tile.height)) {
                targetMap = tile.targetMap;
                portalId = tile.linkedSpawnId; // 使用linkedSpawnId作为portalId
                linkedSpawnId = tile.linkedSpawnId;
                return true;
            }
        }
    }
    return false;
}

// 使用新的字符串系统创建测试地图
void Map::CreateTestMap() {
    ClearAll();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
    m_defaultSpawnId = 1;

    std::vector<std::vector<std::string>> midgroundGrid = {
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","E1","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","P1","00","00","00","P1","PI","00","00","00"},
        {"00","00","S1","00","00","00","P1","00","00","00","P1","00","00","00","00"},
        {"00","00","P1","P1","P1","00","00","00","P1","P1","P1","00","00","00","00"},
        {"00","00","P1","P1","P1","00","00","00","P1","P1","P1","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","DI","00","00"},
        {"G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1"},
        {"G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1"}
    };

    LoadFromGrid(midgroundGrid, MapLayer::MIDGROUND);

    // 添加生成点
    AddSpawnPoint(-0.7f, -0.5f, 1, "MainSpawn");

    // 手动添加传送门
    AddTile(0.6f, -0.8f, "PF", MapLayer::MIDGROUND, "forest", 1);
}

void Map::CreateForestMap() {
    ClearAll();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
    m_defaultSpawnId = 1;

    std::vector<std::vector<std::string>> midgroundGrid = {
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","E2","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","P1","P1","P1","00","00","00","00","00","00","00","00"},
        {"00","00","S1","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","P1","P1","P1","00","00","P1","P1","P1","00","E3","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","DT","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1"},
        {"G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1"}
    };

    LoadFromGrid(midgroundGrid, MapLayer::MIDGROUND);

    AddSpawnPoint(-0.8f, -0.8f, 1, "ForestSpawn");
    AddTile(-0.8f, -0.8f, "PT", MapLayer::MIDGROUND, "test", 1);
}

void Map::CreateIceMap() {
    ClearAll();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
    m_defaultSpawnId = 1;

    std::vector<std::vector<std::string>> midgroundGrid = {
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","E4","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","P2","P2","P2","P2","P2","00","00","00","00","00","00"},
        {"00","00","S1","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","P2","P2","P2","00","00","00","P2","P2","P2","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","DF","00","00"},
        {"G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2"},
        {"G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2"}
    };

    LoadFromGrid(midgroundGrid, MapLayer::MIDGROUND);

    AddSpawnPoint(-0.5f, -0.5f, 1, "IceSpawn");
    AddTile(0.7f, -0.8f, "PT", MapLayer::MIDGROUND, "test", 1);
}