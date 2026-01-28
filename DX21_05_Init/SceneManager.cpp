#include "SceneManager.h"
#include "TitleScene.h"

#include "GameplayScene.h" 
#include "CakeScene.h"
#include "Menu.h"
#include "HowToPlay.h"
#include "Pause.h"
#include "StageSelect.h"
#include "StageSelect2.h"
#include "StageSelect3.h"
#include "Result.h"

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
        if (!(currentSceneType == GAMEPLAY || currentSceneType == CAKE)
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

    case GAMEPLAY:  // for all the areas (stages) including the boss
        if (caller == PAUSE && previousScene)
        {
            currentScene = previousScene;
            previousScene = nullptr;
            comingFromStageSelect = false;
            return true;
        }

        if (previousScene)
        {
            previousScene->Uninit();
            delete previousScene;
            previousScene = nullptr;
        }

        if (comingFromStageSelect)
        {
            g_player.gaugePoints = 0;
            g_player.isInvincible = false;
            g_player.invincibleTimer = 0.0f;
            g_gameStats.Reset();
            g_gameElapsedTime = 0.0f;
            g_gameMinutes = 0;
            g_gameSeconds = 0;
            // so I can play the bgm from the beginning if I start a new stage
            Audio::StopBGM();
            ClearSavedBGMPath();

            comingFromStageSelect = false;
        }

        currentScene = new GameplayScene(this, currentStageInfo.GetWorld(), currentStageInfo.GetArea());
        return currentScene->Init();
        break;

    case CAKE:
        if (caller == PAUSE && previousScene)
        {
            currentScene = previousScene;     // resume existing stage 
            previousScene = nullptr;
            comingFromStageSelect = false;
            return true;
        }

        if (previousScene)
        {
            previousScene->Uninit();
            delete previousScene;
            previousScene = nullptr;
        }

        // Reset gauge bar if coming from stage select
        if (comingFromStageSelect)
        {
            g_player.gaugePoints = 0;
            g_player.isInvincible = false;
            g_player.invincibleTimer = 0.0f;
            g_gameStats.Reset();
            g_gameElapsedTime = 0.0f;
            g_gameMinutes = 0;
            g_gameSeconds = 0;
            comingFromStageSelect = false;
        }
        currentScene = new CakeScene(this);
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
        if (caller == GAMEPLAY || caller == CAKE)
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
        else if (caller == STAGESELECT || caller == STAGESELECT2 || caller == STAGESELECT3)
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
            currentScene = new PauseScene(this, nullptr, GAMEPLAY); // resume target unused
            return currentScene->Init();
        }
        break;

    case STAGESELECT:
        comingFromStageSelect = true;

        // Only update origin if we are entering the stage-select system from outside
        if (caller != STAGESELECT && caller != STAGESELECT2 && caller != STAGESELECT3)
        {
            stageSelectOrigin = caller;
        }

        currentScene = new StageSelect(this, stageSelectOrigin);
        return currentScene->Init();
        break;

    case STAGESELECT2:
        comingFromStageSelect = true;

        if (caller != STAGESELECT && caller != STAGESELECT2 && caller != STAGESELECT3)
        {
            stageSelectOrigin = caller;
        }

        currentScene = new StageSelect2(this, stageSelectOrigin);
        return currentScene->Init();
        break;

    case STAGESELECT3:
        comingFromStageSelect = true;

        if (caller != STAGESELECT && caller != STAGESELECT2 && caller != STAGESELECT3)
        {
            stageSelectOrigin = caller;
        }

        currentScene = new StageSelect3(this, stageSelectOrigin);
        return currentScene->Init();
        break;

    case RESULT:
        currentScene = new ResultScene(this, currentStageInfo.GetWorld());
       
        return currentScene->Init();
        break;

    default:
        return false;
    }

}


//new test
bool SceneManager::SwitchToStage(int world, int stage)
{
    currentStageInfo = StageInfo(world, stage);
    return SwitchScene(GAMEPLAY);
}

SCENE SceneManager::GetStageSelectForCurrentStage()
{
    int stage = currentStageInfo.GetWorld();

    if (stage == 1)
    {
        return STAGESELECT;
    }
    else if (stage == 2)
    {
        return STAGESELECT2;
    }
    else if (stage == 3)
    {
        return STAGESELECT3;
    }
    else
    {
        return STAGESELECT;  // default to stage1
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
