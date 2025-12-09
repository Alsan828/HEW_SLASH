#pragma once
#include "SceneBase.h"
#include "SceneManager.h"
#include "Game.h"

// for the stage 2
class Stage2Scene : public SceneBase
{
private:
    SceneManager* sceneManager;

public:
    Stage2Scene(SceneManager* manager);
    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};