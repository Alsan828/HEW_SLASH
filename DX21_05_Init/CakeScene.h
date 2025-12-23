#pragma once
#include "SceneBase.h"
#include "SceneManager.h"
#include "Game.h"
#include "Enemy.h"


// added November 14th
class CakeScene : public SceneBase
{
private:
    SceneManager* sceneManager;

public:
    CakeScene(SceneManager* manager);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};