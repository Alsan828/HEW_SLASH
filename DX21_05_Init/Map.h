#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "Camera.h"

class SpatialGrid;
struct GridCell;
// 文字列ベースで表現するタイル情報
struct TileInfo {
    std::string code = " ";      // 2 文字コード。例: "G1"、"E1"、"PF"
    std::string type = " ";      // 種別名: ground、wall、enemy、portal など
    std::string subtype = " ";   // サブタイプ: forest、ice、enemy1 など
    bool isSolid = false;          // 固体タイルかどうか（衝突あり）
    bool isSpawn = false;          // スポーン地点かどうか
    bool isPortal = false;         // ポータルかどうか
    bool isEnemy = false;          // 敵スポーンを表すかどうか
};

// 位置・サイズ・タイル属性を持つマップタイル構造体
struct MapTile {
    float posX = 0.0f, posY = 0.0f;      // タイル位置座標
    float width = 0.0f, height = 0.0f;   // タイルサイズ
    TileInfo tileInfo;     // TileType の代わりに TileInfo を使う
    std::string targetMap = " "; // ポータルタイルの遷移先マップ
    int linkedSpawnId = -1;     // 関連するスポーンポイント ID
};

// プレイヤー用スポーンポイント情報
struct SpawnPoint {
    float posX = 0.0f, posY = 0.0f;      // スポーン位置座標
    int id = 0;                // 一意なスポーンポイント ID
    std::string name = " ";      // スポーンポイント名
};

// 敵スポーン情報
struct EnemySpawnInfo {
    float posX = 0.0f;
    float posY = 0.0f;      // 敵スポーン位置
    std::string enemyType = " "; // 敵種別コード: "E1"、"E2"、"E3" など
    int enemySubtype = 1;      // 敵サブタイプ ID
};

// タイル整理用のマップレイヤー種別
enum class MapLayer {
    BACKGROUND = 0,  // 背景装飾タイル（非衝突）
    MIDGROUND = 1,   // メインゲームプレイ用タイル（衝突あり）
    FOREGROUND = 2   // 前景装飾タイル（非衝突）
};


// ゲームマップ、タイル、スポーンポイントを管理するメイン Map クラス
class Map {
private:
    std::string m_name;                          // マップ識別名
    std::vector<MapTile> m_backgroundTiles;     // 背景レイヤータイル
    std::vector<MapTile> m_midgroundTiles;      // 中景レイヤータイル（メインゲームプレイ）
    std::vector<MapTile> m_foregroundTiles;     // 前景レイヤータイル
    std::vector<SpawnPoint> m_spawnPoints;      // プレイヤースポーンポイント
    std::vector<EnemySpawnInfo> m_enemySpawns;  // 敵スポーン位置

    float m_gridWidth, m_gridHeight;            // グリッドセルサイズ
    int m_defaultSpawnId;                       // 既定スポーンポイント ID
    SpatialGrid* m_spatialGrid;  // 追加分

    // コードから TileInfo を引くための辞書
    std::unordered_map<std::string, TileInfo> m_tileDictionary;

    // 利用可能な全タイル種別で辞書を初期化する
    void InitializeTileDictionary();

    // タイルコード文字列を TileInfo 構造体へ変換する
    TileInfo ParseTileCode(const std::string& code);

    // グリッド読み込みと逐次タイル配置で共通利用する処理。
    // tileCode がスポーン / 敵として消費され、タイル一覧へ追加不要なら true を返す。
    bool ProcessSpecialTileCode(float x, float y, const std::string& tileCode, const TileInfo& tileInfo);

    // ポータルのサブタイプ -> 遷移先マップ名
    static const std::unordered_map<std::string, std::string>& GetPortalTargetMapLookup();

public:
    // コンストラクタ: 指定名とグリッドサイズでマップを作成する
    Map(const std::string& name, float gridWidth, float gridHeight);

    ~Map();
    Map(const Map& other);
    Map& operator=(const Map& other);
    Map(Map&& other) noexcept;
    Map& operator=(Map&& other) noexcept;
    // 一方向プラットフォーム衝突判定
    bool CheckOneWayPlatformCollision(float x, float y, float width, float height,
        const MapTile& platform, float& penetrationY) const;

    // 一方向プラットフォーム一覧を取得する
    std::vector<MapTile> GetOneWayPlatforms() const;
    SpatialGrid* GetSpatialGrid() { return m_spatialGrid; }
    void BuildSpatialGrid(float cellSize = 2.0f);
    // マップ読み込みメソッド
    void LoadFromGrid(const std::vector<std::vector<std::string>>& grid, MapLayer layer);
    void AddTile(float x, float y, const std::string& tileCode, MapLayer layer,
        const std::string& targetMap = "", int linkedSpawnId = -1);

    // 既定スポーンポイント ID を取得する（未設定時は -1）
    int GetDefaultSpawnId() const { return m_defaultSpawnId; }

    // スポーンポイント管理
    void AddSpawnPoint(float x, float y, int id, const std::string& name = "");

    // 敵管理
    const std::vector<EnemySpawnInfo>& GetEnemySpawns() const { return m_enemySpawns; }
    void ClearEnemySpawns() { m_enemySpawns.clear(); }

    // マップ情報アクセサ
    const std::string& GetName() const { return m_name; }
    const std::vector<MapTile>& GetTiles(MapLayer layer) const;
    std::vector<MapTile>& GetSolidTiles();

    // スポーンポイント座標取得
    bool GetSpawnPoint(int spawnId, float& x, float& y) const;
    bool GetDefaultSpawnPoint(float& x, float& y) const;

    // ポータル衝突判定と情報取得
    bool CheckPortalCollision(float x, float y, float width, float height,
        std::string& targetMap, int& portalId, int& linkedSpawnId) const;

    // 定義済みマップ作成メソッド
    void CreateWorld1Area1Map();
    void CreateWorld1Area2Map();
    void CreateWorld1Area3Map();
    void CreateWorld1Area4Map();
    void CreateWorld1Area5Map();
    void CreateWorld1Area6Map();
    void CreateWorld1Area7Map();
    void CreateBossMap();
    void CreateBoss2Map();
    void CreateBoss3Map();
    void CreateCakeMap();

    void CreateWorld2Area1Map();
    void CreateWorld2Area2Map();
    void CreateWorld2Area3Map();
    void CreateWorld2Area4Map();
    void CreateWorld2Area5Map();
    void CreateWorld2Area6Map();
    void CreateWorld2Area7Map();
    //void CreateBoss2Map();

    void CreateWorld3Area1Map();
    void CreateWorld3Area2Map();
    void CreateWorld3Area3Map();
    void CreateWorld3Area4Map();
    void CreateWorld3Area5Map();
    void CreateWorld3Area6Map();
    void CreateWorld3Area7Map();
    //void CreateBoss3Map();

    // マップクリア用メソッド
    void ClearLayer(MapLayer layer);
    void ClearAll();
};

// マップ遷移と現在マップ状態を管理する MapManager クラス
class MapManager {
private:
    std::vector<Map> m_maps;           // 利用可能な全マップ
    Map* m_currentMap;                 // 現在アクティブなマップ
    Map* m_previousMap;                // 直前のマップ（遷移用）
    int m_currentPortalId;             // 最後に使ったポータル ID
    int m_enteredSpawnId;              // 入場時に使ったスポーンポイント ID

public:
    MapManager();

    // マップ管理メソッド
    void AddMap(const Map& map);
    bool SwitchMap(const std::string& mapName, int enterPortalId = 0, int spawnId = -1);
    Map* GetCurrentMap() { return m_currentMap; }
    Map* GetMap(const std::string& name);

    // 現在マップの敵生成
    void CreateMapEnemies();

    void ReloadCurrentMap();
    // プレイヤー再出現機能
    void RespawnPlayer(int spawnId = -1);

    // 全ゲームマップを初期化する
    void InitializeMaps();

    // 状態情報アクセサ
    const std::string& GetCurrentMapName() const;
    bool IsMapLoaded() const { return m_currentMap != nullptr; }
    int GetLastSpawnId() const { return m_enteredSpawnId; }
};

// 空間グリッドセル
struct GridCell {
    std::vector<MapTile*> tiles;  // 実際のタイルを指す
    int x = 0;
    int y = 0;                     // グリッド座標
};



// 空間グリッド管理クラス
class SpatialGrid {
private:
    float m_cellSize;             // セルサイズ
    float m_worldWidth, m_worldHeight;  // ワールド境界
    float m_minX, m_minY;         // ワールド左下座標

    std::vector<GridCell> m_cells;  // 全セル
    int m_cellsX, m_cellsY;         // グリッド寸法

    // ワールド座標をグリッド座標へ変換する
    int WorldToGridX(float worldX) const;
    int WorldToGridY(float worldY) const;
    int WorldToGridIndex(float worldX, float worldY) const;

public:
    SpatialGrid(float cellSize, float worldMinX, float worldMinY,
        float worldMaxX, float worldMaxY);

    // マップタイルをグリッドへ追加する
    void BuildFromMap(Map& map);

    // 指定領域内のタイルを取得する
    void GetTilesInArea(float x, float y, float width, float height,
        std::vector<MapTile*>& result) const;

    // グリッドを再構築する（マップ変更時に呼ぶ）
    void Rebuild(Map& map);

    // セル情報を取得する
    const GridCell& GetCell(int x, int y) const;
};