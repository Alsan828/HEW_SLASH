#include "Stage.h"

//the construct
StageScene::StageScene(SceneManager* manager)
{
    sceneManager = manager;
}

// Initialize the stage
bool StageScene::Init()
{

    g_gameState = STATE_PLAYING;

    g_mapManager.SwitchMap("test", -1, -1);
    ResetGame();
    return true;
}

// Update the stage logic
void StageScene::Update(float deltaTime)
{
    UpdateGame(deltaTime);
}

// Draw the stage
void StageScene::Draw()
{
    // Call your global draw function
    DrawGame();
}

// Cleanup
void StageScene::Uninit()
{
    //ResetGame();  // Reset game state

    //CleanUpGameWorld();  // Release all textures and cleanup

}