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
        {"E7", {"E7", "enemy", "beam", false, false, false, true}},

        // Portal types
        {"DF", {"DF", "door", "World1Area2", false, false, true, false}},
        {"DI", {"DI", "door", "World1Area3", false, false, true, false}},
        {"DT", {"DT", "door", "World1Area1", false, false, true, false}},
        {"D4", {"D4", "door", "World1Area4", false, false, true, false}},
        {"D5", {"D5", "door", "World1Area5", false, false, true, false}},
        {"D6", {"D6", "door", "World1Area6", false, false, true, false}},
        {"D7", {"D7", "door", "World1Area7", false, false, true, false}},
        {"DB", {"DB", "door", "boss", false, false, true, false}}, // for the boss of world 1

        //Portal Types for World2 (stage2)
        {"21", {"21", "door", "World2Area1", false, false, true, false}},
        {"22", {"22", "door", "World2Area2", false, false, true, false}},
        {"23", {"23", "door", "World2Area3", false, false, true, false}},
        {"24", {"24", "door", "World2Area4", false, false, true, false}},
        {"25", {"25", "door", "World2Area5", false, false, true, false}},
        {"26", {"26", "door", "World2Area6", false, false, true, false}},
        {"27", {"27", "door", "World2Area7", false, false, true, false}},
        //{"B2", {"B2", "door", "boss2", false, false, true, false}}, // for the boss of world 2

        // Portal Types for Word3 (stage3). add them later

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
                if (tileInfo.subtype == "World1Area2") {
                    tile.targetMap = "World1Area2";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "World1Area3") {
                    tile.targetMap = "World1Area3";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "World1Area1") {
                    tile.targetMap = "World1Area1";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "World1Area4") {
                    tile.targetMap = "World1Area4";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "World1Area5") {
                    tile.targetMap = "World1Area5";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "World1Area6") {
                    tile.targetMap = "World1Area6";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "World1Area7") {
                    tile.targetMap = "World1Area7";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "boss") {
                    tile.targetMap = "boss";
                    tile.linkedSpawnId = 1;
                }
                // FOR WORLD 2
                else if (tileInfo.subtype == "World2Area1") {
                    tile.targetMap = "World2Area1";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "World2Area2") {
                    tile.targetMap = "World2Area2";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "World2Area3") {
                    tile.targetMap = "World2Area3";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "World2Area4") {
                    tile.targetMap = "World2Area4";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "World2Area5") {
                    tile.targetMap = "World2Area5";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "World2Area6") {
                    tile.targetMap = "World2Area6";
                    tile.linkedSpawnId = 1;
                }
                else if (tileInfo.subtype == "World2Area7") {
                    tile.targetMap = "World2Area7";
                    tile.linkedSpawnId = 1;
                }

                // FOR WORLD 3 ADD LATER
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