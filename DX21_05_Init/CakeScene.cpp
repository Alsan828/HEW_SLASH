#include "CakeScene.h"

//the construct
CakeScene::CakeScene(SceneManager* manager)
{
    sceneManager = manager;
}

// Initialize the stage
bool CakeScene::Init()
{

    g_gameState = STATE_PLAYING;

    g_mapManager.SwitchMap("cake", -1, -1);
    ResetGame();
    return true;
}

// Update the stage logic
void CakeScene::Update(float deltaTime)
{
    UpdateGame(deltaTime);

    // if all the enemies are killed and animations are done
    if (g_enemies.empty()) 
    { 
        g_gameStats.UpdateTime(g_gameElapsedTime);
        g_gameStats.CalculateFinalScore();

        sceneManager->SwitchScene(RESULT); // it goes to result
        return; 
    }
}

// Draw the stage
void CakeScene::Draw()
{
    // Call your global draw function
    DrawGame();
}

// Cleanup
void CakeScene::Uninit()
{
    //ResetGame();  // Reset game state

    //CleanUpGameWorld();  // Release all textures and cleanup

}