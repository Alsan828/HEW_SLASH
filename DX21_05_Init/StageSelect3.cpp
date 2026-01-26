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

    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_stage_nomalsize.png", &buttonTexture); // for the button
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_stage_bigsize.png", &buttonHoverTexture);
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_Boss_nomalsize.png", &bossButtonTexture);
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_Boss_bigsize.png", &bossButtonHoverTexture);

    // for the arrow to go to next stageselect screens
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_next_normalsize.png", &arrowTexture);
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_next_bigsize.png", &arrowHoverTexture);

    // add them when I have the actual stage
    // top row
    uiButtons.emplace_back(-0.65f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // go to 3-1
    uiButtons.emplace_back(-0.3f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // go to 3-3
    uiButtons.emplace_back(0.06f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // go to 3-5
    uiButtons.emplace_back(0.43f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // go to 3-7

    // bottom row
    uiButtons.emplace_back(-0.5f, -0.33f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // go to 3-2
    uiButtons.emplace_back(-0.12f, -0.33f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // go to 3-4
    uiButtons.emplace_back(0.26f, -0.33f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // go to 3-6
    uiButtons.emplace_back(0.63f, -0.33f, 0.4f, 0.8f, GAMEPLAY, bossButtonTexture, bossButtonHoverTexture);  // go to boss
    for (auto& btn : uiButtons)
    {
        btn.SetHitboxScale(0.7f, 0.2f);
        btn.SetHitboxOffset(-0.05f);
    }

    // left arrow so I can go to stage select world 2
    uiButtons.emplace_back(-0.905f, 0.0f, 0.4f, 0.6f, STAGESELECT2, arrowTexture, arrowHoverTexture);
    uiButtons.back().SetHitboxScale(0.4f, 0.6f);
    uiButtons.back().SetHitboxOffset(-0.04f);
    uiButtons.back().SetRotation(180.0f); // rotate to pointing to the left


    LoadTexture(g_pDevice, "asset/UI/back/back_normal.png", &backTexture); // for the button
    LoadTexture(g_pDevice, "asset/UI/back/back_hover.png", &backHoverTexture);

    uiButtons.emplace_back(0.8f, -0.9f, 0.4f, 0.8f, returnScene, backTexture, backHoverTexture);
    uiButtons.back().SetHitboxScale(0.25f, 0.13f);  // change this values if needed depending on the size of the button
    uiButtons.back().SetHitboxOffset(-0.06f);

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
            // the first 8 buttons are for the stage
            if (i >= 0 && i < 8)
            {
                int stageNumbers[8] = { 1, 3, 5, 7, 2, 4, 6, 8 }; // 8 is the boss
                sceneManager->SwitchToStage(3, stageNumbers[i]);

                return;
            }
            // for the right arrow
            else if (i == 8)
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


    if (g_numberTexture)
    {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);

        float numberWidth = 0.03f;
        float numberHeight = 0.05f;
        float spaceBetweenNumbers = 0.045f;     // for the space between the numbers

        DrawAreaNumber(3, 1, -0.693f, 0.155f, numberWidth, numberHeight, spaceBetweenNumbers);  // 3-1    1
        DrawAreaNumber(3, 3, -0.343f, 0.155f, numberWidth, numberHeight, spaceBetweenNumbers);  // 3-2    3
        DrawAreaNumber(3, 5, 0.018f, 0.155f, numberWidth, numberHeight, spaceBetweenNumbers);   // 3-3    5
        DrawAreaNumber(3, 7, 0.39f, 0.155f, numberWidth, numberHeight, spaceBetweenNumbers);    // 3-4    7
        DrawAreaNumber(3, 2, -0.54f, -0.325f, numberWidth, numberHeight, spaceBetweenNumbers);  // 3-5    2
        DrawAreaNumber(3, 4, -0.16f, -0.325f, numberWidth, numberHeight, spaceBetweenNumbers);  // 3-6	  4
        DrawAreaNumber(3, 6, 0.22f, -0.325f, numberWidth, numberHeight, spaceBetweenNumbers);   // 3-7    6
    }
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
    if (bossButtonTexture)
    {
        bossButtonTexture->Release();
        bossButtonTexture = nullptr;
    }
    if (bossButtonHoverTexture)
    {
        bossButtonHoverTexture->Release();
        bossButtonHoverTexture = nullptr;
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