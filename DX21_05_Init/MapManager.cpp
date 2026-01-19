#include "Map.h"
#include "Game.h"
#include "Enemy.h"


// Constructor: initialize map manager with null pointers and default values
MapManager::MapManager() : m_currentMap(nullptr), m_previousMap(nullptr),
m_currentPortalId(0), m_enteredSpawnId(-1) {
}

// Add a new map to the map collection
void MapManager::AddMap(const Map& map) {
    m_maps.push_back(map);
}

// Switch to a different map with optional portal and spawn point parameters
bool MapManager::SwitchMap(const std::string& mapName, int enterPortalId, int spawnId) {
    Map* newMap = GetMap(mapName);
    if (!newMap) return false;

    m_previousMap = m_currentMap;
    m_currentMap = newMap;
    m_currentPortalId = enterPortalId;

    // Respawn player in the new map
    RespawnPlayer(spawnId);

    // Create enemies for the new map
    CreateMapEnemies();

    return true;
}
// Map.cpp
void MapManager::CreateMapEnemies() {
    if (!m_currentMap) return;

    // Clear existing enemies before creating new ones
    CleanupEnemies();

    const auto& enemySpawns = m_currentMap->GetEnemySpawns();

    // Create enemies based on their type codes
    for (const auto& spawn : enemySpawns) {
        float x = spawn.posX;
        float y = spawn.posY;
        std::string enemyType = spawn.enemyType;

        // Create appropriate enemy type based on spawn code
        if (enemyType == "E1") {
            g_enemies.push_back(new Enemy(x, y, 100.0f));  // Normal enemy
        }
        else if (enemyType == "E2") {
            g_enemies.push_back(new FlyEnemy(x, y));    // Shield enemy
        }
        else if (enemyType == "E3") {
            g_enemies.push_back(new MageEnemy(x, y));      // Mage enemy
        }
        else if (enemyType == "E4") {
            g_enemies.push_back(new FastEnemy(x, y));      // Fast enemy
        }
        else if (enemyType == "E5") {  // 添加爆炸敌人
            g_enemies.push_back(new BombEnemy(x, y));      // Bomb enemy
        }
        else if(enemyType == "E6") {
            g_enemies.push_back(new SquareEnemy(x, y));    //square enemy
        }
        else if (enemyType == "E7") {
            g_enemies.push_back(new BeamEnemy(x, y));    //square enemy
        }
    }
}
// Find and return a map by name from the map collection
Map* MapManager::GetMap(const std::string& name) {
    for (auto& map : m_maps) {
        if (map.GetName() == name) {
            return &map;
        }
    }
    return nullptr;
}

void MapManager::ReloadCurrentMap() {
    if (!m_currentMap) return;
    // Respawn player at the last entered spawn point
    RespawnPlayer(m_enteredSpawnId);
    // Recreate enemies for the current map
    CreateMapEnemies();
}

const std::string& MapManager::GetCurrentMapName() const {
    static std::string empty = "";
    if (m_currentMap) {
        return m_currentMap->GetName();
    }
    return empty;
}

// Respawn the player at the specified spawn point or default location
void MapManager::RespawnPlayer(int spawnId) {
    float spawnX, spawnY;

    // Try to use specified spawn point
    if (spawnId != -1 && m_currentMap->GetSpawnPoint(spawnId, spawnX, spawnY)) {
        g_player.posX = spawnX;
        g_player.posY = spawnY;
        m_enteredSpawnId = spawnId;
    }
    // Fall back to default spawn point
    else if (m_currentMap->GetDefaultSpawnPoint(spawnX, spawnY)) {
        g_player.posX = spawnX;
        g_player.posY = spawnY;
        m_enteredSpawnId = m_currentMap->GetDefaultSpawnId();
    }
    // Use hardcoded fallback position if no spawn points are available
    else {
        g_player.posX = 0.0f;
        g_player.posY = -0.5f;
        m_enteredSpawnId = -1;
    }

    // Reset player state for fresh start
    g_player.isDead = false;
    g_player.deathTimer = 0.0f;
    g_player.health = g_player.maxHealth;
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;
    g_player.isOnGround = false;
    g_player.isMoving = false;
    g_player.isDashing = false;
    g_player.dashTimer = 0.0f;
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
    g_player.facingRight = true;
}

// Initialize all game maps and set up the initial game state
void MapManager::InitializeMaps() {
    // FOR STAGE1
    // Create test map with basic layout
    Map testMap("World1Area1", 0.15f, 0.15f);
    testMap.CreateWorld1Area1Map();
    AddMap(testMap);

    // Create forest-themed map
    Map forestMap("World1Area2", 0.15f, 0.15f);
    forestMap.CreateWorld1Area2Map();
    AddMap(forestMap);

    // Create ice-themed map
    Map iceMap("World1Area3", 0.15f, 0.15f);
    iceMap.CreateWorld1Area3Map();
    AddMap(iceMap);

    // create the world1 area4
    Map world1Area4Map("World1Area4", 0.15f, 0.15f);
    world1Area4Map.CreateWorld1Area4Map();
    AddMap(world1Area4Map);

    // create the world1 area5
    Map world1Area5Map("World1Area5", 0.15f, 0.15f);
    world1Area5Map.CreateWorld1Area5Map();
    AddMap(world1Area5Map);

    // create the world1 area6
    Map world1Area6Map("World1Area6", 0.15f, 0.15f);
    world1Area6Map.CreateWorld1Area6Map();
    AddMap(world1Area6Map);

    // create the world1 area7
    Map world1Area7Map("World1Area7", 0.15f, 0.15f);
    world1Area7Map.CreateWorld1Area7Map();
    AddMap(world1Area7Map);

    // Create boss map
    Map bossMap ("boss", 0.15f, 0.15f);
    bossMap.CreateBossMap();
    AddMap(bossMap);

    // Create cake map
    Map cakeMap("cake", 0.15f, 0.15f);
    cakeMap.CreateCakeMap();
    AddMap(cakeMap);

    // FOR STAGE2
    // create the world2 area1
    Map world2Area1Map("World2Area1", 0.15f, 0.15f);
    world2Area1Map.CreateWorld2Area1Map();
    AddMap(world2Area1Map);

    // create the world2 area2
    Map world2Area2Map("World2Area2", 0.15f, 0.15f);
    world2Area2Map.CreateWorld2Area2Map();
    AddMap(world2Area2Map);

    // create the world2 area3
    Map world2Area3Map("World2Area3", 0.15f, 0.15f);
    world2Area3Map.CreateWorld2Area3Map();
    AddMap(world2Area3Map);

    // create the world2 area4
    Map world2Area4Map("World2Area4", 0.15f, 0.15f);
    world2Area4Map.CreateWorld2Area4Map();
    AddMap(world2Area4Map);

    // create the world2 area5
    Map world2Area5Map("World2Area5", 0.15f, 0.15f);
    world2Area5Map.CreateWorld2Area5Map();
    AddMap(world2Area5Map);

    // create the world2 area6
    Map world2Area6Map("World2Area6", 0.15f, 0.15f);
    world2Area6Map.CreateWorld2Area6Map();
    AddMap(world2Area6Map);

    // create the world2 area7
    Map world2Area7Map("World2Area7", 0.15f, 0.15f);
    world2Area7Map.CreateWorld2Area7Map();
    AddMap(world2Area7Map);

    // Set initial current map to test map
    m_currentMap = GetMap("World1Area1");

    // Create enemies for the starting map
    CreateMapEnemies();
}