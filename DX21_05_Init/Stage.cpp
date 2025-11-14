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
    g_gameState = STATE_PLAYING;

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
    // Reset game state
    ResetGame();
}