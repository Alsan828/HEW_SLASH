#include "Stage2.h"

//the construct
Stage2Scene::Stage2Scene(SceneManager* manager)
{
    sceneManager = manager;
}

// Initialize the stage
bool Stage2Scene::Init()
{

    g_mapManager.SwitchMap("World1Area2", 0, 1);
    

    g_gameState = STATE_PLAYING;

    g_gameElapsedTime = 0.0f;
    g_gameMinutes = 0;
    g_gameSeconds = 0;

    return true;
}

// Update the stage logic
void Stage2Scene::Update(float deltaTime)
{
    UpdateGame(deltaTime);
}

// Draw the stage
void Stage2Scene::Draw()
{
    // Call your global draw function
    DrawGame();
}

// Cleanup
void Stage2Scene::Uninit()
{
    //ResetGame();  // Reset game state

    //CleanUpGameWorld();  // Release all textures and cleanup
}