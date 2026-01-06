#pragma once
#include "SceneBase.h"
#include "InputSystem.h"
#include "SceneManager.h"
#include <vector>
#include "Render.h"
#include "UIButton.h"


class ResultScene : public SceneBase
{
private:
    SceneManager* sceneManager;   // pointer to the scene manager

    ID3D11ShaderResourceView* backgroundTexture = nullptr;


    std::vector<UIButton> uiButtons;
    ID3D11ShaderResourceView* titleTexture = nullptr;
    ID3D11ShaderResourceView* titleHoverTexture = nullptr;

public:
    ResultScene(SceneManager* manager); // constructor

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};

extern InputSystem g_inputSystem;   // use the global input system
extern ID3D11Device* g_pDevice;     // device for texture loading
