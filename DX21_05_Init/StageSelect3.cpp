#include "StageSelect3.h"


StageSelect3::StageSelect3(SceneManager* manager, SCENE returnTo)
{
    backgroundTexture = nullptr;
    sceneManager = manager;
    returnScene = returnTo;
}


bool StageSelect3::Init()
{
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_background.png", &backgroundTexture);      // abckground texture

    // for test now change it later
    //LoadTexture(g_pDevice, "asset/UI/button_normal.png", &buttonTexture); // for the buttons
   // LoadTexture(g_pDevice, "asset/UI/button_hover.png", &buttonHoverTexture);
    //LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_Boss_nomalsize.png", &bossButtonTexture);
    //LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_Boss_bigsize.png", &bossButtonHoverTexture);

    // for the arrow to go to next stageselect screens
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_next_normalsize.png", &arrowTexture);
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_next_bigsize.png", &arrowHoverTexture);

    // add them when I have the actual stage
    //uiButtons.emplace_back(-0.65f, 0.1f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // 2-1
    //uiButtons.emplace_back(-0.25f, 0.1f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // 2-2
    //uiButtons.emplace_back(0.15f, 0.1f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // 2-3
    //uiButtons.emplace_back(0.65f, 0.1f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // 2-4
    //uiButtons.emplace_back(-0.65f, -0.5f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // 2-5
    //uiButtons.emplace_back(-0.25f, -0.5f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // 2-6
    //uiButtons.emplace_back(0.15f, -0.5f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // 2-7
    //uiButtons.emplace_back(0.63f, -0.33f, 0.4f, 0.8f, GAMEPLAY, bossButtonTexture, bossButtonHoverTexture);  // go to boss
    //for (auto& btn : uiButtons)
    //{
    //    btn.SetHitboxScale(0.7f, 0.2f);
    //    btn.SetHitboxOffset(-0.05f);
    //}

    // left arrow so I can go to stage select world 2
    uiButtons.emplace_back(-0.92f, -0.2f, 0.4f, 0.6f, STAGESELECT2, arrowTexture, arrowHoverTexture);
    uiButtons.back().SetHitboxScale(0.4f, 0.6f);
    uiButtons.back().SetHitboxOffset(-0.04f);
    uiButtons.back().SetRotation(180.0f); // rotate to pointing to the left


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
            if (i == 0) // change to i = 8 later when there are areas (stages)
            {
                sceneManager->SwitchScene(STAGESELECT2);
                return;
            }
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


    //if (g_numberTexture)
    //{
    //    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    //    float numberWidth = 0.03f;
    //    float numberHeight = 0.05f;
    //    float spaceBetweenNumbers = 0.06f;     // for the space between the numbers

    //    DrawAreaNumber(1, 1, -0.7f, 0.16f, numberWidth, numberHeight, spaceBetweenNumbers);    // 1-1
    //    DrawAreaNumber(1, 2, -0.35f, 0.16f, numberWidth, numberHeight, spaceBetweenNumbers);   // 1-2
    //    DrawAreaNumber(1, 3, 0.01f, 0.16f, numberWidth, numberHeight, spaceBetweenNumbers);    // 1-3
    //    DrawAreaNumber(1, 4, 0.38f, 0.16f, numberWidth, numberHeight, spaceBetweenNumbers);    // 1-4
    //    DrawAreaNumber(1, 5, -0.55f, -0.32f, numberWidth, numberHeight, spaceBetweenNumbers);  // 1-5
    //    DrawAreaNumber(1, 6, -0.17f, -0.32f, numberWidth, numberHeight, spaceBetweenNumbers);  // 1-6	
    //    DrawAreaNumber(1, 7, 0.21f, -0.32f, numberWidth, numberHeight, spaceBetweenNumbers);   // 1-7
    //}
}

void StageSelect3::DrawAreaNumber(int world, int stage, float x, float y, float width, float height, float space)
{
    // Draw world number (first digit)
    RenderImage(x, y, width, height, g_numberTexture, world, 1, 10, false, 0.0f, false);

    // Draw stage number (second digit: 1-8) with spacing
    RenderImage(x + space, y, width, height, g_numberTexture, stage, 1, 10, false, 0.0f, false);
}

// for erasing
void StageSelect3::Uninit()
{
    if (backgroundTexture)
    {
        backgroundTexture->Release();
        backgroundTexture = nullptr;
    }
 /*   if (buttonTexture)
    {
        buttonTexture->Release();
        buttonTexture = nullptr;
    }
    if (buttonHoverTexture)
    {
        buttonHoverTexture->Release();
        buttonHoverTexture = nullptr;
    }
    if (bossButtonTexture)
    {
        bossButtonTexture->Release();
        bossButtonTexture = nullptr;
    }
    if (bossButtonHoverTexture)
    {
        bossButtonHoverTexture->Release();
        bossButtonHoverTexture = nullptr;
    }*/

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


    if (arrowTexture)
    {
        arrowTexture->Release();
        arrowTexture = nullptr;
    }
    if (arrowHoverTexture)
    {
        arrowHoverTexture->Release();
        arrowHoverTexture = nullptr;
    }

    // Clear buttons
    uiButtons.clear();
    g_mouseIndicator.Cleanup();
}