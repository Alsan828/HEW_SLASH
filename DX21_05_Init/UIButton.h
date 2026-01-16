#pragma once
#include "Game.h"           
#include "SceneManager.h"
#include "Render.h"


enum class UIButtonResult
{
    None,     // default
    Hovered,  // mouse on the block
    Clicked
};

class UIButton
{
private:
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.4f;
    float height = 0.2f;

    SCENE targetScene = MENU; // make MENU default bc its the scene where there are more buttons (maybe ?)

    ID3D11ShaderResourceView* texture = nullptr;
    ID3D11ShaderResourceView* hoverTexture = nullptr;
    
    bool isHovered = false;

    bool g_wasMouseDown = false;
    bool g_wasMouseDownInitialized = false;

    float hitboxScaleWidth = 1.0f;
    float hitboxScaleHeight = 1.0f;
    float hitboxOffsetY = 0.0f;

public:

    UIButton(); // default contructor

    // constructor 
    UIButton(float centerX, float centerY, float w, float h,
            SCENE scene,
            ID3D11ShaderResourceView* tex,
            ID3D11ShaderResourceView* hoverTex = nullptr);

    UIButtonResult Process();        // call every frame
    void Draw(float baseAlpha = 0.65f) const;

    // Optional: getters/setters if you want to change things later
    void SetPosition(float newX, float newY) { x = newX; y = newY; }

    void SetTargetScene(SCENE scene) { targetScene = scene; }
    SCENE GetTargetScene() const { return targetScene; }

    void SetHitboxScale(float scaleWidth, float scaleHeight); 
    void SetHitboxOffset(float offsetY); 
};