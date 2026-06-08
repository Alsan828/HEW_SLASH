#include "StageInfo.h"

// コンストラクタ
StageInfo::StageInfo(int world, int area)
{
    m_world = world;      // ワールド番号
    m_area = area;        // ステージ番号
    GenerateMapName();    // マップ名を生成する
}

// ワールド番号を設定する
void StageInfo::SetWorld(int world)
{
    m_world = world;
    GenerateMapName();
}

// エリア番号を設定する
void StageInfo::SetArea(int stage)
{
    m_area = stage;
    GenerateMapName();
}

void StageInfo::GenerateMapName()
{
    // ボスエリアはエリア番号 8
    if (m_area == 8) 
    {
        if (m_world == 1) {
            m_mapName = "boss";      // world 1 のボス用
        }
        else if (m_world == 2) {
            m_mapName = "boss2";   // world 2 のボス用
        }
        else if (m_world == 3) {
            m_mapName = "boss3";   // world 3 のボス用
        }
        else {
            m_mapName = "boss";
        }
    }
    else {
        // 通常エリア用
        // Map と MapManager で使う "World1Area1" 形式の名前を生成する
        m_mapName = "World" + std::to_string(m_world) + "Area" + std::to_string(m_area);
    }
}