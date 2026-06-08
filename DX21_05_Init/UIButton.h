#pragma once
#include "Game.h"           
#include "SceneManager.h"
#include "Render.h"

// スコープ付き enum。UI ボタン用に使うと名前の衝突を避けやすい。
// 例: UIButtonResult::None のように記述する。
enum class UIButtonResult
{
    None,     // 既定状態
    Hovered,  // マウスがボタン上にある状態
    Clicked
};

class UIButton
{
private:
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.4f;
    float height = 0.2f;

    float rotation = 0.0f; 

    SCENE targetScene = MENU; // ボタン数が多いため MENU を既定シーンにする

    ID3D11ShaderResourceView* texture = nullptr;
    ID3D11ShaderResourceView* hoverTexture = nullptr;
    
    bool isHovered = false;

    bool g_wasMouseDown = false;
    bool g_wasMouseDownInitialized = false;

    float hitboxScaleWidth = 1.0f;
    float hitboxScaleHeight = 1.0f;
    float hitboxOffsetY = 0.0f;

public:

    UIButton(); // デフォルトコンストラクタ

    // コンストラクタ
    UIButton(float centerX, float centerY, float w, float h,
            SCENE scene,
            ID3D11ShaderResourceView* tex,
            ID3D11ShaderResourceView* hoverTex = nullptr);

    UIButtonResult Process();        // 毎フレーム呼び出す
    void Draw(float baseAlpha = 0.65f) const;

    // 必要なら後で変更できるようにするためのゲッター / セッター
    void SetPosition(float newX, float newY) { x = newX; y = newY; }

    void SetTargetScene(SCENE scene) { targetScene = scene; }
    SCENE GetTargetScene() const { return targetScene; }

    void SetHitboxScale(float scaleWidth, float scaleHeight); 
    void SetHitboxOffset(float offsetY);  // 縦方向の位置を調整する

    void SetRotation(float degrees) { rotation = degrees; } 
    float GetRotation() const { return rotation; }
};