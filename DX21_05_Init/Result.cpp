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

//it initializes the objects in title
bool ResultScene::Init()
{
    LoadTexture(g_pDevice, "asset/result.png", &backgroundTexture);

    return true;
}

//it updates the objects in tile
void ResultScene::Update(float deltaTime)
{
    g_inputSystem.Update();


    if (g_inputSystem.IsTogglePressed(VK_ESCAPE))
    {
        sceneManager->SwitchScene(QUIT_GAME);
    }


}

//it draws the objects in title
void ResultScene::Draw()
{
    if (backgroundTexture) {
        // Always set a color before drawing so the texture is visible
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, backgroundTexture, 0, 1, 1);
    }

}

//it erases the objects in title
void ResultScene::Uninit()
{
    if (backgroundTexture)
    {
        backgroundTexture->Release();
        backgroundTexture = nullptr;
    }
}