#pragma once
#include "SceneBase.h"
#include "SceneManager.h"
#include "Game.h"

// for the stage 6
class Stage6Scene : public SceneBase
{
private:
    SceneManager* sceneManager;

public:
    Stage6Scene(SceneManager* manager);
    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};
