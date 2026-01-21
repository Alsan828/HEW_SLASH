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
    float left = x - hitWidth * 0.5f;
    float right = x + hitWidth * 0.5f;
    float top = centerY - hitHeight * 0.5f;
    float bottom = centerY + hitHeight * 0.5f;

    // if the mouse is inside the hovered area (inside the button hitbox)
    bool hoveredNow = (mx >= left && mx <= right && my >= top && my <= bottom);

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
    //RenderImage(x - width * 0.5f, y - height * 0.5f, width, height, tex, 0, 1, 1);

    // used for rotating the image in case its needed
    float rotate = rotation * 3.14159265f / 180.0f;
    RenderImage(x - width * 0.5f, y - height * 0.5f, width, height, tex,
        0, 1, 1, false, rotate, false);
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
