#include "Stage6.h"

//the construct
Stage6Scene::Stage6Scene(SceneManager* manager)
{
    sceneManager = manager;
}

// Initialize the stage
bool Stage6Scene::Init()
{

    g_mapManager.SwitchMap("World1Area6", 0, 1);

    g_gameState = STATE_PLAYING;

    g_gameElapsedTime = 0.0f;
    g_gameMinutes = 0;
    g_gameSeconds = 0;

    return true;
}

// Update the stage logic
void Stage6Scene::Update(float deltaTime)
{
    UpdateGame(deltaTime);
}

// Draw the stage
void Stage6Scene::Draw()
{
    // Call your global draw function
    DrawGame();
}

// Cleanup
void Stage6Scene::Uninit()
{
    //ResetGame();  // Reset game state

    //CleanUpGameWorld();  // Release all textures and cleanup
}