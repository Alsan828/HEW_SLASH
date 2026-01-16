#pragma once
#include <string>

// used for the information about all the stages (areas)
class StageInfo
{
private:
    int m_world;
    int m_area;
    std::string m_mapName;

public:
    // the constructor
    StageInfo(int world = 1, int area = 1);

    int GetWorld() const { return m_world; }
    int GetArea() const { return m_area; }
    std::string GetMapName() const { return m_mapName; }

    
    bool IsBoss() const { return m_area == 8; } // check if its the boss or not

    void SetWorld(int world); // for setting the world
    void SetArea(int stage); // for setting the area

    void GenerateMapName(); // in order to generate map name
    
};