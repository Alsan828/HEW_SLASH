#pragma once
#include "SceneBase.h"
#include "InputSystem.h"
#include "SceneManager.h"
#include <vector>
#include "Render.h"
#include "UIButton.h"


class HowToPlayScene : public SceneBase
{
private:
    SceneManager* sceneManager;   // pointer to the scene manager

    ID3D11ShaderResourceView* backgroundTexture;

    SCENE returnScene; // for going to back to then scene I want to

    std::vector<UIButton> uiButtons;
    ID3D11ShaderResourceView* buttonTexture = nullptr;

public:
    HowToPlayScene(SceneManager* manager, SCENE returnTo); // constructor

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};

extern InputSystem g_inputSystem;   // use the global input system
extern ID3D11Device* g_pDevice;     // device for texture loading

