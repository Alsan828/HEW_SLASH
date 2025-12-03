#pragma once
#include <vector>
#include <string>
#include <unordered_map>

// Tile information using string-based representation
struct TileInfo {
    std::string code;      // Two-character code, e.g., "G1", "E1", "PF"
    std::string type;      // Type name: ground, wall, enemy, portal, etc.
    std::string subtype;   // Subtype: forest, ice, enemy1, etc.
    bool isSolid;          // Whether the tile is solid (collidable)
    bool isSpawn;          // Whether the tile is a spawn point
    bool isPortal;         // Whether the tile is a portal
    bool isEnemy;          // Whether the tile represents an enemy spawn
};

// Map tile structure containing position, size, and tile properties
struct MapTile {
    float posX, posY;      // Tile position coordinates
    float width, height;   // Tile dimensions
    TileInfo tileInfo;     // Tile properties using TileInfo instead of TileType
    std::string targetMap; // Target map for portal tiles
    int linkedSpawnId;     // Associated spawn point ID
};

// Player spawn point information
struct SpawnPoint {
    float posX, posY;      // Spawn position coordinates
    int id;                // Unique spawn point identifier
    std::string name;      // Descriptive name for the spawn point
};

// Enemy spawn information
struct EnemySpawnInfo {
    float posX, posY;      // Enemy spawn position
    std::string enemyType; // Enemy type code: "E1", "E2", "E3", etc.
    int enemySubtype;      // Enemy subtype identifier
};

// Map layer types for organizing tiles
enum class MapLayer {
    BACKGROUND = 0,  // Background decorative tiles (non-collidable)
    MIDGROUND = 1,   // Main gameplay tiles (collidable objects)
    FOREGROUND = 2   // Foreground decorative tiles (non-collidable)
};

// Main Map class for managing game maps, tiles, and spawn points
class Map {
private:
    std::string m_name;                          // Map identifier name
    std::vector<MapTile> m_backgroundTiles;     // Background layer tiles
    std::vector<MapTile> m_midgroundTiles;      // Midground layer tiles (main gameplay)
    std::vector<MapTile> m_foregroundTiles;     // Foreground layer tiles
    std::vector<SpawnPoint> m_spawnPoints;      // Player spawn points
    std::vector<EnemySpawnInfo> m_enemySpawns;  // Enemy spawn locations

    float m_gridWidth, m_gridHeight;            // Grid cell dimensions
    int m_defaultSpawnId;                       // Default spawn point ID

    // Tile type lookup dictionary mapping codes to TileInfo
    std::unordered_map<std::string, TileInfo> m_tileDictionary;

    // Initialize the tile dictionary with all available tile types
    void InitializeTileDictionary();

    // Convert tile code string to TileInfo structure
    TileInfo ParseTileCode(const std::string& code);

public:
    // Constructor: create a map with specified name and grid dimensions
    Map(const std::string& name, float gridWidth, float gridHeight);

    // Map loading methods
    void LoadFromGrid(const std::vector<std::vector<std::string>>& grid, MapLayer layer);
    void AddTile(float x, float y, const std::string& tileCode, MapLayer layer,
        const std::string& targetMap = "", int linkedSpawnId = -1);

    // Get default spawn point ID (returns -1 as placeholder)
    int GetDefaultSpawnId() const { return -1; }

    // Spawn point management
    void AddSpawnPoint(float x, float y, int id, const std::string& name = "");

    // Enemy management
    const std::vector<EnemySpawnInfo>& GetEnemySpawns() const { return m_enemySpawns; }
    void ClearEnemySpawns() { m_enemySpawns.clear(); }

    // Map information accessors
    const std::string& GetName() const { return m_name; }
    const std::vector<MapTile>& GetTiles(MapLayer layer) const;
    std::vector<MapTile>& GetSolidTiles();

    // Spawn point coordinate retrieval
    bool GetSpawnPoint(int spawnId, float& x, float& y) const;
    bool GetDefaultSpawnPoint(float& x, float& y) const;

    // Portal collision detection and information retrieval
    bool CheckPortalCollision(float x, float y, float width, float height,
        std::string& targetMap, int& portalId, int& linkedSpawnId) const;

    // Predefined map creation methods
    void CreateTestMap();
    void CreateForestMap();
    void CreateIceMap();

    // Map clearing methods
    void ClearLayer(MapLayer layer);
    void ClearAll();
};

// Map manager class for handling map transitions and current map state
class MapManager {
private:
    std::vector<Map> m_maps;           // Collection of all available maps
    Map* m_currentMap;                 // Currently active map
    Map* m_previousMap;                // Previously loaded map (for transitions)
    int m_currentPortalId;             // ID of the last used portal
    int m_enteredSpawnId;              // Spawn point ID used when entering map

public:
    MapManager();

    // Map management methods
    void AddMap(const Map& map);
    bool SwitchMap(const std::string& mapName, int enterPortalId = 0, int spawnId = -1);
    Map* GetCurrentMap() { return m_currentMap; }
    Map* GetMap(const std::string& name);

    // Enemy creation for current map
    void CreateMapEnemies();

    void ReloadCurrentMap();
    // Player respawn functionality
    void RespawnPlayer(int spawnId = -1);

    // Initialize all game maps
    void InitializeMaps();

    // State information accessors
    const std::string& GetCurrentMapName() const;
    bool IsMapLoaded() const { return m_currentMap != nullptr; }
    int GetLastSpawnId() const { return m_enteredSpawnId; }
};