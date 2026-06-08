#include "Map.h"
#include "Game.h"
#include "Enemy.h"
#include <cmath>
#include <algorithm>

namespace {
    const std::string kEmptyTileCode = "00";
}

// ボス 2 体版マップ（boss2）を作成する
void Map::CreateBoss2Map() {
    ClearAll();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
    m_defaultSpawnId = 1;

    std::vector<std::vector<std::string>> midgroundGrid = {
        {"G3","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G4"},
        {"G5","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","OP","OP","00","00","00","00","00","00","00","00","00","00","00","OP","OP","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","BS","00","00","00","00","00","00","00","00","00","00","BS","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","S1","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G6"},
        {"G8","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G9"},};

    LoadFromGrid(midgroundGrid, MapLayer::MIDGROUND);
}

// ボスマップ variant 3: 2 倍速の赤ボス 1 体を作成する
void Map::CreateBoss3Map() {
    ClearAll();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
    m_defaultSpawnId = 1;

    std::vector<std::vector<std::string>> midgroundGrid = {
        {"G3","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G4"},
        {"G5","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","OP","OP","00","00","00","00","00","00","00","00","00","00","00","OP","OP","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","BR","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","S1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G6"},
        {"G8","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G9"},};

    LoadFromGrid(midgroundGrid, MapLayer::MIDGROUND);
}

// Map クラスの実装
Map::Map(const std::string& name, float gridWidth, float gridHeight)
    : m_name(name), m_gridWidth(gridWidth), m_gridHeight(gridHeight), m_defaultSpawnId(0), m_spatialGrid(nullptr) {
    InitializeTileDictionary();
}

Map::~Map() {
    delete m_spatialGrid;
    m_spatialGrid = nullptr;
}

Map::Map(const Map& other)
    : m_name(other.m_name),
      m_backgroundTiles(other.m_backgroundTiles),
      m_midgroundTiles(other.m_midgroundTiles),
      m_foregroundTiles(other.m_foregroundTiles),
      m_spawnPoints(other.m_spawnPoints),
      m_enemySpawns(other.m_enemySpawns),
      m_gridWidth(other.m_gridWidth),
      m_gridHeight(other.m_gridHeight),
      m_defaultSpawnId(other.m_defaultSpawnId),
      m_spatialGrid(nullptr),
      m_tileDictionary(other.m_tileDictionary) {
}

Map& Map::operator=(const Map& other) {
    if (this == &other) return *this;

    delete m_spatialGrid;
    m_spatialGrid = nullptr;

    m_name = other.m_name;
    m_backgroundTiles = other.m_backgroundTiles;
    m_midgroundTiles = other.m_midgroundTiles;
    m_foregroundTiles = other.m_foregroundTiles;
    m_spawnPoints = other.m_spawnPoints;
    m_enemySpawns = other.m_enemySpawns;
    m_gridWidth = other.m_gridWidth;
    m_gridHeight = other.m_gridHeight;
    m_defaultSpawnId = other.m_defaultSpawnId;
    m_tileDictionary = other.m_tileDictionary;

    return *this;
}

Map::Map(Map&& other) noexcept
    : m_name(std::move(other.m_name)),
      m_backgroundTiles(std::move(other.m_backgroundTiles)),
      m_midgroundTiles(std::move(other.m_midgroundTiles)),
      m_foregroundTiles(std::move(other.m_foregroundTiles)),
      m_spawnPoints(std::move(other.m_spawnPoints)),
      m_enemySpawns(std::move(other.m_enemySpawns)),
      m_gridWidth(other.m_gridWidth),
      m_gridHeight(other.m_gridHeight),
      m_defaultSpawnId(other.m_defaultSpawnId),
      m_spatialGrid(other.m_spatialGrid),
      m_tileDictionary(std::move(other.m_tileDictionary)) {
    other.m_spatialGrid = nullptr;
}

Map& Map::operator=(Map&& other) noexcept {
    if (this == &other) return *this;

    delete m_spatialGrid;

    m_name = std::move(other.m_name);
    m_backgroundTiles = std::move(other.m_backgroundTiles);
    m_midgroundTiles = std::move(other.m_midgroundTiles);
    m_foregroundTiles = std::move(other.m_foregroundTiles);
    m_spawnPoints = std::move(other.m_spawnPoints);
    m_enemySpawns = std::move(other.m_enemySpawns);
    m_gridWidth = other.m_gridWidth;
    m_gridHeight = other.m_gridHeight;
    m_defaultSpawnId = other.m_defaultSpawnId;
    m_spatialGrid = other.m_spatialGrid;
    m_tileDictionary = std::move(other.m_tileDictionary);

    other.m_spatialGrid = nullptr;
    return *this;
}

// 利用可能な全タイル種別で辞書を初期化する
void Map::InitializeTileDictionary() {
    // タイル種別辞書を初期化する
    m_tileDictionary = {
        // 空き領域
        {"00", {"00", "empty", "none", false, false, false, false}},

        // 地面タイプ
        {"BB", {"BB", "ground", "black", true, false, false, false}}, // 黒い足場用
        {"G1", {"G1", "ground", "grass", true, false, false, false}},
        {"G2", {"G2", "ground", "top", true, false, false, false}}, // 実際の足場上面の装飾用
        {"G3", {"G3", "ground", "top_corner_left", true, false, false, false}},
        {"G4", {"G4", "ground", "top_corner_right", true, false, false, false}},
        {"G5", {"G5", "ground", "left_side", true, false, false, false}},
        {"G6", {"G6", "ground", "right_side", true, false, false, false}},
        {"G7", {"G7", "ground", "bottom", true, false, false, false}},
        {"G8", {"G8", "ground", "bottom_corner_left", true, false, false, false}},
        {"G9", {"G9", "ground", "bottom_corner_right", true, false, false, false}},
        {"C1", {"C1", "ground", "corner_top_facing_left", true, false, false, false}},
        {"C2", {"C2", "ground", "corner_top_facing_right", true, false, false, false}},
        {"C3", {"C3", "ground", "corner_bottom_facing_left", true, false, false, false}},
        {"C4", {"C4", "ground", "corner_bottom_facing_right", true, false, false, false}},
        {"BD", {"BD", "ground", "boss_decoration", true, false, false, false}},
        
        // 壁タイプ
        {"W1", {"W1", "wall", "brick", true, false, false, false}},
        {"W2", {"W2", "wall", "stone", true, false, false, false}},
       
        // 足場タイプ
        {"P1", {"P1", "platform", "wood", true, false, false, false}},
        {"P2", {"P2", "platform", "metal", true, false, false, false}},
        {"OP", {"OP", "platform", "one_way", true, false, false, false}},  // 一方向足場

        // 敵タイプ
        {"E1", {"E1", "enemy", "normal", false, false, false, true}},
        {"E2", {"E2", "enemy", "shield", false, false, false, true}},
        {"E3", {"E3", "enemy", "mage", false, false, false, true}},
        {"E4", {"E4", "enemy", "fast", false, false, false, true}},
        {"E5", {"E5", "enemy", "bombdd", false, false, false, true}},
        {"E6", {"E6", "enemy", "square", false, false, false, true}},
        {"E7", {"E7", "enemy", "beam", false, false, false, true}},
        {"E8", {"E8", "enemy", "thrower", false, false, false, true}},
        {"E9", {"E9", "enemy", "blind_eye", false, false, false, true}},
        {"BS", {"BS", "enemy", "boss", false, false, false, true}}, // Boss 敵コード
        {"BR", {"BR", "enemy", "boss_red", false, false, false, true}}, // 赤ボスの派生種

        // ポータル / 扉タイプ
        // 既定では、ボス扉以外は次のエリアへ進む。
        {"DF", {"DF", "door", "World1Area2", false, false, true, false}},
        {"DI", {"DI", "door", "World1Area3", false, false, true, false}},
        {"D4", {"D4", "door", "World1Area4", false, false, true, false}},
        {"D5", {"D5", "door", "World1Area5", false, false, true, false}},
        {"D6", {"D6", "door", "World1Area6", false, false, true, false}},
        {"D7", {"D7", "door", "World1Area7", false, false, true, false}},
        {"DB", {"DB", "door", "boss", false, false, true, false}}, // ボス扉

        // World2（stage2）用ポータルタイプ
        {"21", {"21", "door", "World2Area1", false, false, true, false}},
        {"22", {"22", "door", "World2Area2", false, false, true, false}},
        {"23", {"23", "door", "World2Area3", false, false, true, false}},
        {"24", {"24", "door", "World2Area4", false, false, true, false}},
        {"25", {"25", "door", "World2Area5", false, false, true, false}},
        {"26", {"26", "door", "World2Area6", false, false, true, false}},
        {"27", {"27", "door", "World2Area7", false, false, true, false}},
        //{"B2", {"B2", "door", "boss2", false, false, true, false}}, // World2 のボス用

        // World3（stage3）用ポータルタイプ
        {"31", {"31", "door", "World3Area1", false, false, true, false}},
        {"32", {"32", "door", "World3Area2", false, false, true, false}},
        {"33", {"33", "door", "World3Area3", false, false, true, false}},
        {"34", {"34", "door", "World3Area4", false, false, true, false}},
        {"35", {"35", "door", "World3Area5", false, false, true, false}},
        {"36", {"36", "door", "World3Area6", false, false, true, false}},
        {"37", {"37", "door", "World3Area7", false, false, true, false}},
        // 必要ならボスステージも追加する

        // スポーン地点タイプ
        {"S1", {"S1", "spawn", "default", false, true, false, false}},
        {"S2", {"S2", "spawn", "secondary", false, true, false, false}},

        // 装飾タイプ（任意）
        {"B1", {"B1", "decoration", "signWASD", false, false, false, false}},
        {"B2", {"B2", "decoration", "signS", false, false, false, false}},
        {"B3", {"B3", "decoration", "signRight", false, false, false, false}},
        {"B4", {"B4", "decoration", "signRelease", false, false, false, false}},
        {"B5", {"B5", "decoration", "signRed", false, false, false, false}},
        {"B6", {"B6", "decoration", "signPink", false, false, false, false}},
        {"B7", {"B7", "decoration", "signLongClick", false, false, false, false}},
        {"B8", {"B8", "decoration", "signClick", false, false, false, false}},
        {"B9", {"B9", "decoration", "signESC", false, false, false, false}}
        ,
        // 危険なトゲ（方向別）
        {"ddup",   {"ddup",   "hazard", "spike_up",    true, false, false, false}},
        {"ddleft", {"ddleft", "hazard", "spike_left",  true, false, false, false}},
        {"dddown", {"dddown", "hazard", "spike_down",  true, false, false, false}},
        {"ddright",{"ddright","hazard", "spike_right", true, false, false, false}},
        // 汎用トゲタイル（非方向指定）
        {"DD",     {"DD",     "hazard", "spike",       true, false, false, false}},

        // チュートリアルトリガータイル
        {"T1", {"T1", "tutorial", "tutorial_1", false, false, false, false}},
        {"T2", {"T2", "tutorial", "tutorial_2", false, false, false, false}},
        {"T3", {"T3", "tutorial", "tutorial_3", false, false, false, false}},
        {"T4", {"T4", "tutorial", "tutorial_4", false, false, false, false}}
    };
}

// タイルコード文字列を TileInfo 構造体へ変換する
TileInfo Map::ParseTileCode(const std::string& code) {
    auto it = m_tileDictionary.find(code);
    if (it != m_tileDictionary.end()) {
        return it->second;
    }
    // 見つからない場合は空タイルを返す
    return m_tileDictionary.at(kEmptyTileCode);
}

const std::unordered_map<std::string, std::string>& Map::GetPortalTargetMapLookup() {
    static const std::unordered_map<std::string, std::string> lookup = {
        {"World1Area1", "World1Area1"},
        {"World1Area2", "World1Area2"},
        {"World1Area3", "World1Area3"},
        {"World1Area4", "World1Area4"},
        {"World1Area5", "World1Area5"},
        {"World1Area6", "World1Area6"},
        {"World1Area7", "World1Area7"},
        {"boss", "boss"},
        {"World2Area1", "World2Area1"},
        {"World2Area2", "World2Area2"},
        {"World2Area3", "World2Area3"},
        {"World2Area4", "World2Area4"},
        {"World2Area5", "World2Area5"},
        {"World2Area6", "World2Area6"},
        {"World2Area7", "World2Area7"},

        {"World3Area1", "World3Area1"},
        {"World3Area2", "World3Area2"},
        {"World3Area3", "World3Area3"},
        {"World3Area4", "World3Area4"},
        {"World3Area5", "World3Area5"},
        {"World3Area6", "World3Area6"},
        {"World3Area7", "World3Area7"},

        // 扉コード -> 次ステージ（ボス扉を除く）
        {"World1Area2", "World1Area2"},
        {"World1Area3", "World1Area3"},
        {"World1Area4", "World1Area4"},
        {"World1Area5", "World1Area5"},
        {"World1Area6", "World1Area6"},
        {"World1Area7", "World1Area7"},
    };
    return lookup;
}

bool Map::ProcessSpecialTileCode(float x, float y, const std::string& tileCode, const TileInfo& tileInfo) {
    if (tileInfo.isEnemy) {
        EnemySpawnInfo spawn;
        spawn.posX = x;
        spawn.posY = y;
        spawn.enemyType = tileCode;
        // 一部の敵コードは接頭辞以降が数値ではない（例: "BS"）。
        // std::stoi で落ちないようにする。
        spawn.enemySubtype = -1;
        if (tileCode.size() > 1) {
            try {
                spawn.enemySubtype = std::stoi(tileCode.substr(1));
            }
            catch (const std::exception&) {
                spawn.enemySubtype = -1;
            }
        }
        m_enemySpawns.push_back(spawn);
        return true;
    }

    if (tileInfo.isSpawn) {
        int spawnId = std::stoi(tileCode.substr(1));
        AddSpawnPoint(x, y, spawnId, "Spawn_" + tileCode);
        return true;
    }

    return false;
}

// 2D タイルコード配列からマップデータを読み込む
void Map::LoadFromGrid(const std::vector<std::vector<std::string>>& grid, MapLayer layer) {
    ClearLayer(layer);

    auto& tiles = (layer == MapLayer::BACKGROUND) ? m_backgroundTiles :
        (layer == MapLayer::MIDGROUND) ? m_midgroundTiles : m_foregroundTiles;

    int gridRows = static_cast<int>(grid.size());
    int gridCols = gridRows > 0 ? static_cast<int>(grid[0].size()) : 0;

    if (gridRows <= 0 || gridCols <= 0) {
        return;
    }

    // マップ全体のサイズを計算する
    float totalWidth = gridCols * m_gridWidth;
    float totalHeight = gridRows * m_gridHeight;

    // 開始位置を計算する（原点中心）
    float startX = -totalWidth * 0.5f;
    float startY = -totalHeight * 0.5f;

    tiles.reserve(static_cast<size_t>(gridRows) * static_cast<size_t>(gridCols));

    // グリッド内の各セルを処理する
    for (int y = 0; y < gridRows; y++) {
        for (int x = 0; x < gridCols; x++) {
            std::string tileCode = grid[y][x];
            if (tileCode == kEmptyTileCode) continue;  // 空タイルは飛ばす

            TileInfo tileInfo = ParseTileCode(tileCode);

            // タイル位置を計算する
            float tileX = startX + static_cast<float>(x) * m_gridWidth;
            float tileY = startY + static_cast<float>(gridRows - 1 - y) * m_gridHeight;

            if (ProcessSpecialTileCode(tileX, tileY, tileCode, tileInfo)) {
                continue;
            }

            // 通常タイルを作成する
            MapTile tile;
            tile.posX = tileX;
            tile.posY = tileY;
            tile.width = m_gridWidth;
            tile.height = m_gridHeight;
            tile.tileInfo = tileInfo;
            tile.linkedSpawnId = -1;

            if (tileInfo.isPortal) {
                const auto& lookup = GetPortalTargetMapLookup();
                auto it = lookup.find(tileInfo.subtype);
                if (it != lookup.end()) {
                    tile.targetMap = it->second;
                    tile.linkedSpawnId = 1;
                }
            }

            tiles.push_back(tile);
        }
    }
}

// 指定位置に 1 枚のタイルを追加する
void Map::AddTile(float x, float y, const std::string& tileCode, MapLayer layer,
    const std::string& targetMap, int linkedSpawnId) {
    TileInfo tileInfo = ParseTileCode(tileCode);

    if (ProcessSpecialTileCode(x, y, tileCode, tileInfo)) {
        return;
    }

    // 通常タイルを作成する
    MapTile tile;
    tile.posX = x;
    tile.posY = y;
    tile.width = m_gridWidth;
    tile.height = m_gridHeight;
    tile.tileInfo = tileInfo;
    tile.targetMap = targetMap;
    tile.linkedSpawnId = linkedSpawnId;

    // 対応するレイヤーへ追加する
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

// プレイヤーのスポーン地点を追加する
void Map::AddSpawnPoint(float x, float y, int id, const std::string& name) {
    SpawnPoint spawn;
    spawn.posX = x;
    spawn.posY = y;
    spawn.id = id;
    spawn.name = name;
    m_spawnPoints.push_back(spawn);

    // 最初のスポーン地点なら既定スポーンにする
    if (m_defaultSpawnId == 0) {
        m_defaultSpawnId = id;
    }
}

// 指定レイヤー内のタイルをすべて削除する
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

// タイル・スポーン地点・敵スポーンを含む全マップデータを消去する
void Map::ClearAll() {
    m_backgroundTiles.clear();
    m_midgroundTiles.clear();
    m_foregroundTiles.clear();
    m_spawnPoints.clear();
    m_enemySpawns.clear();

    delete m_spatialGrid;
    m_spatialGrid = nullptr;
}

// 指定レイヤーのタイルを取得する
const std::vector<MapTile>& Map::GetTiles(MapLayer layer) const {
    static const std::vector<MapTile> empty;
    switch (layer) {
    case MapLayer::BACKGROUND: return m_backgroundTiles;
    case MapLayer::MIDGROUND: return m_midgroundTiles;
    case MapLayer::FOREGROUND: return m_foregroundTiles;
    default: return empty;
    }
}

// すべての固体タイルを取得する（midground の衝突対象タイル）
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

// ID からスポーン地点座標を取得する
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

// 既定スポーン地点の座標を取得する
bool Map::GetDefaultSpawnPoint(float& x, float& y) const {
    return GetSpawnPoint(m_defaultSpawnId, x, y);
}


// ポータル判定も同様に最適化できる
bool Map::CheckPortalCollision(float x, float y, float width, float height,
    std::string& targetMap, int& portalId, int& linkedSpawnId) const {

    // 元の方法へフォールバックする
    for (const auto& tile : m_midgroundTiles) {
        if (tile.tileInfo.isPortal) {
            // 扉スプライトは 1x2。ポータル判定も高さ 2 タイルとして扱い、
            // 基準タイルより 1 タイル上を中心にする。
            float portalX = tile.posX;
            // ポータル判定を半タイルぶん下へ移動する。
            // 元の判定中心は基準タイルの 0.5 タイル上（posY + 0.5h）だった。
            // 0.5 タイル下げるには中心を +0.5h シフトし、posY + 1.0h にする。
            // ワールド Y 軸が反転している場合（下が負）には逆に見えることがある。
            // その場合は posY + 0.0h を使う。
            float portalY = tile.posY + (tile.height * 0.0f);
            float portalW = tile.width;
            float portalH = tile.height * 2.0f;

            if (CheckCollision(x, y, width, height,
                portalX, portalY, portalW, portalH)) {
                targetMap = tile.targetMap;
                portalId = tile.linkedSpawnId; // linkedSpawnId を portalId として使う
                linkedSpawnId = tile.linkedSpawnId;
                return true;
            }
        }
    }

    return false;
}


// ボスマップを作成する
void Map::CreateBossMap() {
    ClearAll();
    m_spawnPoints.clear();
    m_enemySpawns.clear();
    m_defaultSpawnId = 1;
   
    std::vector<std::vector<std::string>> midgroundGrid = {
        {"G3","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G2","G4"},
        {"G5","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","00","00","00","00","00","00","BS","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","00","00","S1","00","00","00","00","00","00","00","00","00","00","00","00","00","00","G1","G6"},
        {"G5","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G1","G6"},
        {"G8","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G7","G9"},};

    LoadFromGrid(midgroundGrid, MapLayer::MIDGROUND);
}

// ケーキマップを作成する
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
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","G1",},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","G1",},
        {"G1","00","00","00","00","00","00","00","00","00","00","00","00","G1",},
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
    // マップ境界を計算する
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

    // 境界に少し余裕を持たせる
    minX -= 5.0f;
    minY -= 5.0f;
    maxX += 5.0f;
    maxY += 5.0f;

    // 空間グリッドを作成または再構築する
    if (m_spatialGrid) {
        delete m_spatialGrid;
    }
    m_spatialGrid = new SpatialGrid(cellSize, minX, minY, maxX, maxY);
    m_spatialGrid->BuildFromMap(*this);
}

// GetCell の実装
const GridCell& SpatialGrid::GetCell(int x, int y) const {
    if (x < 0 || x >= m_cellsX || y < 0 || y >= m_cellsY) {
        static GridCell emptyCell;
        return emptyCell;
    }
    int index = y * m_cellsX + x;
    return m_cells[index];
}

// Rebuild の実装
void SpatialGrid::Rebuild(Map& map) {
    // 既存セルを空にする
    for (auto& cell : m_cells) {
        cell.tiles.clear();
    }
    // マップからグリッドを再構築する
    BuildFromMap(map);
}

// 一方向足場の衝突判定
bool Map::CheckOneWayPlatformCollision(float x, float y, float width, float height,
    const MapTile& platform, float& penetrationY) const {
    // 基本 AABB 衝突を確認する
    if (x + width <= platform.posX || x >= platform.posX + platform.width ||
        y + height <= platform.posY || y >= platform.posY + platform.height) {
        return false;
    }

    // 各辺のめり込み量を計算する
    float topPenetration = (y + height) - platform.posY;  // プレイヤー底辺から足場上端までの距離

    // 一方向足場では上からの衝突だけを有効にする
    // プレイヤー底辺が足場上端付近にあり、かつ落下中のときだけ有効衝突とみなす
    if (topPenetration > 0 && topPenetration < 0.1f) {  // 小さな許容範囲を設ける
        penetrationY = -topPenetration;  // 負値は上方向へ補正することを示す
        return true;
    }

    return false;
}

// すべての一方向足場を取得する
std::vector<MapTile> Map::GetOneWayPlatforms() const {
    std::vector<MapTile> oneWayPlatforms;

    for (const auto& tile : m_midgroundTiles) {
        if (tile.tileInfo.type == "platform" && tile.tileInfo.subtype == "one_way") {
            oneWayPlatforms.push_back(tile);
        }
    }

    return oneWayPlatforms;
}