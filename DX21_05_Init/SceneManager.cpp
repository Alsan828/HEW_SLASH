#include "SceneManager.h"
#include "TitleScene.h"
#include "Stage.h"
#include "Stage2.h"
#include "Stage3.h"
#include "BossScene.h"
#include "Menu.h"
#include "HowToPlay.h"
#include "Pause.h"
#include "StageSelect.h"

//for initializing it
bool SceneManager::Init(SCENE startScene) 
{

    return SwitchScene(startScene);
}

//for switching between scenes
bool SceneManager::SwitchScene(SCENE newScene) 
{
    SceneBase* oldScene = currentScene;  // used for the pause menu
    SCENE caller = currentSceneType;     // save who called us (the scene type) before overwriting it
    
    if (currentScene) 
    {       
        if (!(currentSceneType == STAGE || currentSceneType == STAGE2 || currentSceneType == STAGE3 || currentSceneType == BOSS) // add here mor stages depening on how many there are 
            || newScene != PAUSE) 
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
        if (caller == PAUSE && previousScene) 
        {
            currentScene = previousScene;     // resume existing stage 
            previousScene = nullptr;
            return true;                     
        }

        if (previousScene) 
        {
            previousScene->Uninit();
            delete previousScene;
            previousScene = nullptr;
        }
        currentScene = new StageScene(this);
        return currentScene->Init();
        break;

    case STAGE2:
        if (caller == PAUSE && previousScene) 
        {
            currentScene = previousScene;     // resume existing stage
            previousScene = nullptr;
            return true;                    
        }

        if (previousScene)
        {
            previousScene->Uninit();
            delete previousScene;
            previousScene = nullptr;
        }
        currentScene = new Stage2Scene(this);
        return currentScene->Init();
        break;

    case STAGE3:
        if (caller == PAUSE && previousScene) 
        {
            currentScene = previousScene;     // resume existing stage 
            previousScene = nullptr;
            return true;                      
        }

        if (previousScene) 
        {
            previousScene->Uninit();
            delete previousScene;
            previousScene = nullptr;
        }
        currentScene = new Stage3Scene(this);
        return currentScene->Init();
        break;

    case BOSS:
        if (caller == PAUSE && previousScene)
        {
            currentScene = previousScene;     // resume existing stage 
            previousScene = nullptr;
            return true;
        }

        if (previousScene)
        {
            previousScene->Uninit();
            delete previousScene;
            previousScene = nullptr;
        }
        currentScene = new BossScene(this);
        return currentScene->Init();
        break;

    case MENU:
        if (caller == PAUSE)  // if you come from pause scene, start the stage from the beginning
        { 
            if (previousScene) 
            {
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
            currentScene = new HowToPlayScene(this, TITLE); // for default in case there is an error
        }
        return currentScene->Init();
        break;

    case PAUSE:
        if (caller == STAGE || caller == STAGE2 || caller == STAGE3 || caller == BOSS)  // add here more stages depending on how many there are 
        {
            // so you can preserve the gameplay scene for when you resume the game
            previousScene = oldScene;                
            originalPausedScene = caller;  
            currentScene = new PauseScene(this, previousScene, originalPausedScene);
            return currentScene->Init();
        }
        else if (caller == HOWTOPLAY) 
        {
            // if coming bak from howtoplay scene,you can continue from you were in the stage
            currentScene = new PauseScene(this, previousScene, originalPausedScene);
            return currentScene->Init();
        }
        else if (caller == STAGESELECT)
        {   
            // if coming bak from stageselect scene,you can continue from you were in the stage
            currentScene = new PauseScene(this, previousScene, originalPausedScene); 
            return currentScene->Init(); 
        }
        else 
        {
            // there is no gameplay. you still have the preserved scene
            if (previousScene) 
            {
                previousScene->Uninit();
                delete previousScene;
                previousScene = nullptr;
            }
            currentScene = new PauseScene(this, nullptr, STAGE); // resume target unused
            return currentScene->Init();
        }
        break;

    case STAGESELECT:
        if (caller == PAUSE) // if in pause
        {
            currentScene = new StageSelect(this, PAUSE); // go back to pause
        }
        else if (caller == MENU) // if in menu
        {
            currentScene = new StageSelect(this, MENU);// go back to menu
        }
        else
        {
            currentScene = new StageSelect(this, TITLE); // for default in case there is an error
        }
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

    RendererDrawB();
}

//for erasing
void SceneManager::Uninit() 
{

    // for the current scene
    if (currentScene) 
    {
        currentScene->Uninit();
        delete currentScene;
        currentScene = nullptr;
    }

    // for the previous scene
    if (previousScene) 
    {
        previousScene->Uninit();
        delete previousScene;
        previousScene = nullptr;
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
