#pragma once
#include "SceneBase.h"
#include "InputSystem.h"
#include "SceneManager.h"
#include <vector>
#include "Render.h"
#include "Game.h"
#include "UIButton.h"

// World 2 用のステージ選択（ボスを含む 8 エリア）
class StageSelect2 : public SceneBase
{
private:
    SceneManager* sceneManager;   // シーンマネージャーへのポインタ

    ID3D11ShaderResourceView* backgroundTexture;

    SCENE returnScene; // 戻り先のシーン

    // 12 月 1 日追加
    std::vector<UIButton> uiButtons;
    ID3D11ShaderResourceView* buttonTexture = nullptr;       
    ID3D11ShaderResourceView* buttonHoverTexture = nullptr;  
    ID3D11ShaderResourceView* bossButtonTexture = nullptr;
    ID3D11ShaderResourceView* bossButtonHoverTexture = nullptr;

    ID3D11ShaderResourceView* backTexture = nullptr;
    ID3D11ShaderResourceView* backHoverTexture = nullptr;

    // 次のステージ選択画面へ移動する矢印用
    ID3D11ShaderResourceView* arrowTexture = nullptr;
    ID3D11ShaderResourceView* arrowHoverTexture = nullptr;

public:
    StageSelect2(SceneManager* manager, SCENE returnTo); // コンストラクタ

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;

    void DrawAreaNumber(int world, int stage, float x, float y, float width, float height, float space);
};

extern InputSystem g_inputSystem;   // グローバル入力システムを使う
extern ID3D11Device* g_pDevice;     // テクスチャ読み込み用デバイス

