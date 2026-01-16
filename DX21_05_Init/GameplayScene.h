#pragma once
#include "SceneBase.h"
#include "StageInfo.h"
#include "SceneManager.h"
#include "Enemy.h"

// this is used for all the areas including the boss area (total of 8 areas)
class GameplayScene : public SceneBase
{
private:
    SceneManager* sceneManager;
    int worldNumber;  // for the world number ( world1, 2 or 3)
    int areaNumber;  // for the area number ( area1, area2, ... area8)
    bool isBossStage; // to check if its boss stage or not

    // for the boss
    Enemy* m_boss;
    float m_bossCheckpointHP;   // this is for the checkpoint of the boss when it respawns so the hp bar will be full or not
    bool m_checkpoint1Reached;  // for when the hp bar has reached 2/3 of it
    bool m_checkpoint2Reached;  // for when the hp bar has reached 1/3 of it

public:
    GameplayScene(SceneManager* manager, int world, int area);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;


    // for the boss hp logic
    void UpdateBossLogic(float deltaTime);
    void CheckBossCheckpoints();      // if we reached the checkpoint of the hp bar
    void RespawnBossAtCheckpoint();   // the boss respawns at the checkpoint, so I can see the bar full or not
    void RenderBossHealthBar();       // draws the boss HP bar
};