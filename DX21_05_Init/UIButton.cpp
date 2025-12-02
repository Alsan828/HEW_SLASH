#include "UIButton.h"


// default constructor
UIButton::UIButton()
{

}

// construct
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
    if (!s_wasMouseDownInitialized)
    {
        s_wasMouseDown = false;
        s_wasMouseDownInitialized = true;
    }

    // point to the mouse
    POINT mouse = g_inputSystem.GetRawMousePosition();
    float mx = (mouse.x / (float)g_windowWidth) * 2.0f - 1.0f;
    float my = 1.0f - (mouse.y / (float)g_windowHeight) * 2.0f;

    // click detection 
    bool downNow = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    bool justClicked = downNow && !s_wasMouseDown;
    s_wasMouseDown = downNow;

    // hovering test (for when the mouse is on top)
    float l = x - width * 0.5f, r = x + width * 0.5f;
    float t = y - height * 0.5f, b = y + height * 0.5f;
    isHovered = (mx >= l && mx <= r && my >= t && my <= b);

    if (isHovered && justClicked) {
        return UIButtonResult::Clicked;
    }
    if (isHovered) {
        return UIButtonResult::Hovered;
    }
    return UIButtonResult::None;
}

void UIButton::Draw(float baseAlpha) const
{
    if (!texture) return;

    ID3D11ShaderResourceView* tex = isHovered && hoverTexture ? hoverTexture : texture;
    float alpha = isHovered ? 1.0f : baseAlpha;

    SetColor(1.0f, 1.0f, 1.0f, alpha);
    RenderImage(x - width * 0.5f, y - height * 0.5f, width, height, tex, 0, 1, 1);
}