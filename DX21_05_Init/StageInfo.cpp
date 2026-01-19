#include "StageInfo.h"

// construct
StageInfo::StageInfo(int world, int area)
{
    m_world = world;      // for the world number
    m_area = area;        // for the stage number
    GenerateMapName();    // generates the map name
}

// for the world
void StageInfo::SetWorld(int world)
{
    m_world = world;
    GenerateMapName();
}

// for the area (which is on each world)
void StageInfo::SetArea(int stage)
{
    m_area = stage;
    GenerateMapName();
}

void StageInfo::GenerateMapName()
{
    // for the boss area, which is area number 8
    if (m_area == 8) 
    {
        if (m_world == 1) {
            m_mapName = "boss";      // for world 1 Boss
        }
        else if (m_world == 2) {
            m_mapName = "boss";   // for world 2 Boss.
        }
        //else if (m_world == 3) {
        //    m_mapName = "boss3";   // for world 3 Boss. add it when there is one
        //}
    }
    else {
        // for normal areas
        // this will make the "World1Area1", and the other ones I have in the map and mapmanager
        m_mapName = "World" + std::to_string(m_world) + "Area" + std::to_string(m_area);
    }
}