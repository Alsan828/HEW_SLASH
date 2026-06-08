#include "Map.h"
#include "Game.h"
#include "Enemy.h"


// コンストラクタ: null ポインタと既定値でマップマネージャーを初期化する
MapManager::MapManager() : m_currentMap(nullptr), m_previousMap(nullptr),
m_currentPortalId(0), m_enteredSpawnId(-1) {
}

// マップコレクションへ新しいマップを追加する
void MapManager::AddMap(const Map& map) {
    m_maps.push_back(map);
}

// ポータル ID とスポーン ID を指定して別マップへ切り替える
bool MapManager::SwitchMap(const std::string& mapName, int enterPortalId, int spawnId) {
    Map* newMap = GetMap(mapName);
    if (!newMap) return false;

    m_previousMap = m_currentMap;
    m_currentMap = newMap;
    m_currentPortalId = enterPortalId;

    // 新しいマップでプレイヤーを再出現させる
    RespawnPlayer(spawnId);

    // 新しいマップ用の敵を生成する
    CreateMapEnemies();

    return true;
}
// Map.cpp
void MapManager::CreateMapEnemies() {
    if (!m_currentMap) return;

    // 新規生成前に既存の敵をクリアする
    CleanupEnemies();

    const auto& enemySpawns = m_currentMap->GetEnemySpawns();

    // タイプコードに応じて敵を生成する
    for (const auto& spawn : enemySpawns) {
        float x = spawn.posX;
        float y = spawn.posY;
        std::string enemyType = spawn.enemyType;

        // スポーンコードに応じた敵を生成する
        if (enemyType == "E1") {
            g_enemies.push_back(new Enemy(x, y, 10.0f));  // 通常敵
        }
        else if (enemyType == "E2") {
            g_enemies.push_back(new FlyEnemy(x, y));    // 飛行敵
        }
        else if (enemyType == "E3") {
            g_enemies.push_back(new MageEnemy(x, y));      // 魔法敵
        }
        else if (enemyType == "E4") {
            g_enemies.push_back(new FastEnemy(x, y));      // 高速敵
        }
        else if (enemyType == "E5") {  // 爆発敵を追加する
            g_enemies.push_back(new BombEnemy(x, y));      // 爆弾敵
        }
        else if(enemyType == "E6") {
            g_enemies.push_back(new SquareEnemy(x, y));    // 四角敵
        }
        else if (enemyType == "E7") {
            g_enemies.push_back(new BeamEnemy(x, y));    // ビーム敵
        }
        else if (enemyType == "E8") {
            g_enemies.push_back(new ThrowerEnemy(x, y));
        }
        else if (enemyType == "BS") {
            // ボス派生をサポートする: BS=通常、BR=赤ボス
            // スポーンコードが "BR" の場合のみ赤ボスとして扱う。
            g_enemies.push_back(new BossEnemy(x, y)); // ボス（BS を共通利用）
        }
        else if (enemyType == "BR") {
            // 赤ボス派生
            FinalBossEnemy* be = new FinalBossEnemy(x, y);
            // 内部速度を上げて赤ボス派生にする
            be->SetDashSpeedMultiplier(40.0f); // 既定 20.0f の 2 倍
            be->SetSlashSpeed(0.03f); // 1 フレーム時間を半分にして高速化
            be->SetChargeDuration(0.5f); // チャージを 2 倍速にする（1.0f -> 0.5f）
            be->SetTint(1.0f, 1.0f, 1.0f); // 赤ボス用の見た目設定
            // 描画側で専用テクスチャが選ばれる前提で、ここでは速度だけ変更する。
            g_enemies.push_back(be);
        }
        else if (enemyType == "E9") {
            g_enemies.push_back(new BlindEyeEnemy(x, y));
        }
        else if (enemyType == "B1") {
            // B1: 看板（Billboard）
            // TODO: 既存の看板 / 装飾システムがあれば、ここで対応オブジェクトを生成する。
            // 現状では誤って Boss を生成しない。
        }
    }
}
// 名前からマップを検索して返す
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
    // 最後に入場したスポーンポイントでプレイヤーを再出現させる
    RespawnPlayer(m_enteredSpawnId);
    // 現在マップの敵を再生成する
    CreateMapEnemies();
}

const std::string& MapManager::GetCurrentMapName() const {
    static std::string empty = "";
    if (m_currentMap) {
        return m_currentMap->GetName();
    }
    return empty;
}

// 指定スポーン地点、または既定位置でプレイヤーを再出現させる
void MapManager::RespawnPlayer(int spawnId) {
    float spawnX, spawnY;

    // 指定スポーンポイントを優先して使う
    if (spawnId != -1 && m_currentMap->GetSpawnPoint(spawnId, spawnX, spawnY)) {
        g_player.posX = spawnX;
        g_player.posY = spawnY;
        m_enteredSpawnId = spawnId;
    }
    // 使えなければ既定スポーンポイントへフォールバックする
    else if (m_currentMap->GetDefaultSpawnPoint(spawnX, spawnY)) {
        g_player.posX = spawnX;
        g_player.posY = spawnY;
        m_enteredSpawnId = m_currentMap->GetDefaultSpawnId();
    }
    // スポーンポイントがなければ固定のフォールバック座標を使う
    else {
        g_player.posX = 0.0f;
        g_player.posY = -0.5f;
        m_enteredSpawnId = -1;
    }

    // プレイヤー状態を初期状態へ戻す
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

// 全ゲームマップを初期化し、初期ゲーム状態を設定する
void MapManager::InitializeMaps() {
    // STAGE1 用
    // 基本レイアウトのテストマップを作成する
    Map testMap("World1Area1", 0.15f, 0.15f);
    testMap.CreateWorld1Area1Map();
    AddMap(testMap);

    // 森テーマのマップを作成する
    Map forestMap("World1Area2", 0.15f, 0.15f);
    forestMap.CreateWorld1Area2Map();
    AddMap(forestMap);

    // 氷テーマのマップを作成する
    Map iceMap("World1Area3", 0.15f, 0.15f);
    iceMap.CreateWorld1Area3Map();
    AddMap(iceMap);

    // world1 area4 を作成する
    Map world1Area4Map("World1Area4", 0.15f, 0.15f);
    world1Area4Map.CreateWorld1Area4Map();
    AddMap(world1Area4Map);

    // world1 area5 を作成する
    Map world1Area5Map("World1Area5", 0.15f, 0.15f);
    world1Area5Map.CreateWorld1Area5Map();
    AddMap(world1Area5Map);

    // world1 area6 を作成する
    Map world1Area6Map("World1Area6", 0.15f, 0.15f);
    world1Area6Map.CreateWorld1Area6Map();
    AddMap(world1Area6Map);

    // world1 area7 を作成する
    Map world1Area7Map("World1Area7", 0.15f, 0.15f);
    world1Area7Map.CreateWorld1Area7Map();
    AddMap(world1Area7Map);

    // ボスマップを作成する
    Map bossMap ("boss", 0.15f, 0.15f);
    bossMap.CreateBossMap();
    AddMap(bossMap);

    // boss2: 2 体ボスマップ
    Map boss2Map("boss2", 0.15f, 0.15f);
    boss2Map.CreateBoss2Map();
    AddMap(boss2Map);

    // boss3: 高速な赤ボス
    Map boss3Map("boss3", 0.15f, 0.15f);
    boss3Map.CreateBoss3Map();
    AddMap(boss3Map);

    // cake マップを作成する
    Map cakeMap("cake", 0.15f, 0.15f);
    cakeMap.CreateCakeMap();
    AddMap(cakeMap);

    // STAGE2 用
    // world2 area1 を作成する
    Map world2Area1Map("World2Area1", 0.15f, 0.15f);
    world2Area1Map.CreateWorld2Area1Map();
    AddMap(world2Area1Map);

    // world2 area2 を作成する
    Map world2Area2Map("World2Area2", 0.15f, 0.15f);
    world2Area2Map.CreateWorld2Area2Map();
    AddMap(world2Area2Map);

    // world2 area3 を作成する
    Map world2Area3Map("World2Area3", 0.15f, 0.15f);
    world2Area3Map.CreateWorld2Area3Map();
    AddMap(world2Area3Map);

    // world2 area4 を作成する
    Map world2Area4Map("World2Area4", 0.15f, 0.15f);
    world2Area4Map.CreateWorld2Area4Map();
    AddMap(world2Area4Map);

    // world2 area5 を作成する
    Map world2Area5Map("World2Area5", 0.15f, 0.15f);
    world2Area5Map.CreateWorld2Area5Map();
    AddMap(world2Area5Map);

    // world2 area6 を作成する
    Map world2Area6Map("World2Area6", 0.15f, 0.15f);
    world2Area6Map.CreateWorld2Area6Map();
    AddMap(world2Area6Map);

    // world2 area7 を作成する
    Map world2Area7Map("World2Area7", 0.15f, 0.15f);
    world2Area7Map.CreateWorld2Area7Map();
    AddMap(world2Area7Map);

    // STAGE3 用
    // world3 area1 を作成する
    Map world3Area1Map("World3Area1", 0.15f, 0.15f);
    world3Area1Map.CreateWorld3Area1Map();
    AddMap(world3Area1Map);

    // world3 area2 を作成する
    Map world3Area2Map("World3Area2", 0.15f, 0.15f);
    world3Area2Map.CreateWorld3Area2Map();
    AddMap(world3Area2Map);

    // world3 area3 を作成する
    Map world3Area3Map("World3Area3", 0.15f, 0.15f);
    world3Area3Map.CreateWorld3Area3Map();
    AddMap(world3Area3Map);

    // world3 area4 を作成する
    Map world3Area4Map("World3Area4", 0.15f, 0.15f);
    world3Area4Map.CreateWorld3Area4Map();
    AddMap(world3Area4Map);

    // world3 area5 を作成する
    Map world3Area5Map("World3Area5", 0.15f, 0.15f);
    world3Area5Map.CreateWorld3Area5Map();
    AddMap(world3Area5Map);

    // world3 area6 を作成する
    Map world3Area6Map("World3Area6", 0.15f, 0.15f);
    world3Area6Map.CreateWorld3Area6Map();
    AddMap(world3Area6Map);

    // world3 area7 を作成する
    Map world3Area7Map("World3Area7", 0.15f, 0.15f);
    world3Area7Map.CreateWorld3Area7Map();
    AddMap(world3Area7Map);

    // 初期カレントマップをテストマップに設定する
    m_currentMap = GetMap("World1Area1");

    // 開始マップ用の敵を生成する
    CreateMapEnemies();
}