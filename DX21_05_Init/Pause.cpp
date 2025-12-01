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

    return true;
}   

void PauseScene::Update(float deltaTime)
{
    // resume game
    if (g_inputSystem.IsTogglePressed(VK_RETURN)) // at the end you will use mouse
    {
        sceneManager->SwitchScene(STAGE); // back to stage
    }

    // go to HowToPlay
    if (g_inputSystem.IsTogglePressed(VK_C))  // at the end you will use mouse
    {
        sceneManager->SwitchScene(HOWTOPLAY);
    }

    // go to Title
    if (g_inputSystem.IsTogglePressed(VK_M))  // at the end you will use mouse
    {
        sceneManager->SwitchScene(MENU);
    }
}

void PauseScene::Draw()
{
    // for the pause
    //if (backgroundTexture) {
    //    // Always set a color before drawing so the texture is visible
    //    SetColor(1.0f, 1.0f, 1.0f, 0.5f);
    //    RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, backgroundTexture, 0, 1, 1);
    //}

    if (underlyingScene) {
        underlyingScene->Draw(); // draw stage frozen
    }
    if (backgroundTexture) {
        SetColor(1, 1, 1, 0.5f);
        RenderImage(-1, -1, 2, 2, backgroundTexture, 0, 1, 1);
    }
}

void PauseScene::Uninit()
{
    if (backgroundTexture)
    {
        backgroundTexture->Release();
        backgroundTexture = nullptr;
    }
}
