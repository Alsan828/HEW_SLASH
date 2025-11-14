#include "SceneManager.h"
#include "TitleScene.h"
#include "Stage.h"

//for initializing it
bool SceneManager::Init(SCENE startScene) 
{
    return SwitchScene(startScene);
}

//for switching between scenes
bool SceneManager::SwitchScene(SCENE newScene) 
{
    if (currentScene) 
    {
        currentScene->Uninit();
        delete currentScene;
        currentScene = nullptr;
    }

    // this is to initialize the new scene
    currentSceneType = newScene;

    // Initialize the new scene
    switch (newScene) 
    {
    case TITLE:
        currentScene = new TitleScene(this); // for the title
        break;

    case STAGE:
        currentScene = new StageScene(this); // for the prototype stage
        break;


    default:
        return false;
    }

    return currentScene->Init();
}

//for updating 
void SceneManager::Update(float deltaTime) 
{
    if (currentScene)
    {
        currentScene->Update(deltaTime);
    }
}

//for drawing the objects
void SceneManager::Draw() 
{
    if (currentScene)
    {
        currentScene->Draw();
    }
}

//for erasing
void SceneManager::Uninit() 
{
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

    HandleInput();  // Handle keyboard input
    // Update game state
    Update(delta);
    Draw();
}