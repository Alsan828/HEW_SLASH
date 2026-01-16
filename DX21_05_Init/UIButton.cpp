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

    //if (!s_wasMouseDownInitialized)
    //{
    //    s_wasMouseDown = false;
    //    s_wasMouseDownInitialized = true;
    //}
    //// point to the mouse
    //POINT mouse = g_inputSystem.GetRawMousePosition();
    //float mx = (mouse.x / (float)g_windowWidth) * 2.0f - 1.0f;
    //float my = 1.0f - (mouse.y / (float)g_windowHeight) * 2.0f;
    //// click detection 
    //bool downNow = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    //bool justClicked = downNow && !s_wasMouseDown;
    //s_wasMouseDown = downNow;

    //// hovering test (for when the mouse is on top) with customizable hitbox
    //float hitWidth = width * hitboxScaleWidth;
    //float hitHeight = height * hitboxScaleHeight;
    //float centerY = y - hitboxOffsetY;

    //float l = x - hitWidth * 0.5f, r = x + hitWidth * 0.5f;
    //float t = centerY - hitHeight * 0.5f, b = centerY + hitHeight * 0.5f;
    //isHovered = (mx >= l && mx <= r && my >= t && my <= b);

    //if (isHovered && justClicked) {
    //    return UIButtonResult::Clicked;
    //}
    //if (isHovered) {
    //    return UIButtonResult::Hovered;
    //}
    //return UIButtonResult::None;
    if (!g_wasMouseDownInitialized)
    {
        g_wasMouseDown = false;
        g_wasMouseDownInitialized = true;
    }

    // point to the mouse
    POINT mouse = g_inputSystem.GetRawMousePosition();
    float mx = (mouse.x / (float)g_windowWidth) * 2.0f - 1.0f;
    float my = 1.0f - (mouse.y / (float)g_windowHeight) * 2.0f;

    // click detection 
    bool downNow = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

    // Calculate the hitbox / hover
    float hitWidth = width * hitboxScaleWidth;
    float hitHeight = height * hitboxScaleHeight;
    float centerY = y - hitboxOffsetY;
    float l = x - hitWidth * 0.5f;
    float r = x + hitWidth * 0.5f;
    float t = centerY - hitHeight * 0.5f;
    float b = centerY + hitHeight * 0.5f;

    // if the mouse is inside the hovered area (inside the button hitbox)
    bool hoveredNow = (mx >= l && mx <= r && my >= t && my <= b);

    // the result when clicking or hovering
    UIButtonResult result = UIButtonResult::None;

    if (hoveredNow)
    {
        // the mouse is over the button part
        result = UIButtonResult::Hovered;

        // only triggers the click when the mouse is released after clicking
        if (g_wasMouseDown && !downNow)
        {
            result = UIButtonResult::Clicked;
        }
    }

    // updates the state for the next frame
    g_wasMouseDown = downNow;

    // for the hover part so I can see the texture when hovered
    isHovered = hoveredNow;

    return result;
}

void UIButton::Draw(float baseAlpha) const
{
   
    if (!texture) return;
    // Use hover texture when hovering, otherwise use normal texture
    ID3D11ShaderResourceView* tex = texture;  // Default to normal
    if (isHovered && hoverTexture) {
        tex = hoverTexture;  // Switch to hover texture
    }
    // set the color with no transparency
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    RenderImage(x - width * 0.5f, y - height * 0.5f, width, height, tex, 0, 1, 1);
}

void UIButton::SetHitboxScale(float scaleWidth, float scaleHeight)
{
    hitboxScaleWidth = scaleWidth;
    hitboxScaleHeight = scaleHeight;
}

// so I can adjust the vertical position in case I need
void UIButton::SetHitboxOffset(float offsetY)
{
    hitboxOffsetY = offsetY;
}
