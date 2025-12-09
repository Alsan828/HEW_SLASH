#include "Stage3.h"

//the construct
Stage3Scene::Stage3Scene(SceneManager* manager)
{
    sceneManager = manager;
}

// Initialize the stage
bool Stage3Scene::Init()
{
    InitGameWorld();

    g_mapManager.SwitchMap("ice", 0, 1);

    g_gameState = STATE_PLAYING;

    return true;
}

// Update the stage logic
void Stage3Scene::Update(float deltaTime)
{
    UpdateGame(deltaTime);
}

// Draw the stage
void Stage3Scene::Draw()
{
    // Call your global draw function
    DrawGame();
}

// Cleanup
void Stage3Scene::Uninit()
{
    ResetGame();  // Reset game state
}