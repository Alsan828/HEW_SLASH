#pragma once
#include "SceneBase.h"
#include "SceneManager.h"
#include "Game.h"

// for the stage 5
class Stage5Scene : public SceneBase
{
private:
    SceneManager* sceneManager;

public:
    Stage5Scene(SceneManager* manager);
    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};
