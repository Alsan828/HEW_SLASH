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
    SceneManager* sceneManager;   // シーンマネージャーへのポインタ

    ID3D11ShaderResourceView* backgroundTexture = nullptr;
    ID3D11ShaderResourceView* normalScoreTexture = nullptr;
    // todo: ハイスコア用とロースコア用のテクスチャも追加する

    ID3D11ShaderResourceView* numberTexture = nullptr;
    ID3D11ShaderResourceView* dotTexture = nullptr;

    std::vector<UIButton> uiButtons;
    ID3D11ShaderResourceView* titleTexture = nullptr;
    ID3D11ShaderResourceView* titleHoverTexture = nullptr;

    ID3D11ShaderResourceView* continueTexture = nullptr;
    ID3D11ShaderResourceView* continueHoverTexture = nullptr;

    int m_completedWorld; // どのワールド（ステージ）をクリアしたかを保持する
    SCENE m_nextScene;

public:
    ResultScene(SceneManager* manager, int completedWorld); // コンストラクタ

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};

extern InputSystem g_inputSystem;   // グローバル入力システムを使う
extern ID3D11Device* g_pDevice;     // テクスチャ読み込み用デバイス
