#include "Map.h"
#include "Game.h"
#include "Enemy.h"
#include <cmath>
#include <algorithm>

// Map class implementation
Map::Map(const std::string& name, float gridWidth, float gridHeight)
    : m_name(name), m_gridWidth(gridWidth), m_gridHeight(gridHeight), m_defaultSpawnId(0), m_spatialGrid(nullptr) {
    InitializeTileDictionary();
}

// Initialize the tile dictionary with all available tile types
void Map::InitializeTileDictionary() {
    // Initialize tile type dictionary
    m_tileDictionary = {
        // Empty space
        {"00", {"00", "empty", "none", false, false, false, false}},

        // Ground types
        {"G1", {"G1", "ground", "grass", true, false, false, false}},
        {"G2", {"G2", "ground", "dirt", true, false, false, false}},
        {"G3", {"G3", "ground", "stone", true, false, false, false}},

        // Wall types
        {"W1", {"W1", "wall", "brick", true, false, false, false}},
        {"W2", {"W2", "wall", "stone", true, false, false, false}},

        // Platform types
        {"P1", {"P1", "platform", "wood", true, false, false, false}},
        {"P2", {"P2", "platform", "metal", true, false, false, false}},
        {"OP", {"OP", "platform", "one_way", true, false, false, false}},  // 单向平台

        // Enemy types
        {"E1", {"E1", "enemy", "normal", false, false, false, true}},
        {"E2", {"E2", "enemy", "shield", false, false, false, true}},
        {"E3", {"E3", "enemy", "mage", false, false, false, true}},
        {"E4", {"E4", "enemy", "fast", false, false, false, true}},
        {"E5", {"E5", "enemy", "bombdd", false, false, false, true}},
        {"E6", {"E6", "enemy", "square", false, false, false, true}},

        // Portal types
        {"DF", {"DF", "door", "forest", false, false, true, false}},
        {"DI", {"DI", "door", "ice", false, false, true, false}},
        {"DT", {"DT", "door", "test", false, false, true, false}},

        // Spawn point types
        {"S1", {"S1", "spawn", "default", false, true, false, false}},
        {"S2", {"S2", "spawn", "secondary", false, true, false, false}},

        // Decoration types (optional)
        {"D1", {"D1", "decoration", "tree", false, false, false, false}},
        {"D2", {"D2", "decoration", "rock", false, false, false, false}}
    };
}

// Convert tile code string to TileInfo structure
TileInfo Map::ParseTileCode(const std::string& code) {
    auto it = m_tileDictionary.find(code);
    if (it != m_tileDictionary.end()) {
        return it->second;
    }
    // Return empty tile by default
    return m_tileDictionary.at("00");
}

// Load map data from a 2D grid of tile codes
void Map::LoadFromGrid(const std::vector<std::vector<std::string>>& grid, MapLayer layer) {
    ClearLayer(layer);

    auto& tiles = (layer == MapLayer::BACKGROUND) ? m_backgroundTiles :
        (layer == MapLayer::MIDGROUND) ? m_midgroundTiles : m_foregroundTiles;

    int gridRows = static_cast<int>(grid.size());
    int gridCols = gridRows > 0 ? static_cast<int>(grid[0].size()) : 0;

    // Calculate total map dimensions
    float totalWidth = gridCols * m_gridWidth;
    float totalHeight = gridRows * m_gridHeight;

    // Calculate starting position (centered at origin)
    float startX = -totalWidth * 0.5f;
    float startY = -totalHeight * 0.5f;

    // Process each cell in the grid
    for (int y = 0; y < gridRows; y++) {
        for (int x = 0; x < gridCols; x++) {
            std::string tileCode = grid[y][x];
            if (tileCode == "00") continue;  // Skip empty tiles

            TileInfo tileInfo = ParseTileCode(tileCode);

            // Calculate tile position
            float tileX = startX + static_cast<float>(x) * m_gridWidth;
            float tileY = startY + static_cast<float>(gridRows - 1 - y) * m_gridHeight;

            // Handle enemy spawn points
            if (tileInfo.isEnemy) {
                EnemySpawnInfo spawn;
                spawn.posX = tileX;
                spawn.posY = tileY;
                spawn.enemyType = tileCode;
                spawn.enemySubtype = std::stoi(tileCode.substr(1)); // Extract numeric part
                m_enemySpawns.push_back(spawn);
                continue; // Don't add enemies to tile list
            }

            // Handle player spawn points
            if (tileInfo.isSpawn) {
                int spawnId = std::stoi(tileCode.substr(1)); // Extract numeric part
                AddSpawnPoint(tileX, tileY, spawnId, "Spawn_" + tileCode);
                continue; // Don't add spawn points to tile list
            }

            // Create regular tile
            MapTile tile;
            tile.posX = tileX;
            tile.posY = tileY;
            tile.width = m_gridWidth;
            tile.height = m_gridHeight;
            tile.tileInfo = tileInfo;
            tile.linkedSpawnId = -1;

            // Handle portal tiles
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

// Add a single tile to the map at specified position
void Map::AddTile(float x, float y, const std::string& tileCode, MapLayer layer,
    const std::string& targetMap, int linkedSpawnId) {
    TileInfo tileInfo = ParseTileCode(tileCode);

    // Handle enemy spawn points
    if (tileInfo.isEnemy) {
        EnemySpawnInfo spawn;
        spawn.posX = x;
        spawn.posY = y;
        spawn.enemyType = tileCode;
        spawn.enemySubtype = std::stoi(tileCode.substr(1));
        m_enemySpawns.push_back(spawn);
        return;
    }

    // Handle player spawn points
    if (tileInfo.isSpawn) {
        int spawnId = std::stoi(tileCode.substr(1));
        AddSpawnPoint(x, y, spawnId, "Spawn_" + tileCode);
        return;
    }

    // Create regular tile
    MapTile tile;
    tile.posX = x;
    tile.posY = y;
    tile.width = m_gridWidth;
    tile.height = m_gridHeight;
    tile.tileInfo = tileInfo;
    tile.targetMap = targetMap;
    tile.linkedSpawnId = linkedSpawnId;

    // Add to appropriate layer
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

// Add a player spawn point to the map
void Map::AddSpawnPoint(float x, float y, int id, const std::string& name) {
    SpawnPoint spawn;
    spawn.posX = x;
    spawn.posY = y;
    spawn.id = id;
    spawn.name = name;
    m_spawnPoints.push_back(spawn);

    // Set as default spawn if this is the first one
    if (m_defaultSpawnId == 0) {
        m_defaultSpawnId = id;
    }
}

// Clear all tiles from a specific layer
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

// Clear all map data including tiles, spawn points, and enemy spawns
void Map::ClearAll() {
    m_backgroundTiles.clear();
    m_midgroundTiles.clear();
    m_foregroundTiles.clear();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
}

// Get tiles from a specific layer
const std::vector<MapTile>& Map::GetTiles(MapLayer layer) const {
    static const std::vector<MapTile> empty;
    switch (layer) {
    case MapLayer::BACKGROUND: return m_backgroundTiles;
    case MapLayer::MIDGROUND: return m_midgroundTiles;
    case MapLayer::FOREGROUND: return m_foregroundTiles;
    default: return empty;
    }
}

// Get all solid tiles (collidable tiles from midground layer)
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

// Get spawn point coordinates by ID
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

// Get the default spawn point coordinates
bool Map::GetDefaultSpawnPoint(float& x, float& y) const {
    return GetSpawnPoint(m_defaultSpawnId, x, y);
}


// 同样可以优化传送门检测
bool Map::CheckPortalCollision(float x, float y, float width, float height,
    std::string& targetMap, int& portalId, int& linkedSpawnId) const {

    // 回退到原始方法
    for (const auto& tile : m_midgroundTiles) {
        if (tile.tileInfo.isPortal) {
            if (CheckCollision(x, y, width, height,
                tile.posX, tile.posY, tile.width, tile.height)) {
                targetMap = tile.targetMap;
                portalId = tile.linkedSpawnId; // Use linkedSpawnId as portalId
                linkedSpawnId = tile.linkedSpawnId;
                return true;
            }
        }
    }

    return false;
}

// Create a test map with basic layout
void Map::CreateTestMap() {
    ClearAll();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
    m_defaultSpawnId = 1;

    // Define map layout using tile codes
    std::vector<std::vector<std::string>> midgroundGrid = {
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","E6","00","00","00","00","00","00","00","00","G1","G1","00","00","00","00","00","G1","G1","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","OP","OP","OP","00","00","G1","G1","00","00","00","00","00","G1","G1","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G1","00","00","00","00","00","G1","G1","00","00","00","00"},
        {"00","00","00","00","G1","G1","00","00","00","00","00","00","00","G1","G1","00","00","00","00","00","G1","G1","00","00","00","00"},
        {"00","S1","00","00","G1","G1","00","00","00","00","00","00","00","G1","G1","00","00","00","00","00","G1","G1","00","00","DF","00"},
        {"G1","G1","G1","G1","G1","G1","G1","G1","OP","OP","G1","G1","G1","G1","G1","00","00","00","00","00","G1","G1","G1","G1","G1","G1"},
        {"G1","G1","G1","G1","G1","G1","G1","G1","00","00","G1","G1","G1","G1","G1","00","00","00","00","00","G1","G1","G1","G1","G1","G1"}
    };

    LoadFromGrid(midgroundGrid, MapLayer::MIDGROUND);
}

// Create a forest-themed map
void Map::CreateForestMap() {
    ClearAll();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
    m_defaultSpawnId = 1;

    std::vector<std::vector<std::string>> midgroundGrid = {
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","G1","00","00","00","00","00","00","00","00","00","00","00","00","G1","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","G1","00","00","00","00","00","00","00","00","00","00","00","DI","G1","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","G1","00","00","00","00","00","00","00","E5","00","00","G1","G1","G1","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","G1","00","00","00","00","00","00","00","00","00","00","G1","G1","G1","00"},
        {"G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","00","00","00","00","00","00","00","00","00","00","00","00","G1","00"},
        {"G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","00","00","00","00","00","E5","00","00","00","00","00","00","G1","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","00"},
        {"00","00","00","00","E5","00","00","E5","00","00","E5","00","00","00","00","00","00","00","00","00","00","00","G1","G1","00","00","G1","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","E5","00","00","00","00","G1","G1","00","00","G1","00"},
        {"00","S1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","00"},
        {"G1","G1","G1","00","00","00","00","00","00","00","00","00","G1","G1","00","00","00","00","00","00","00","00","00","00","00","00","G1","00"},
        {"G1","G1","G1","00","00","00","00","00","00","00","00","00","G1","G1","00","00","00","00","00","00","00","00","00","00","G1","G1","G1","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","G1","00","00","00","00","00","00","00","00","00","00","G1","G1","G1","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","G1","00","00","00","00","00","00","00","00","00","G1","G1","00","G1","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","G1","00","00","00","00","00","00","00","00","00","G1","G1","00","G1","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","00"},
    };

    LoadFromGrid(midgroundGrid, MapLayer::MIDGROUND);
}

// Create an ice-themed map
void Map::CreateIceMap() {
    ClearAll();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
    m_defaultSpawnId = 1;

    std::vector<std::vector<std::string>> massiveTestGrid = {
        // 行 0-2: 天空区域，有飞行敌人和装饰
        {"00","00","00","00","D1","00","00","00","D1","00","00","00","00","00","00","00","00","D1","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","E5","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","D1","00","00","00","00","00","00","00","00","00","00","00","00","D1","00","00","00","00","00","00","00","00","D1","00","00","00","00","00"},

        // 行 3-5: 上层平台区域，有多个平台和敌人
        {"00","00","00","00","P1","P1","P1","00","00","00","P2","P2","P2","00","00","00","P1","P1","P1","00","00","00","P2","P2","P2","00","00","00","00","00"},
        {"00","E5","00","00","00","00","00","00","00","00","00","00","00","00","E2","00","00","00","00","00","00","00","00","00","00","00","E4","00","00","00"},
        {"00","00","00","P2","P2","P2","00","00","00","00","00","P1","P1","P1","00","00","00","00","00","00","00","00","00","P1","P1","P1","00","00","00","00"},

        // 行 6-8: 中层地形，有不同地面类型和障碍物
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","W1","W1","W1"},
        {"00","S1","00","W2","00","00","00","W2","00","00","00","W2","00","E3","00","W2","00","00","00","W2","00","00","00","W2","00","E2","00","G1","G1","G1"},
        {"00","00","00","W2","00","00","00","W2","00","00","00","W2","00","00","00","W2","00","00","00","W2","00","00","00","W2","00","00","00","G1","G1","G1"},

        // 行 9-11: 主要地面层，有复杂的地形变化
        {"P1","P1","P1","W2","P1","P1","P1","W2","P1","P1","P1","W2","P1","P1","P1","W2","P1","P1","P1","W2","P1","P1","P1","W2","P1","P1","P1","G2","G2","G2"},
        {"00","00","00","W2","00","D2","00","W2","00","D2","00","W2","00","D2","00","W2","00","D2","00","W2","00","D2","00","W2","00","D2","00","G2","G2","G2"},
        {"00","00","00","W2","00","00","00","W2","00","00","00","W2","00","00","00","W2","00","00","00","W2","00","00","00","W2","00","00","00","G2","G2","G2"},

        // 行 12-14: 下层区域，有水域和特殊地形
        {"G3","G3","G3","W2","G3","G3","G3","W2","G3","G3","G3","W2","G3","G3","G3","W2","G3","G3","G3","W2","G3","G3","G3","W2","G3","G3","G3","G3","G3","G3"},
        {"G3","G3","G3","00","G3","G3","G3","00","G3","G3","G3","00","G3","G3","G3","00","G3","G3","G3","00","G3","G3","G3","00","G3","G3","G3","G3","G3","G3"},
        {"00","00","00","00","00","E4","00","00","00","00","00","00","00","E2","00","00","00","00","00","00","00","E1","00","00","00","00","00","00","00","00"},

        // 行 15-17: 地下洞穴区域
        {"00","00","00","00","P2","P2","P2","00","00","00","00","00","P1","P1","P1","00","00","00","00","00","P2","P2","P2","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"W1","W1","W1","00","00","00","00","00","00","00","W2","W2","W2","00","00","00","00","00","00","00","00","00","00","00","00","00","W1","W1","W1","W1"},

        // 行 18-19: 最终区域，包含所有传送门和第二个生成点
        {"G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2"},
        {"G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2"},

        // 行 20-22: 额外扩展区域
        {"00","00","00","00","00","00","00","00","00","00","00","00","S2","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","P1","P1","P1","00","00","00","P2","P2","P2","00","00","00","P1","P1","P1","00","00","00","P2","P2","P2","00","00","00","00","00","00"},
        {"00","E2","00","00","00","00","00","E1","00","00","00","00","00","E4","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},

        // 行 23-25: 终点区域，包含所有类型的门
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"00","00","00","00","00","00","00","00","00","00","00","00","DF","00","DI","00","DT","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1"}
    };

    LoadFromGrid(massiveTestGrid, MapLayer::MIDGROUND);

    AddSpawnPoint(-0.5f, -0.5f, 1, "IceSpawn");
    AddTile(0.7f, -0.8f, "PT", MapLayer::MIDGROUND, "test", 1);
}

// Create a boss map
void Map::CreateBossMap() {
    ClearAll();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
    m_defaultSpawnId = 1;
   
    std::vector<std::vector<std::string>> midgroundGrid = {
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00"},
        {"G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1"},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1"},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1"},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1"},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1"},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1"},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1"},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1"},
        {"G1","00","00","S1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","E5","00","00","00","00","00","00","00","00","G1"},
        {"G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1"},
    };

    LoadFromGrid(midgroundGrid, MapLayer::MIDGROUND);
}

// Create a cake map
void Map::CreateCakeMap() {
    ClearAll();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
    m_defaultSpawnId = 1;

    std::vector<std::vector<std::string>> midgroundGrid = {
        {"00","00","00","00","00","00","00","00","00","00","00","00","00","00",},
        {"G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1",},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","G1",},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","G1",},
        {"G1","00","00","00","00","00","00","00","00","00","E1","00","00","G1",},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","G1",},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","G1",},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","G1",},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","G1",},
        {"G1","00","S1","00","00","00","00","00","00","00","00","00","00","G1",},
        {"G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1",},
    };

    LoadFromGrid(midgroundGrid, MapLayer::MIDGROUND);
}

void Map::BuildSpatialGrid(float cellSize) {
    // 计算地图边界
    float minX = 0, minY = 0, maxX = 0, maxY = 0;
    bool first = true;

    for (const auto& tile : m_midgroundTiles) {
        if (first) {
            minX = tile.posX;
            minY = tile.posY;
            maxX = tile.posX + tile.width;
            maxY = tile.posY + tile.height;
            first = false;
        }
        else {
            minX = std::min(minX, tile.posX);
            minY = std::min(minY, tile.posY);
            maxX = std::max(maxX, tile.posX + tile.width);
            maxY = std::max(maxY, tile.posY + tile.height);
        }
    }

    // 扩展一些边界
    minX -= 5.0f;
    minY -= 5.0f;
    maxX += 5.0f;
    maxY += 5.0f;

    // 创建或重新构建空间网格
    if (m_spatialGrid) {
        delete m_spatialGrid;
    }
    m_spatialGrid = new SpatialGrid(cellSize, minX, minY, maxX, maxY);
    m_spatialGrid->BuildFromMap(*this);
}

//GetCell implementation
const GridCell& SpatialGrid::GetCell(int x, int y) const {
    if (x < 0 || x >= m_cellsX || y < 0 || y >= m_cellsY) {
        static GridCell emptyCell;
        return emptyCell;
    }
    int index = y * m_cellsX + x;
    return m_cells[index];
}

//Rebuild implementation
void SpatialGrid::Rebuild(Map& map) {
    // 清空现有单元格
    for (auto& cell : m_cells) {
        cell.tiles.clear();
    }
    // 重新从地图构建网格
    BuildFromMap(map);
}

// 单向平台碰撞检测
bool Map::CheckOneWayPlatformCollision(float x, float y, float width, float height,
    const MapTile& platform, float& penetrationY) const {
    // 检测基本AABB碰撞
    if (x + width <= platform.posX || x >= platform.posX + platform.width ||
        y + height <= platform.posY || y >= platform.posY + platform.height) {
        return false;
    }

    // 计算各边的穿透深度
    float leftPenetration = (x + width) - platform.posX;
    float rightPenetration = (platform.posX + platform.width) - x;
    float topPenetration = (y + height) - platform.posY;  // 玩家底部到平台顶部的距离
    float bottomPenetration = (platform.posY + platform.height) - y;

    // 对于单向平台，只有从上方碰撞才有效
    // 当玩家的底部在平台顶部附近，并且玩家正在下落时，才视为有效碰撞
    if (topPenetration > 0 && topPenetration < 0.1f) {  // 设置一个小的容差范围
        penetrationY = -topPenetration;  // 负值表示向上调整
        return true;
    }

    return false;
}

// 获取所有单向平台
std::vector<MapTile> Map::GetOneWayPlatforms() const {
    std::vector<MapTile> oneWayPlatforms;

    for (const auto& tile : m_midgroundTiles) {
        if (tile.tileInfo.type == "platform" && tile.tileInfo.subtype == "one_way") {
            oneWayPlatforms.push_back(tile);
        }
    }

    return oneWayPlatforms;
}