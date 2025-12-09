#include "SceneManager.h"
#include "TitleScene.h"
#include "Stage.h"
#include "Stage2.h"
#include "Stage3.h"
#include "Menu.h"
#include "HowToPlay.h"
#include "Pause.h"
#include "StageSelect.h"

//for initializing it
bool SceneManager::Init(SCENE startScene) 
{
    // Load pause overlay texture once
    LoadTexture(g_pDevice, "asset/pause.png", &g_pauseTexture);

    return SwitchScene(startScene);
}

//for switching between scenes
bool SceneManager::SwitchScene(SCENE newScene) 
{
    SceneBase* oldScene = currentScene;  // used for the pause menu
    SCENE caller = currentSceneType;     // save who called us BEFORE overwrite
    
    if (currentScene) 
    {
        if (newScene != PAUSE && newScene != HOWTOPLAY)  // Only delete if not pausing the game
        {
            currentScene->Uninit();
            delete currentScene;
            currentScene = nullptr;
        }
    }
   
    // this is to initialize the new scene
    currentSceneType = newScene;

    // Initialize the new scene
    switch (newScene) 
    {
    case TITLE:
        currentScene = new TitleScene(this); // for the title
        return currentScene->Init();
        break;

    case STAGE:
        // Clear previous scene when coming from stage select or menu
        if (caller == STAGESELECT) 
        {
            if (previousScene) 
            {
                previousScene->Uninit();
                delete previousScene;
                previousScene = nullptr;
            }
        }
        // resume if coming from pause
        if (previousScene) 
        {
            currentScene = previousScene;
            previousScene = nullptr;
            return true;  // resume without restarting the game
        }
        // default start the stage
        else 
        {
            currentScene = new StageScene(this);
            return currentScene->Init();
        }
        break;

    case STAGE2:
        // Clear previous scene when coming from stage select or menu
        if (caller == STAGESELECT) 
        {
            if (previousScene) 
            {
                previousScene->Uninit();
                delete previousScene;
                previousScene = nullptr;
            }
        }
        // Only resume if coming from pause
        if (previousScene)
        {
            currentScene = previousScene;
            previousScene = nullptr;
            return true;  // resume without restarting the game
        }
        // default start the stage
        else
        {
            currentScene = new Stage2Scene(this);
            return currentScene->Init();
        }
        break;

    case STAGE3:
        // Clear previous scene when coming from stage select or menu
        if (caller == STAGESELECT) 
        {
            if (previousScene) 
            {
                previousScene->Uninit();
                delete previousScene;
                previousScene = nullptr;
            }
        }
        // Only resume if coming from pause
        if (previousScene)
        {
            currentScene = previousScene;
            previousScene = nullptr;
            return true;  // resume without restarting the game
        }
        // default start the stage
        else
        {
            currentScene = new Stage3Scene(this);
            return currentScene->Init();
        }
        break;

    case MENU:
        if (caller == PAUSE) { // if you come from pause scene, start the stage from the beginning
            if (previousScene) {
                previousScene->Uninit();
                delete previousScene;
                previousScene = nullptr;
            }
        }
        currentScene = new MenuScene(this); // for the prototype stage
        return currentScene->Init();
        break;

    case HOWTOPLAY:
        if (caller == PAUSE) // if in pause
        { 
            currentScene = new HowToPlayScene(this, PAUSE); // go back to pause
        }
        else if (caller == MENU) // if in menu
        { 
            currentScene = new HowToPlayScene(this, MENU);// go back to menu
        }
        else 
        {
            currentScene = new HowToPlayScene(this, TITLE); // fallback
        }
        return currentScene->Init();
        break;

    case PAUSE:
        if (caller == STAGE || caller == STAGE2 || caller == STAGE3)  // coming directly from stage to pause. ADD more stages depending on how many there are
        { 
            previousScene = oldScene; // Stage
        }
        if (previousScene) // reuse stage if it exists (so I can see it again)
        {
            currentScene = new PauseScene(this, previousScene, originalPausedScene);
        }
        else // if no stage saved, fallback
        { 
            currentScene = new PauseScene(this, nullptr, STAGE);
        }
        return currentScene->Init();
        break;

    case STAGESELECT:
        currentScene = new StageSelect(this); // for the title
        return currentScene->Init();
        break;

    default:
        return false;
    }

}

//for updating 
void SceneManager::Update(float deltaTime) 
{
    if (currentScene && g_gameState == STATE_PLAYING) 
    {
        currentScene->Update(deltaTime);  // only update when playing
    }
}

//for drawing the objects
void SceneManager::Draw() 
{
    RendererDrawF();

    if (currentScene) currentScene->Draw();

    // If paused, draw pause overlay on top
    if (g_gameState == STATE_PAUSED && g_pauseTexture) {
        SetColor(1.0f, 1.0f, 1.0f, 0.5f); // a bit transparent overlay
        RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, g_pauseTexture, 0, 1, 1);
    }

    RendererDrawB();
}

//for erasing
void SceneManager::Uninit() 
{
    // for the pause
    if (g_pauseTexture) {
        g_pauseTexture->Release();
        g_pauseTexture = nullptr;
    }

    // for the current scene
    if (currentScene) 
    {
        currentScene->Uninit();
        delete currentScene;
        currentScene = nullptr;
    }
}

// Modified game loop
void SceneManager::GameLoop()
{
    g_gameTimer.Tick();
    float delta = g_gameTimer.GetDeltaTime();

    // Update input
    g_inputSystem.Update();

    HandleInput();  // Handle keyboard input
    Update(delta);   // Update game state
    Draw();
}
