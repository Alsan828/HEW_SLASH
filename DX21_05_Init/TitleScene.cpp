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
    // I dont need to use "this" but its better for a clearer code
}

//it initializes the objects in title
bool TitleScene::Init() 
{
    //background.Init("asset/title.png");
   // background.SetPos(0.0f, 0.0f, 0.0f);
   // background.SetSize(960.0f, 640.0f, 0.0f);

    LoadTexture(g_pDevice, "asset/title.png", &backgroundTexture);      // abckground texture


    return true;
}

//it updated the objects in ttile
void TitleScene::Update(float deltaTime) 
{
    g_inputSystem.Update();

    // it goes to the menu scene
    if (g_inputSystem.IsKeyDown(VK_RETURN)) // at the end you will use mouse
    {
        sceneManager->SwitchScene(MENU);
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
}

//it erases the objects in title
void TitleScene::Uninit() 
{
    //RendererUninit();
    if (backgroundTexture) 
    {
        backgroundTexture->Release();
        backgroundTexture = nullptr;
    }
}