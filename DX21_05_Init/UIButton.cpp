#include "UIButton.h"


// デフォルトコンストラクタ
UIButton::UIButton()
{

}

// コンストラクタ
UIButton::UIButton(float centerX, float centerY,float w, float h, SCENE scene, ID3D11ShaderResourceView* tex,ID3D11ShaderResourceView* hoverTex)
{
    x = centerX;
    y = centerY;
    width = w;
    height = h;
    targetScene = scene;
    texture = tex;
    hoverTexture = hoverTex;
}


UIButtonResult UIButton::Process()
{
    if (!g_wasMouseDownInitialized)
    {
        g_wasMouseDown = false;
        g_wasMouseDownInitialized = true;
    }

    // マウス位置を取得する
    POINT mouse = g_inputSystem.GetRawMousePosition();
    float mouseX = (mouse.x / (float)g_windowWidth) * 2.0f - 1.0f;
    float mouseY = 1.0f - (mouse.y / (float)g_windowHeight) * 2.0f;

    // クリック状態を判定する
    bool downNow = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

    // ホバー判定用のヒットボックスを計算する
    float hitWidth = width * hitboxScaleWidth;
    float hitHeight = height * hitboxScaleHeight;
    float centerY = y - hitboxOffsetY;
    float left = x - hitWidth * 0.5f;
    float right = x + hitWidth * 0.5f;
    float top = centerY - hitHeight * 0.5f;
    float bottom = centerY + hitHeight * 0.5f;

    // マウスがホバー領域内（ボタンのヒットボックス内）にあるか確認する
    bool hoveredNow = (mouseX >= left && mouseX <= right && mouseY >= top && mouseY <= bottom);

    // クリックまたはホバー時の結果
    UIButtonResult result = UIButtonResult::None;

    if (hoveredNow)
    {
        // マウスがボタン上にある
        result = UIButtonResult::Hovered;

        // クリック後にマウスを離したときだけクリックを確定する
        if (g_wasMouseDown && !downNow)
        {
            result = UIButtonResult::Clicked;
        }
    }

    // 次のフレーム用に状態を更新する
    g_wasMouseDown = downNow;

    // ホバー中かどうかを保存し、ホバー用テクスチャを表示できるようにする
    isHovered = hoveredNow;

    return result;
}

void UIButton::Draw(float baseAlpha) const
{
   
    if (!texture) return;
    // ホバー中はホバーテクスチャ、それ以外は通常テクスチャを使う
    ID3D11ShaderResourceView* tex = texture;  // 通常状態を既定とする
    if (isHovered && hoverTexture) {
        tex = hoverTexture;  // ホバーテクスチャへ切り替える
    }

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    //RenderImage(x - width * 0.5f, y - height * 0.5f, width, height, tex, 0, 1, 1);

    // 必要に応じて画像を回転できるようにする
    float rotate = rotation * 3.14159265f / 180.0f;
    RenderImage(x - width * 0.5f, y - height * 0.5f, width, height, tex,
        0, 1, 1, false, rotate, false);
}

void UIButton::SetHitboxScale(float scaleWidth, float scaleHeight)
{
    hitboxScaleWidth = scaleWidth;
    hitboxScaleHeight = scaleHeight;
}

// 必要に応じて縦位置を調整する
void UIButton::SetHitboxOffset(float offsetY)
{
    hitboxOffsetY = offsetY;
}
