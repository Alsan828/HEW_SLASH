#pragma once
#include "SceneBase.h"
#include "SceneManager.h"
#include "Game.h"

// for the stage 7
class Stage7Scene : public SceneBase
{
private:
    SceneManager* sceneManager;

public:
    Stage7Scene(SceneManager* manager);
    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};
