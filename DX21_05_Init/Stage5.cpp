#include "Stage5.h"

//the construct
Stage5Scene::Stage5Scene(SceneManager* manager)
{
    sceneManager = manager;
}

// Initialize the stage
bool Stage5Scene::Init()
{

    g_mapManager.SwitchMap("World1Area5", 0, 1);

    g_gameState = STATE_PLAYING;

    g_gameElapsedTime = 0.0f;
    g_gameMinutes = 0;
    g_gameSeconds = 0;

    return true;
}

// Update the stage logic
void Stage5Scene::Update(float deltaTime)
{
    UpdateGame(deltaTime);
}

// Draw the stage
void Stage5Scene::Draw()
{
    // Call your global draw function
    DrawGame();
}

// Cleanup
void Stage5Scene::Uninit()
{
    //ResetGame();  // Reset game state

    //CleanUpGameWorld();  // Release all textures and cleanup
}