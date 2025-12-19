#pragma once
#include "SceneBase.h"
#include "SceneManager.h"
#include "Game.h"

// for the stage 3
class Stage3Scene : public SceneBase
{
private:
    SceneManager* sceneManager;

public:
    Stage3Scene(SceneManager* manager);
    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};
