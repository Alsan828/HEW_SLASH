#include "Stage3.h"

//the construct
Stage3Scene::Stage3Scene(SceneManager* manager)
{
    sceneManager = manager;
}

// Initialize the stage
bool Stage3Scene::Init()
{

    g_mapManager.SwitchMap("ice", 0, 1);

    g_gameState = STATE_PLAYING;

    g_gameElapsedTime = 0.0f;
    g_gameMinutes = 0;
    g_gameSeconds = 0;

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
    //ResetGame();  // Reset game state

    //CleanUpGameWorld();  // Release all textures and cleanup
}