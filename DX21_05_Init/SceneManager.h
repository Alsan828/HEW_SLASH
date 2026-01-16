#pragma once
#include "SceneBase.h"
#include "StageInfo.h"

enum SCENE 
{
    TITLE,
    MENU,
    GAMEPLAY, // this is for all the areas (8 areas) I have including the boss area 
    CAKE,
    RESULT,
    PAUSE,
    HOWTOPLAY,
    STAGESELECT,   // for stage1 (world 1)
    STAGESELECT2,  // for stage2 (world 2)
    STAGESELECT3,  // for stage3 (world 3)
    QUIT_GAME = -1

    // ADD MORE HERE LATER IF NEEDED
};

class SceneManager 
{
private:
    SceneBase* currentScene = nullptr; // this is for for the current scene. It starts empty until a scene is loaded
    SCENE currentSceneType = GAMEPLAY;

    SceneBase* previousScene = nullptr;
    SCENE previousSceneType = GAMEPLAY;

    SCENE originalPausedScene = GAMEPLAY;

    bool comingFromStageSelect = false;

    StageInfo currentStageInfo; //new test

    SCENE stageSelectOrigin = TITLE;

public:
    bool Init(SCENE startScene);
    void Update(float deltaTime);
    void Draw();
    void Uninit();
    bool SwitchScene(SCENE newScene);

    bool SwitchToStage(int world, int stage); //new test

    // Game loop
    void GameLoop();

    // helper to access previous scene
    SCENE GetCurrentSceneType() const { return currentSceneType; }
    SceneBase* GetPreviousScene() const { return previousScene; }
    SCENE GetPreviousSceneType() const { return previousSceneType; }

    SCENE GetOriginalPausedScene() const { return originalPausedScene; }
    void SetOriginalPausedScene(SCENE scene) { originalPausedScene = scene; }

    StageInfo GetCurrentStageInfo() const { return currentStageInfo; } //new test
};