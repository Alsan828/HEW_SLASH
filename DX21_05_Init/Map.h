#pragma once
#include <vector>
#include <string>
#include <unordered_map>

// 使用字符串表示瓦片类型
struct TileInfo {
    std::string code;      // 两个字符的代码，如 "G1", "E1", "PF"
    std::string type;      // 类型名称：ground, wall, enemy, portal等
    std::string subtype;   // 子类型：forest, ice, enemy1等
    bool isSolid;          // 是否是固体
    bool isSpawn;         // 是否是生成点
    bool isPortal;        // 是否是传送门
    bool isEnemy;         // 是否是敌人
};

// 瓦片结构
struct MapTile {
    float posX, posY;
    float width, height;
    TileInfo tileInfo;     // 使用TileInfo而不是TileType
    std::string targetMap; // 传送门目标地图
    int linkedSpawnId;     // 关联的生成点ID
};

// 生成点信息
struct SpawnPoint {
    float posX, posY;
    int id;
    std::string name;
};

// 敌人生成信息
struct EnemySpawnInfo {
    float posX, posY;
    std::string enemyType;  // "E1", "E2", "E3"等
    int enemySubtype;       // 敌人子类型
};

// 地图层类型
enum class MapLayer {
    BACKGROUND = 0,
    MIDGROUND = 1,
    FOREGROUND = 2
};

class Map {
private:
    std::string m_name;
    std::vector<MapTile> m_backgroundTiles;
    std::vector<MapTile> m_midgroundTiles;
    std::vector<MapTile> m_foregroundTiles;
    std::vector<SpawnPoint> m_spawnPoints;
    std::vector<EnemySpawnInfo> m_enemySpawns;  // 敌人生成点

    float m_gridWidth, m_gridHeight;
    int m_defaultSpawnId;

    // 瓦片类型查找表
    std::unordered_map<std::string, TileInfo> m_tileDictionary;

    void InitializeTileDictionary();
    TileInfo ParseTileCode(const std::string& code);

public:
    Map(const std::string& name, float gridWidth, float gridHeight);

    // 地图加载
    void LoadFromGrid(const std::vector<std::vector<std::string>>& grid, MapLayer layer);
    void AddTile(float x, float y, const std::string& tileCode, MapLayer layer,
        const std::string& targetMap = "", int linkedSpawnId = -1);

    int GetDefaultSpawnId() const {
        return -1;
    }

    // 生成点管理
    void AddSpawnPoint(float x, float y, int id, const std::string& name = "");

    // 敌人管理
    const std::vector<EnemySpawnInfo>& GetEnemySpawns() const { return m_enemySpawns; }
    void ClearEnemySpawns() { m_enemySpawns.clear(); }

    // 获取地图信息
    const std::string& GetName() const { return m_name; }
    const std::vector<MapTile>& GetTiles(MapLayer layer) const;
    std::vector<MapTile>& GetSolidTiles();

    // 生成点相关
    bool GetSpawnPoint(int spawnId, float& x, float& y) const;
    bool GetDefaultSpawnPoint(float& x, float& y) const;

    // 传送门检测
    bool CheckPortalCollision(float x, float y, float width, float height,
        std::string& targetMap, int& portalId, int& linkedSpawnId) const;

    // 地图创建
    void CreateTestMap();
    void CreateForestMap();
    void CreateIceMap();

    void ClearLayer(MapLayer layer);
    void ClearAll();
};


// 地图管理器类
class MapManager {
private:
    std::vector<Map> m_maps;
    Map* m_currentMap;
    Map* m_previousMap;
    int m_currentPortalId;
    int m_enteredSpawnId;  // 进入时使用的生成点ID

public:
    MapManager();

    // 地图管理
    void AddMap(const Map& map);
    bool SwitchMap(const std::string& mapName, int enterPortalId = 0, int spawnId = -1);
    Map* GetCurrentMap() { return m_currentMap; }
    Map* GetMap(const std::string& name);

    void CreateMapEnemies();
    // 玩家重生
    void RespawnPlayer(int spawnId = -1);

    // 初始化所有地图
    void InitializeMaps();

    // 状态获取
    const std::string& GetCurrentMapName() const;
    bool IsMapLoaded() const { return m_currentMap != nullptr; }
    int GetLastSpawnId() const { return m_enteredSpawnId; }
};