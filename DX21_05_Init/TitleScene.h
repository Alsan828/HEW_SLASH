#pragma once
#include "SceneBase.h"
#include "InputSystem.h"
#include "SceneManager.h"
#include <vector>
#include "Render.h"
#include "UIButton.h"
//#include "Animation.h"

// タイトル背景用の構造体
struct Background 
{
    float posX, posY;
    float width, height;
};

class TitleScene : public SceneBase 
{
private:
    SceneManager* sceneManager;   // シーンマネージャーへのポインタ

    //ID3D11ShaderResourceView* backgroundTexture;

    // タイトル導入アニメーション用
    std::vector<ID3D11ShaderResourceView*> frames;
    ID3D11ShaderResourceView* tex;

    Animation m_titleAnim;
    bool m_playing = false;

    std::vector<UIButton> uiButtons;

public:
    TitleScene(SceneManager* manager); // コンストラクタ

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};

extern Animation paddingTitleAnim;
extern InputSystem g_inputSystem;   // グローバル入力システムを使う
extern ID3D11Device* g_pDevice;     // テクスチャ読み込み用デバイス
