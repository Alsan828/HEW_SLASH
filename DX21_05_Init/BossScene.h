//#pragma once
//#include "SceneBase.h"
//#include "SceneManager.h"
//#include "Game.h"
//#include "Enemy.h"
//#include "CakeScene.h"
//
//
//// added November 14th
//class BossScene : public SceneBase
//{
//private:
//    SceneManager* sceneManager;
//    
//    Enemy* m_boss;
//
//    float m_bossCheckpointHP;   // this is for the checkpoint of the boss when it respawns so the hp bar will be full or not
//    bool m_checkpoint1Reached;  // for when the hp bar has reached 2/3 of it
//    bool m_checkpoint2Reached;  // for when the hp bar has reached 1/3 of it
//
//public:
//    BossScene(SceneManager* manager);
//
//    bool Init() override;
//    void Update(float deltaTime) override;
//    void Draw() override;
//    void Uninit() override;
//
//    void RenderBossHealthBar();
//    void CheckBossCheckPoints();     // if we reached the chackpoint of the hp bar
//    void RespawnBossAtCheckpoint();  // the boss respawns at the checkopint, so I can see the bar full or not
//};