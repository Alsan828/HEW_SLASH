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

// 初期化処理
bool SceneManager::Init(SCENE startScene) 
{
    return SwitchScene(startScene);
}

// シーン切り替え処理
bool SceneManager::SwitchScene(SCENE newScene) 
{
    SceneBase* oldScene = currentScene;  // ポーズメニュー用に保持する
    SCENE caller = currentSceneType;     // 上書き前に呼び出し元のシーン種別を保存する
    
   
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
   
    // 新しいシーンを初期化するための準備
    currentSceneType = newScene;

    // 新しいシーンを初期化する
    switch (newScene) 
    {
    case TITLE:
        currentScene = new TitleScene(this); // タイトル用
        return currentScene->Init();
        break;

    case GAMEPLAY:  // ボスを含むすべてのエリア（ステージ）用
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
            // 新しいステージ開始時は BGM を最初から再生できるようにする
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

        // ステージ選択から来た場合はゲージをリセットする
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
        if (caller == PAUSE)  // ポーズ画面から来た場合はステージを最初から始める
        { 
            if (previousScene) 
            {
                previousScene->Uninit();
                delete previousScene;
                previousScene = nullptr;
            }
        }

        currentScene = new MenuScene(this); // メニュー用
        return currentScene->Init();
        break;

    case HOWTOPLAY:
        if (caller == PAUSE) // ポーズ中の場合
        { 
            currentScene = new HowToPlayScene(this, PAUSE); // ポーズへ戻る
        }
        else if (caller == MENU) // メニュー中の場合
        { 
            currentScene = new HowToPlayScene(this, MENU);// メニューへ戻る
        }
        else 
        {
            currentScene = new HowToPlayScene(this, TITLE); // エラー時の既定戻り先
        }
        return currentScene->Init();
        break;

    case PAUSE:
        if (caller == GAMEPLAY || caller == CAKE)
        {
            // 再開時のためにゲームプレイシーンを保持する
            previousScene = oldScene;                
            originalPausedScene = caller;  
            currentScene = new PauseScene(this, previousScene, originalPausedScene);
            return currentScene->Init();
        }
        else if (caller == HOWTOPLAY) 
        {
            // HowToPlay から戻る場合は元のステージの続きから再開する
            currentScene = new PauseScene(this, previousScene, originalPausedScene);
            return currentScene->Init();
        }
        else if (caller == STAGESELECT || caller == STAGESELECT2 || caller == STAGESELECT3)
        {   
            // StageSelect から戻る場合は元のステージの続きから再開する
            currentScene = new PauseScene(this, previousScene, originalPausedScene); 
            return currentScene->Init(); 
        }
        else 
        {
            // ゲームプレイ中でなくても保持済みシーンは確認する
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
    // If tutorial active, pause game time and let the current scene handle tutorial input
    if (g_tutorialActive) {
        // Update raw input so UI can react
        g_inputSystem.Update();
        // Call scene Update with zero delta so it can process tutorial UI without advancing timers
        if (currentScene) currentScene->Update(0.0f);
        Draw();
        return;
    }

    g_gameTimer.Tick();
    float delta = g_gameTimer.GetDeltaTime();

    // Update input
    g_inputSystem.Update();

    HandleInput();  // Handle keyboard input
    Update(delta);   // Update game state
    Draw();
}
