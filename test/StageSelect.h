#pragma once
#include "SceneBase.h"
#include "InputSystem.h"
#include "SceneManager.h"
#include <vector>
#include "Render.h"
#include "Game.h"
#include "UIButton.h"


class StageSelect : public SceneBase
{
private:
    SceneManager* sceneManager;   // pointer to the scene manager

    ID3D11ShaderResourceView* backgroundTexture;

    SCENE returnScene; // for going to back to then scene I want to

    // added december 1st
    std::vector<UIButton> uiButtons;
    ID3D11ShaderResourceView* buttonTexture = nullptr;       // delete this later. this is just for test
    ID3D11ShaderResourceView* buttonHoverTexture = nullptr;   // delete this later. this is just for test

    ID3D11ShaderResourceView* backTexture = nullptr;
    ID3D11ShaderResourceView* backHoverTexture = nullptr;

public:
    StageSelect(SceneManager* manager/*, SCENE returnTo*/); // constructor

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};

extern InputSystem g_inputSystem;   // use the global input system
extern ID3D11Device* g_pDevice;     // device for texture loading

