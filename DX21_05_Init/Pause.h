#pragma once
#include "SceneBase.h"
#include "InputSystem.h"
#include "SceneManager.h"
#include <vector>
#include "Render.h"
#include "Game.h"

class PauseScene : public SceneBase 
{
private:
    SceneManager* sceneManager;   // pointer to the scene manager

    SceneBase* underlyingScene;

    ID3D11ShaderResourceView* backgroundTexture = nullptr;

public:
    PauseScene(SceneManager* manager, SceneBase* stage);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};