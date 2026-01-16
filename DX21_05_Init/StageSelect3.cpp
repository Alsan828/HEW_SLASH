#include "StageSelect3.h"


StageSelect3::StageSelect3(SceneManager* manager, SCENE returnTo)
{
    backgroundTexture = nullptr;
    sceneManager = manager;
    returnScene = returnTo;
}


bool StageSelect3::Init()
{
    LoadTexture(g_pDevice, "asset/stageselect.png", &backgroundTexture); // background texture

    // for test now
    LoadTexture(g_pDevice, "asset/UI/button_normal.png", &buttonTexture); // for the buttons
    LoadTexture(g_pDevice, "asset/UI/button_hover.png", &buttonHoverTexture);

    // add them when I have the actual stage
    //uiButtons.emplace_back(-0.65f, 0.1f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // 2-1
    //uiButtons.emplace_back(-0.25f, 0.1f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // 2-2
    //uiButtons.emplace_back(0.15f, 0.1f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // 2-3
    //uiButtons.emplace_back(0.65f, 0.1f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // 2-4
    //uiButtons.emplace_back(-0.65f, -0.5f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // 2-5
    //uiButtons.emplace_back(-0.25f, -0.5f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // 2-6
    //uiButtons.emplace_back(0.15f, -0.5f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // 2-7
    //uiButtons.emplace_back(0.65f, -0.5f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // 2-8 (boss)
    //for (auto& btn : uiButtons)
    //{
    //    btn.SetHitboxScale(0.7f, 0.2f);
    //    btn.SetHitboxOffset(-0.05f);
    //}

    // left arrow so I can go to stage select world 2
    uiButtons.emplace_back(-0.9f, -0.2f, 0.3f, 0.2f, STAGESELECT2, buttonTexture, buttonHoverTexture);
    uiButtons.back().SetHitboxScale(0.15f, 0.15f);
    uiButtons.back().SetHitboxOffset(-0.04f);

    LoadTexture(g_pDevice, "asset/UI/back/back_normal.png", &backTexture); // for the button
    LoadTexture(g_pDevice, "asset/UI/back/back_hover.png", &backHoverTexture);

    uiButtons.emplace_back(0.8f, -0.9f, 0.4f, 0.8f, returnScene, backTexture, backHoverTexture);
    uiButtons.back().SetHitboxScale(0.25f, 0.13f);  // change this values if needed depending on the size of the button
    uiButtons.back().SetHitboxOffset(-0.06f);

    g_mouseIndicator.ShowMouseIndicator(false);
    return true;
}

// update
void StageSelect3::Update(float deltaTime)
{
    g_inputSystem.Update();

    // Check each button for clicks
    for (int i = 0; i < uiButtons.size(); i++)
    {
        if (uiButtons[i].Process() == UIButtonResult::Clicked)
        {
            // for the areas (stages) ADD LATER
            //if (i >= 0 && i < 8)
            //{
            //    // Go to World3Area1, and to the other ones
            //    sceneManager->SwitchToStage(3, i + 1);
            //    return;
            //}
            // left arrow to go to stageselect world2
            /*else*/ if (i == 0) // change to i = 8 later when there are areas (stages)
            {
                sceneManager->SwitchScene(STAGESELECT2);
                return;
            }
            /*else
            {
                sceneManager->SwitchScene(uiButtons[i].GetTargetScene());
                return;
            }*/
            // back button
            else
            {
                sceneManager->SwitchScene(returnScene);
                return;
            }
        }
    }
}

// Draw the stage select screen
void StageSelect3::Draw()
{
    // Draw background
    if (backgroundTexture)
    {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, backgroundTexture, 0, 1, 1);
    }

    // Draw all buttons
    for (const auto& btn : uiButtons)
        btn.Draw(0.65f);
}

// for erasing
void StageSelect3::Uninit()
{
    if (backgroundTexture)
    {
        backgroundTexture->Release();
        backgroundTexture = nullptr;
    }
    if (buttonTexture)
    {
        buttonTexture->Release();
        buttonTexture = nullptr;
    }
    if (buttonHoverTexture)
    {
        buttonHoverTexture->Release();
        buttonHoverTexture = nullptr;
    }

    if (backTexture)
    {
        backTexture->Release();
        backTexture = nullptr;
    }
    if (backHoverTexture)
    {
        backHoverTexture->Release();
        backHoverTexture = nullptr;
    }

    // Clear buttons
    uiButtons.clear();
    g_mouseIndicator.Cleanup();
}