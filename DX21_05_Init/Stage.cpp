#include "Stage.h"

//the construct
StageScene::StageScene(SceneManager* manager)
{
    sceneManager = manager;
}

// Initialize the stage
bool StageScene::Init()
{
    InitGameWorld();

    g_mapManager.SwitchMap("stage1", 0, 1);

    g_gameState = STATE_PLAYING;

    g_gameElapsedTime = 0.0f;
    g_gameMinutes = 0;
    g_gameSeconds = 0;

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

    CleanUpGameWorld();  // Release all textures and cleanup

}