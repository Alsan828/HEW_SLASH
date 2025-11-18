#include "Map.h"
#include "Game.h"
#include "Enemy.h"

MapManager::MapManager() : m_currentMap(nullptr), m_previousMap(nullptr),
m_currentPortalId(0), m_enteredSpawnId(-1) {
}

void MapManager::AddMap(const Map& map) {
    m_maps.push_back(map);
}

bool MapManager::SwitchMap(const std::string& mapName, int enterPortalId, int spawnId) {
    Map* newMap = GetMap(mapName);
    if (!newMap) return false;

    m_previousMap = m_currentMap;
    m_currentMap = newMap;
    m_currentPortalId = enterPortalId;

    // 重生玩家
    RespawnPlayer(spawnId);

    // 生成新地图的敌人
    CreateMapEnemies();

    return true;
}

void MapManager::CreateMapEnemies() {
    if (!m_currentMap) return;

    CleanupEnemies();

    const auto& enemySpawns = m_currentMap->GetEnemySpawns();

    for (const auto& spawn : enemySpawns) {
        float x = spawn.posX;
        float y = spawn.posY;
        std::string enemyType = spawn.enemyType;

        if (enemyType == "E1") {
            g_enemies.push_back(new Enemy(x, y, 100.0f));
        }
        else if (enemyType == "E2") {
            g_enemies.push_back(new ShieldEnemy(x, y));
        }
        else if (enemyType == "E3") {
            g_enemies.push_back(new MageEnemy(x, y));
        }
        else if (enemyType == "E4") {
            g_enemies.push_back(new FastEnemy(x, y));
        }
    }
}

// 其余MapManager函数保持不变...
Map* MapManager::GetMap(const std::string& name) {
    for (auto& map : m_maps) {
        if (map.GetName() == name) {
            return &map;
        }
    }
    return nullptr;
}

void MapManager::RespawnPlayer(int spawnId) {
    float spawnX, spawnY;

    if (spawnId != -1 && m_currentMap->GetSpawnPoint(spawnId, spawnX, spawnY)) {
        g_player.posX = spawnX;
        g_player.posY = spawnY;
        m_enteredSpawnId = spawnId;
    }
    else if (m_currentMap->GetDefaultSpawnPoint(spawnX, spawnY)) {
        g_player.posX = spawnX;
        g_player.posY = spawnY;
        m_enteredSpawnId = m_currentMap->GetDefaultSpawnId();
    }
    else {
        g_player.posX = 0.0f;
        g_player.posY = -0.5f;
        m_enteredSpawnId = -1;
    }

    // 重置玩家状态
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;
    g_player.isOnGround = false;
    g_player.isMoving = false;
    g_player.isDashing = false;
    g_player.dashTimer = 0.0f;
    g_player.dashCooldown = 0.0f;
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
    g_player.facingRight = true;
}

void MapManager::InitializeMaps() {
    // 创建测试地图
    Map testMap("test", 0.15f, 0.15f);
    testMap.CreateTestMap();
    AddMap(testMap);

    // 创建森林地图
    Map forestMap("forest", 0.15f, 0.15f);
    forestMap.CreateForestMap();
    AddMap(forestMap);

    // 创建冰地图
    Map iceMap("ice", 0.15f, 0.15f);
    iceMap.CreateIceMap();
    AddMap(iceMap);

    // 设置当前地图
    m_currentMap = GetMap("test");

    // 生成初始敌人
    CreateMapEnemies();
}