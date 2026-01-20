#include "StageSelect2.h"


StageSelect2::StageSelect2(SceneManager* manager, SCENE returnTo)
{
    backgroundTexture = nullptr;
    sceneManager = manager;
    returnScene = returnTo;
}


bool StageSelect2::Init()
{
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_background.png", &backgroundTexture);      // abckground texture

    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_stage_nomalsize.png", &buttonTexture); // for the button
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_stage_bigsize.png", &buttonHoverTexture);
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_Boss_nomalsize.png", &bossButtonTexture);
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_Boss_bigsize.png", &bossButtonHoverTexture);

    // for the arrow to go to next stageselect screens
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_next_normalsize.png", &arrowTexture);
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_next_bigsize.png", &arrowHoverTexture);

    // top row
    uiButtons.emplace_back(-0.65f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // go to 2-1
    uiButtons.emplace_back(-0.3f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // go to 2-3
    uiButtons.emplace_back(0.06f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // go to 2-5
    uiButtons.emplace_back(0.43f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // go to 2-7

    // bottom row
    uiButtons.emplace_back(-0.5f, -0.33f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // go to 2-2
    uiButtons.emplace_back(-0.12f, -0.33f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // go to 2-4
    uiButtons.emplace_back(0.26f, -0.33f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // go to 2-6
    uiButtons.emplace_back(0.63f, -0.33f, 0.4f, 0.8f, GAMEPLAY, bossButtonTexture, bossButtonHoverTexture);  // go to boss
    for (auto& btn : uiButtons)
    {
        btn.SetHitboxScale(0.7f, 0.2f);
        btn.SetHitboxOffset(-0.05f);
    }

    // left arrow so I can go to stage select world 1
    uiButtons.emplace_back(-0.92f, -0.2f, 0.4f, 0.6f, STAGESELECT, arrowTexture, arrowHoverTexture);
    uiButtons.back().SetHitboxScale(0.4f, 0.6f);
    uiButtons.back().SetHitboxOffset(-0.04f);
	uiButtons.back().SetRotation(-180.0f); // rotate to pointing to the left

    // right arrow so I can go to stage select world 3
    uiButtons.emplace_back(0.92f, -0.2f, 0.4f, 0.6f, STAGESELECT3, arrowTexture, arrowHoverTexture);
    uiButtons.back().SetHitboxScale(0.4f, 0.6f);
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
void StageSelect2::Update(float deltaTime)
{
    g_inputSystem.Update();

    // Check each button for clicks
    for (int i = 0; i < uiButtons.size(); i++)
    {
  
        if (uiButtons[i].Process() == UIButtonResult::Clicked)
        {
            //buttons for the areas (stage) ADD LATER
            if (i >= 0 && i < 8)
            {
                // Map button index to stage number
                int stageNumbers[8] = { 1, 3, 5, 7, 2, 4, 6, 8 }; // 8 is boss
                sceneManager->SwitchToStage(2, stageNumbers[i]);
                return;
            }
            // left arrow to go to stageselect world1
            if (i == 8) // change to i = 8 later when there are areas (stages)
            {
                sceneManager->SwitchScene(STAGESELECT);
                return;
            }
            // right arrow to go to stageselect world3
            else if (i == 9) // change to i = 9 later when there are areas (stages)
            {
                sceneManager->SwitchScene(STAGESELECT3);
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
void StageSelect2::Draw()
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

        // top row
        DrawAreaNumber(2, 1, -0.693f, 0.155f, numberWidth, numberHeight, spaceBetweenNumbers);     // 2-1
        DrawAreaNumber(2, 3, -0.343f, 0.155f, numberWidth, numberHeight, spaceBetweenNumbers);    // 2-3
        DrawAreaNumber(2, 5, 0.018f, 0.155f, numberWidth, numberHeight, spaceBetweenNumbers);    // 2-5
        DrawAreaNumber(2, 7, 0.39f,  0.155f, numberWidth, numberHeight, spaceBetweenNumbers);     // 2-7

        // bottom row
        DrawAreaNumber(2, 2, -0.54f, -0.325f, numberWidth, numberHeight, spaceBetweenNumbers);   // 2-2
        DrawAreaNumber(2, 4, -0.16f, -0.325f, numberWidth, numberHeight, spaceBetweenNumbers);   // 2-4
        DrawAreaNumber(2, 6, 0.22f, -0.325f, numberWidth, numberHeight, spaceBetweenNumbers);    // 2-6
    }
}

void StageSelect2::DrawAreaNumber(int world, int stage, float x, float y, float width, float height, float space)
{
    // Draw world number (first digit)
    RenderImage(x, y, width, height, g_numberTexture, world, 1, 10, false, 0.0f, false);

    // Draw stage number (second digit: 1-8) with spacing
    RenderImage(x + space, y, width, height, g_numberTexture, stage, 1, 10, false, 0.0f, false);
}

// for erasing
void StageSelect2::Uninit()
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