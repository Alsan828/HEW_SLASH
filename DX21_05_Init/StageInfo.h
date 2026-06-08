#pragma once
#include <string>

// すべてのステージ（エリア）情報を保持する
class StageInfo
{
private:
    int m_world;
    int m_area;
    std::string m_mapName;

public:
    // コンストラクタ
    StageInfo(int world = 1, int area = 1);

    int GetWorld() const { return m_world; }
    int GetArea() const { return m_area; }
    std::string GetMapName() const { return m_mapName; }

    
    bool IsBoss() const { return m_area == 8; } // ボスかどうかを確認する

    void SetWorld(int world); // ワールドを設定する
    void SetArea(int stage); // エリアを設定する

    void GenerateMapName(); // マップ名を生成する
    
};