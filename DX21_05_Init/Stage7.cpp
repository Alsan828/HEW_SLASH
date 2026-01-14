#include "Stage7.h"

//the construct
Stage7Scene::Stage7Scene(SceneManager* manager)
{
    sceneManager = manager;
}

// Initialize the stage
bool Stage7Scene::Init()
{

    g_mapManager.SwitchMap("World1Area7", 0, 1);

    g_gameState = STATE_PLAYING;

    g_gameElapsedTime = 0.0f;
    g_gameMinutes = 0;
    g_gameSeconds = 0;

    return true;
}

// Update the stage logic
void Stage7Scene::Update(float deltaTime)
{
    UpdateGame(deltaTime);
}

// Draw the stage
void Stage7Scene::Draw()
{
    // Call your global draw function
    DrawGame();
}

// Cleanup
void Stage7Scene::Uninit()
{
    //ResetGame();  // Reset game state

    //CleanUpGameWorld();  // Release all textures and cleanup
}