#pragma once
#include "SceneBase.h"
#include "SceneManager.h"
#include "Game.h"

// for the stage 4
class Stage4Scene : public SceneBase
{
private:
    SceneManager* sceneManager;

public:
    Stage4Scene(SceneManager* manager);
    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};
