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
    // Create test map with basic layout
    Map testMap("test", 0.15f, 0.15f);
    testMap.CreateTestMap();
    AddMap(testMap);

    // Create forest-themed map
    Map forestMap("forest", 0.15f, 0.15f);
    forestMap.CreateForestMap();
    AddMap(forestMap);

    // Create ice-themed map
    Map iceMap("ice", 0.15f, 0.15f);
    iceMap.CreateIceMap();
    AddMap(iceMap);

    // Create boss map
    Map bossMap ("boss", 0.15f, 0.15f);
    bossMap.CreateBossMap();
    AddMap(bossMap);

    // Set initial current map to test map
    m_currentMap = GetMap("test");

    // Create enemies for the starting map
    CreateMapEnemies();
}