//================================
//      RESULT
//================================

#include "Result.h"
#include "SceneManager.h" // for switching scenes

// construct
ResultScene::ResultScene(SceneManager* manager)
{
    sceneManager = manager;
}

//it initializes the objects
bool ResultScene::Init()
{
    LoadTexture(g_pDevice, "asset/result.png", &backgroundTexture);

    LoadTexture(g_pDevice, "asset/UI/button_normal.png", &titleTexture); // for the button
    LoadTexture(g_pDevice, "asset/UI/button_hover.png", &titleHoverTexture);

    uiButtons.emplace_back(-0.8f, -0.9f, 0.4f, 0.8f, TITLE, titleTexture, titleHoverTexture);
    uiButtons.back().SetHitboxScale(0.25f, 0.13f);  // change this values if needed depending on the size of the button
    uiButtons.back().SetHitboxOffset(-0.06f);


    //todo: make another button for going to the next stage

    return true;
}

//it updates the objects
void ResultScene::Update(float deltaTime)
{
    g_inputSystem.Update();


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

//it draws the objects
void ResultScene::Draw()
{
    if (backgroundTexture) {
        // Always set a color before drawing so the texture is visible
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, backgroundTexture, 0, 1, 1);
    }

    for (const auto& btn : uiButtons)
        btn.Draw(0.65f);
}

//it erases the objects
void ResultScene::Uninit()
{
    if (backgroundTexture)
    {
        backgroundTexture->Release();
        backgroundTexture = nullptr;
    }

    if (titleTexture)
    {
        titleTexture->Release();
        titleTexture = nullptr;
    }
    if (titleHoverTexture)
    {
        titleHoverTexture->Release();
        titleHoverTexture = nullptr;
    }

    uiButtons.clear();
    g_mouseIndicator.Cleanup();
}