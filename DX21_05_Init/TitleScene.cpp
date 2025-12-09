//================================
//      TITLE
//================================

#include "TitleScene.h"
#include "SceneManager.h" // for switching scenes

//the construct
TitleScene::TitleScene(SceneManager* manager) 
{
    sceneManager = manager;
    backgroundTexture = nullptr;
}

//it initializes the objects in title
bool TitleScene::Init() 
{
    LoadTexture(g_pDevice, "asset/title.png", &backgroundTexture);      // abckground texture

    LoadTexture(g_pDevice, "asset/UI/button_normal.png", &buttonTexture); // for the button
    LoadTexture(g_pDevice, "asset/UI/button_hover.png", &buttonHoverTexture);


    uiButtons.clear();
    g_mouseIndicator.ShowMouseIndicator(false);

    uiButtons.emplace_back(0.0f, -0.0f, 0.8f, 1.0f, MENU, buttonTexture, buttonHoverTexture);

    uiButtons[0].SetHitboxScale(0.7f, 0.22f);  // change this values if needed depending on the size of the button
    uiButtons[0].SetHitboxOffset(-0.07f);

    return true;
}

//it updated the objects in ttile
void TitleScene::Update(float deltaTime) 
{
    g_inputSystem.Update();

    // it goes to the menu scene
    //if (g_inputSystem.IsKeyDown(VK_RETURN)) // at the end you will use mouse
    //{
    //    sceneManager->SwitchScene(MENU);
    //}

    // for the buttons
    for (auto& btn : uiButtons)
    {
        if (btn.Process() == UIButtonResult::Clicked)
        {
            sceneManager->SwitchScene(btn.GetTargetScene());
            return;
        }
    }
}

//it draws the objects in title
void TitleScene::Draw() 
{
    if (backgroundTexture) {
        // Always set a color before drawing so the texture is visible
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, backgroundTexture, 0, 1, 1);
    }

    for (const auto& btn : uiButtons)
        btn.Draw(0.65f);
}

//it erases the objects in title
void TitleScene::Uninit() 
{

    if (backgroundTexture) 
    {
        backgroundTexture->Release();
        backgroundTexture = nullptr;
    }

    if (buttonTexture) {
        buttonTexture->Release();
        buttonTexture = nullptr;
    }

    if (buttonHoverTexture) {  
        buttonHoverTexture->Release();
        buttonHoverTexture = nullptr;
    }

    uiButtons.clear();
    g_mouseIndicator.Cleanup();
}