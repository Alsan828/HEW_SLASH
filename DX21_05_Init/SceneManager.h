#pragma once
#include "SceneBase.h"

enum SCENE 
{
    TITLE,
    MENU,
    STAGE,
    PAUSE,
    HOWTOPLAY,
    QUIT_GAME = -1

    // ADD MORE HERE LATER
};

class SceneManager 
{
private:
    SceneBase* currentScene = nullptr; // this is for for the current scene. It starts empty until a scene is loaded
    SCENE currentSceneType;

    SceneBase* previousScene = nullptr;

public:
    bool Init(SCENE startScene);
    void Update(float deltaTime);
    void Draw();
    void Uninit();
    bool SwitchScene(SCENE newScene);

    // Game loop
    void GameLoop();

    // helper to access previous scene
    SCENE GetCurrentSceneType() const { return currentSceneType; }
    SceneBase* GetPreviousScene() const { return previousScene; }
};