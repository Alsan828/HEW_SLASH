#pragma once

#include "SceneBase.h"
#include "SceneManager.h"
#include "Game.h"

// added November 14th
class StageScene : public SceneBase 
{
private:
    SceneManager* sceneManager;

    GameTimer game;

public:
    StageScene(SceneManager* manager);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};