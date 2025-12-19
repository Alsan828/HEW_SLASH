#pragma once
#include "SceneBase.h"
#include "InputSystem.h"
#include "SceneManager.h"
#include <vector>
#include "Render.h"
#include "Game.h"
#include "UIButton.h"

class UIButton;

class PauseScene : public SceneBase 
{
private:
    SceneManager* sceneManager;   // pointer to the scene manager

    SceneBase* underlyingScene;

    ID3D11ShaderResourceView* blackTexture = nullptr; // this is used to make the stage look transparent

    ID3D11ShaderResourceView* backgroundTexture = nullptr;

    std::vector<UIButton> uiButtons;
    ID3D11ShaderResourceView* continueTexture = nullptr;
    ID3D11ShaderResourceView* continueHoverTexture = nullptr;

    ID3D11ShaderResourceView* controlTexture = nullptr;
    ID3D11ShaderResourceView* controlHoverTexture = nullptr;

    ID3D11ShaderResourceView* selectTexture = nullptr;
    ID3D11ShaderResourceView* selectHoverTexture = nullptr;

    ID3D11ShaderResourceView* quitTexture = nullptr;
    ID3D11ShaderResourceView* quitHoverTexture = nullptr;

    SCENE pausedSceneType; // for depending on the scene you are to pause the game

public:
    PauseScene(SceneManager* manager, SceneBase* stage, SCENE PAUSE);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};