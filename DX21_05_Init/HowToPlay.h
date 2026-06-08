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
    SceneManager* sceneManager;   // シーンマネージャーへのポインタ

    ID3D11ShaderResourceView* backgroundTexture = nullptr;

    SCENE returnScene; // 戻り先のシーン

    std::vector<UIButton> uiButtons;
    ID3D11ShaderResourceView* backTexture = nullptr;
    ID3D11ShaderResourceView* backHoverTexture = nullptr;

public:
    HowToPlayScene(SceneManager* manager, SCENE returnTo); // コンストラクタ

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};

extern InputSystem g_inputSystem;   // グローバル入力システムを使う
extern ID3D11Device* g_pDevice;     // テクスチャ読み込み用デバイス

