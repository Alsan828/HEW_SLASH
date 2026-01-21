//================================
//      RESULT
//================================

#include "Result.h"
#include "SceneManager.h" // for switching scenes

// construct
ResultScene::ResultScene(SceneManager* manager, int completedWorld)
{
    sceneManager = manager;
    m_completedWorld = completedWorld;
    m_nextScene = MENU; // default to MENU 
}

//it initializes the objects
bool ResultScene::Init()
{
    // Result screen is UI-driven; show mouse cursor.
    ShowCursor(TRUE);

    LoadTexture(g_pDevice, "asset/UI/result/background.png", &backgroundTexture);
    LoadTexture(g_pDevice, "asset/UI/result/normal_score.png", &normalScoreTexture);

    LoadTexture(g_pDevice, "asset/UI/number.png", &numberTexture);
    LoadTexture(g_pDevice, "asset/UI/dot.png", &dotTexture);

    LoadTexture(g_pDevice, "asset/UI/result/title_normal.png", &titleTexture); // for the button
    LoadTexture(g_pDevice, "asset/UI/result/title_hover.png", &titleHoverTexture);
    uiButtons.emplace_back(-0.3f, -0.8f, 0.6f, 1.0f, TITLE, titleTexture, titleHoverTexture);
    uiButtons.back().SetHitboxScale(0.45f, 0.1f);  // change this values if needed depending on the size of the button
    uiButtons.back().SetHitboxOffset(0.02f);

    if (m_completedWorld == 1)
    {
        m_nextScene = STAGESELECT2;  // if you complete world1 you go to world12
    }
    else if (m_completedWorld == 2)
    {
        m_nextScene = STAGESELECT3;  // Cif you completr world2 you go to world3
    }
    else if (m_completedWorld == 3)
    {
        m_nextScene = MENU;  // if you compelte world3 you go to menu as default
    }

    LoadTexture(g_pDevice, "asset/UI/result/next_normal.png", &continueTexture); // for the button
    LoadTexture(g_pDevice, "asset/UI/result/next_hover.png", &continueHoverTexture);
    uiButtons.emplace_back(0.3f, -0.8f, 0.6f, 1.0f, m_nextScene/*MENU*/, continueTexture, continueHoverTexture); //todo: change MENU to next world when there is one
    uiButtons.back().SetHitboxScale(0.63f, 0.1f);  // change this values if needed depending on the size of the button3
    uiButtons.back().SetHitboxOffset(0.02f);

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

    if (normalScoreTexture) {
        // Always set a color before drawing so the texture is visible
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-0.7f, -1.1f, 1.3f, 1.5f, normalScoreTexture, 0, 1, 1);
    }

    // Draw statistics numbers
    if (numberTexture) {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);

        // for kills
        DrawNumber(g_gameStats.GetEnemiesKilled(), -0.6f, 0.45f, 0.1f, 0.1f, numberTexture);

        // for deaths
        DrawNumber(g_gameStats.GetTotalDeaths(), -0.6f, 0.05f, 0.1f, 0.1f, numberTexture);

        // for time
        int minutes = (int)(g_gameStats.GetTotalTime() / 60.0f);
        int seconds = (int)g_gameStats.GetTotalTime() % 60;
        DrawTime(minutes, seconds, -0.6f, 0.25f, 0.1f, numberTexture);
        // for the dot between minutes and seconds
        RenderImage(-0.45f, 0.25f, 0.1f, 0.16f, dotTexture, 0, 1, 1);

        // for the score
        DrawNumber(g_gameStats.GetTotalScore(), 0.3f, 0.05f, 0.1f, 0.1f, numberTexture);
    }

    for (const auto& btn : uiButtons)
        btn.Draw(0.65f);
}


void DrawNumber(int number, float x, float y, float width, float height, ID3D11ShaderResourceView* texture) {
    // if number is 0, just draw 0
    if (number == 0) {
        RenderImage(x, y, width, height, texture, 0, 1, 10, false, 0.0f, false);
        return;
    }

    std::vector<int> digits;
    int temp = number;
    while (temp > 0) {
        digits.push_back(temp % 10);  // Get last digit
        temp /= 10;                    // Remove last digit
    }
    std::reverse(digits.begin(), digits.end()); // reverse the digit to get the end

    // Draw each digit from left to right
    float digitX = x;  // Start at the X position
    for (int digit : digits) {
        // Draw the digit at current position
        RenderImage(digitX, y, width, height, texture, digit, 1, 10, false, 0.0f, false);
        // Move X position to the right for next digit
        // width * 0.7f so there will be a small gap between digits
        digitX += width * 0.7f;
    }
}

void DrawTime(int minutes, int seconds, float x, float y, float size, ID3D11ShaderResourceView* texture) {
    float digitX = x; // start position

    //  the first digit of minutes
    RenderImage(digitX, y, size, size, texture, minutes / 10, 1, 10, false, 0.0f, false);
    digitX += size * 0.7f; // size * 0.7f so there will be a small gap between digits
    // the second digit of minutes
    RenderImage(digitX, y, size, size, texture, minutes % 10, 1, 10, false, 0.0f, false);
    digitX += size * 1.2f; // size * 0.7f so there will be a small gap between digits

    // the first digit of seconds 
    RenderImage(digitX, y, size, size, texture, seconds / 10, 1, 10, false, 0.0f, false);
    digitX += size * 0.7f; // size * 0.7f so there will be a small gap between digits
    // the second digit of seconds
    RenderImage(digitX, y, size, size, texture, seconds % 10, 1, 10, false, 0.0f, false);
}


//it erases the objects
void ResultScene::Uninit()
{
    if (backgroundTexture)
    {
        backgroundTexture->Release();
        backgroundTexture = nullptr;
    }

    if (normalScoreTexture)
    {
        normalScoreTexture->Release();
        normalScoreTexture = nullptr;
    }

    if (numberTexture) {
        numberTexture->Release();
        numberTexture = nullptr;
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

    if (continueTexture)
    {
        continueTexture->Release();
        continueTexture = nullptr;
    }
    if (continueHoverTexture)
    {
        continueHoverTexture->Release();
        continueHoverTexture = nullptr;
    }

    uiButtons.clear();
    g_mouseIndicator.Cleanup();
}