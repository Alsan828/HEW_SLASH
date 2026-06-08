#pragma once
#include "SceneBase.h"
#include "InputSystem.h"
#include "SceneManager.h"
#include <vector>
#include "Render.h"
#include "Game.h"
#include "UIButton.h"


class MenuScene : public SceneBase
{
private:
    SceneManager* sceneManager;  // シーンマネージャーへのポインタ

    ID3D11ShaderResourceView* backgroundTexture = nullptr;

    std::vector<UIButton> uiButtons;
    ID3D11ShaderResourceView* startTexture = nullptr;
    ID3D11ShaderResourceView* startHoverTexture = nullptr;
    
    ID3D11ShaderResourceView* controlTexture = nullptr;
    ID3D11ShaderResourceView* controlHoverTexture = nullptr;

    ID3D11ShaderResourceView* quitTexture = nullptr;
    ID3D11ShaderResourceView* quitHoverTexture = nullptr;

public:
    MenuScene(SceneManager* manager); // コンストラクタ

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};

extern Animation paddingTitleAnim;
extern InputSystem g_inputSystem;   // グローバル入力システムを使う
extern ID3D11Device* g_pDevice;     // テクスチャ読み込み用デバイス

