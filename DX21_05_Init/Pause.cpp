#include "Pause.h"

// construct
PauseScene::PauseScene(SceneManager* manager, SceneBase* stage)
{
    sceneManager = manager;
    underlyingScene = stage; // keep pointer to StageScene
}

bool PauseScene::Init()
{
    // Load the pause background texture
    LoadTexture(g_pDevice, "asset/pause.png", &backgroundTexture);

    LoadTexture(g_pDevice, "asset/block.png", &buttonTexture); // for the button

    uiButtons.clear();
    g_mouseIndicator.ShowMouseIndicator(false);

    uiButtons.emplace_back(-0.7f, -0.3f, 0.25f, 0.3f, STAGE, buttonTexture);
    uiButtons.emplace_back(-0.25f, -0.3f, 0.25f, 0.3f, HOWTOPLAY, buttonTexture);
    uiButtons.emplace_back(+0.25f, -0.3f, 0.25f, 0.3f, MENU, buttonTexture);
    uiButtons.emplace_back(+0.7f, -0.3f, 0.25f, 0.3f, QUIT_GAME, buttonTexture);

    return true;
}   

void PauseScene::Update(float deltaTime)
{
    // resume game
    g_inputSystem.Update();

    for (auto& btn : uiButtons)
    {
        if (btn.Process() == UIButtonResult::Clicked)
        {
            SCENE target = btn.GetTargetScene();

            if (target == QUIT_GAME)
            {
                PostQuitMessage(0);// it quits the game
                return;
            }
            else
            {
                sceneManager->SwitchScene(target);
                return;
            }
        }
    }
}

void PauseScene::Draw()
{
 
    if (underlyingScene) {
        underlyingScene->Draw(); // draw stage frozen
    }
    if (backgroundTexture) {
        SetColor(1, 1, 1, 0.5f);
        RenderImage(-1, -1, 2, 2, backgroundTexture, 0, 1, 1);
    }

    // for the buttons
    for (const auto& btn : uiButtons)
        btn.Draw(0.65f);
   
}

void PauseScene::Uninit()
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

    uiButtons.clear();
    g_mouseIndicator.Cleanup();
}
