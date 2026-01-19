#pragma once
#include "SceneBase.h"
#include "InputSystem.h"
#include "SceneManager.h"
#include <vector>
#include "Render.h"
#include "Game.h"
#include "UIButton.h"

// used for stage select for World 3 (8 areas including boss)
class StageSelect3 : public SceneBase
{
private:
    SceneManager* sceneManager;   // pointer to the scene manager

    ID3D11ShaderResourceView* backgroundTexture;

    SCENE returnScene; // for going to back to then scene I want to

    // added december 1st
    std::vector<UIButton> uiButtons;
    //ID3D11ShaderResourceView* buttonTexture = nullptr;      
    //ID3D11ShaderResourceView* buttonHoverTexture = nullptr; 
    //ID3D11ShaderResourceView* bossButtonTexture = nullptr;
    //ID3D11ShaderResourceView* bossButtonHoverTexture = nullptr;

    ID3D11ShaderResourceView* backTexture = nullptr;
    ID3D11ShaderResourceView* backHoverTexture = nullptr;

    // for the arrow to go to next stageselect screens
    ID3D11ShaderResourceView* arrowTexture = nullptr;
    ID3D11ShaderResourceView* arrowHoverTexture = nullptr;

public:
    StageSelect3(SceneManager* manager, SCENE returnTo); // constructor

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;

    void DrawAreaNumber(int world, int stage, float x, float y, float width, float height, float space);
};

extern InputSystem g_inputSystem;   // use the global input system
extern ID3D11Device* g_pDevice;     // device for texture loading

