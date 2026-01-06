#pragma once
#include "SceneBase.h"

enum SCENE 
{
    TITLE,
    MENU,
    STAGE,
    STAGE2,
    STAGE3,
    BOSS,
    CAKE,
    RESULT,
    PAUSE,
    HOWTOPLAY,
    STAGESELECT,
    QUIT_GAME = -1

    // ADD MORE HERE LATER
};

class SceneManager 
{
private:
    SceneBase* currentScene = nullptr; // this is for for the current scene. It starts empty until a scene is loaded
    SCENE currentSceneType = STAGE;

    SceneBase* previousScene = nullptr;
    SCENE previousSceneType = STAGE;

    SCENE originalPausedScene = STAGE;

    bool comingFromStageSelect = false;

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
    SCENE GetPreviousSceneType() const { return previousSceneType; }

    SCENE GetOriginalPausedScene() const { return originalPausedScene; }
    void SetOriginalPausedScene(SCENE scene) { originalPausedScene = scene; }
};