#pragma once
#include "SceneBase.h"
#include "InputSystem.h"
#include "SceneManager.h"
#include <vector>
#include "Render.h"
#include "UIButton.h"
#include "Game.h"


class ResultScene : public SceneBase
{
private:
    SceneManager* sceneManager;   // pointer to the scene manager

    ID3D11ShaderResourceView* backgroundTexture = nullptr;
    ID3D11ShaderResourceView* normalScoreTexture = nullptr;
    //todo: make also for high score and low sccore texture

    ID3D11ShaderResourceView* numberTexture = nullptr;


    std::vector<UIButton> uiButtons;
    ID3D11ShaderResourceView* titleTexture = nullptr;
    ID3D11ShaderResourceView* titleHoverTexture = nullptr;

    ID3D11ShaderResourceView* continueTexture = nullptr;
    ID3D11ShaderResourceView* continueHoverTexture = nullptr;

    int m_completedWorld; // to track what world (stage) was completed
    SCENE m_nextScene;

public:
    ResultScene(SceneManager* manager, int completedWorld); // constructor

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};

extern InputSystem g_inputSystem;   // use the global input system
extern ID3D11Device* g_pDevice;     // device for texture loading
