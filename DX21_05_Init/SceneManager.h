#pragma once
#include "SceneBase.h"

enum SCENE 
{
    TITLE,
    MENU,
    STAGE,

    // ADD MORE HERE LATER
};

class SceneManager 
{
private:
    SceneBase* currentScene = nullptr; // this is for for the current scene. It starts empty until a scene is loaded
    SCENE currentSceneType;

public:
    bool Init(SCENE startScene);
    void Update(float deltaTime);
    void Draw();
    void Uninit();
    bool SwitchScene(SCENE newScene);

    // Game loop
    void GameLoop();
};