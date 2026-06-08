#pragma once
#include "SceneBase.h"
#include "StageInfo.h"

enum SCENE 
{
    TITLE,
    MENU,
    GAMEPLAY, // ボスエリアを含むすべてのエリア（全 8 エリア）用
    CAKE,
    RESULT,
    PAUSE,
    HOWTOPLAY,
    STAGESELECT,   // stage1（world 1）用
    STAGESELECT2,  // stage2（world 2）用
    STAGESELECT3,  // stage3（world 3）用
    QUIT_GAME = -1

    // 必要なら後で追加する
};

class SceneManager 
{
private:
    SceneBase* currentScene = nullptr; // 現在のシーン。読み込まれるまでは空のまま
    SCENE currentSceneType = GAMEPLAY;

    SceneBase* previousScene = nullptr;
    SCENE previousSceneType = GAMEPLAY;

    SCENE originalPausedScene = GAMEPLAY;

    bool comingFromStageSelect = false;

    StageInfo currentStageInfo; // 新しいテスト用

    SCENE stageSelectOrigin = TITLE;

    std::string savedBGMPath = "";

public:
    bool Init(SCENE startScene);
    void Update(float deltaTime);
    void Draw();
    void Uninit();
    bool SwitchScene(SCENE newScene);

    bool SwitchToStage(int world, int stage); // 新しいテスト用

    // ゲームループ
    void GameLoop();

    // 直前のシーンにアクセスするための補助関数
    SCENE GetCurrentSceneType() const { return currentSceneType; }
    SceneBase* GetPreviousScene() const { return previousScene; }
    SCENE GetPreviousSceneType() const { return previousSceneType; }

    SCENE GetOriginalPausedScene() const { return originalPausedScene; }
    void SetOriginalPausedScene(SCENE scene) { originalPausedScene = scene; }

    StageInfo GetCurrentStageInfo() const { return currentStageInfo; } // 新しいテスト用
    SCENE GetStageSelectForCurrentStage();

    void SaveBGMPath(const std::string& path) { savedBGMPath = path; }
    std::string GetSavedBGMPath() const { return savedBGMPath; }
    void ClearSavedBGMPath() { savedBGMPath = ""; }
};