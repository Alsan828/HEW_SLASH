#include "Stage2.h"

//the construct
Stage2Scene::Stage2Scene(SceneManager* manager)
{
    sceneManager = manager;
}

// Initialize the stage
bool Stage2Scene::Init()
{
    InitGameWorld();

    g_mapManager.SwitchMap("forest", 0, 1);

    g_gameState = STATE_PLAYING;

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
    ResetGame();  // Reset game state
}