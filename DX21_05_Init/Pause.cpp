#include "Pause.h"
#include "Audio.h"

// コンストラクタ
PauseScene::PauseScene(SceneManager* manager, SceneBase* stage, SCENE PAUSE)
{
    sceneManager = manager;
    underlyingScene = stage; // StageScene へのポインタを保持する
    pausedSceneType = PAUSE;
}

bool PauseScene::Init()
{
	SetInGameCursorEnabled(true);
    LoadTexture(g_pDevice, "asset/UI/pause/background.png", &g_pauseTexture); // ポーズ背景テクスチャを読み込む

    LoadTexture(g_pDevice, "asset/UI/pause/pause_black.png", &blackTexture); // ポーズ中にステージを半透明で見せるため
    
        // 続行ボタン用
    LoadTexture(g_pDevice, "asset/UI/pause/continue_normal.png", &continueTexture);
    LoadTexture(g_pDevice, "asset/UI/pause/continue_hover.png", &continueHoverTexture);

        // 操作説明ボタン用
        LoadTexture(g_pDevice, "asset/UI/pause/control_normal.png", &controlTexture); // ボタン用
    LoadTexture(g_pDevice, "asset/UI/pause/control_hover.png", &controlHoverTexture);

        // ステージ選択ボタン用
        LoadTexture(g_pDevice, "asset/UI/pause/select_normal.png", &selectTexture); // ボタン用
    LoadTexture(g_pDevice, "asset/UI/pause/select_hover.png", &selectHoverTexture);

        // 終了ボタン用
        LoadTexture(g_pDevice, "asset/UI/pause/quit_normal.png", &quitTexture); // ボタン用
    LoadTexture(g_pDevice, "asset/UI/pause/quit_hover.png", &quitHoverTexture);


        // 各ポーズボタンのサイズが異なるため、ヒットボックスも個別に調整する。
    uiButtons.emplace_back(0.3f, -0.3f, 1.0f, 1.5f, pausedSceneType, continueTexture, continueHoverTexture);
    uiButtons.back().SetHitboxScale(0.27f, 0.1f);
    uiButtons.back().SetHitboxOffset(-0.03f);

    uiButtons.emplace_back(0.34f, -0.53f, 1.0f, 1.5f, HOWTOPLAY, controlTexture, controlHoverTexture);
    uiButtons.back().SetHitboxScale(0.35f, 0.1f);
    uiButtons.back().SetHitboxOffset(-0.03f);
                                                        // 現在いるエリアに応じて StageSelect 1/2/3 に戻る。
    uiButtons.emplace_back(0.50f, -0.65f, 1.0f, 1.5f, sceneManager->GetStageSelectForCurrentStage()/*STAGESELECT*/, selectTexture, selectHoverTexture);
    uiButtons.back().SetHitboxScale(0.7f, 0.1f);
    uiButtons.back().SetHitboxOffset(-0.0f);

    uiButtons.emplace_back(0.375f, -0.88f, 1.0f, 1.5f, QUIT_GAME, quitTexture, quitHoverTexture);
    uiButtons.back().SetHitboxScale(0.38f, 0.1f);
    uiButtons.back().SetHitboxOffset(-0.03f);


    //uiButtons.clear();
	Audio::PlaySE(SoundEffect::PAUSE);

    return true;
}   

void PauseScene::Update(float deltaTime)
{
    // ゲームを再開する処理
    g_inputSystem.Update();

    for (auto& btn : uiButtons)
    {
        if (btn.Process() == UIButtonResult::Clicked)
        {
			Audio::PlaySE(SoundEffect::UI_CLICK);
            SCENE target = btn.GetTargetScene();

            if (target == QUIT_GAME)
            {
                PostQuitMessage(0);// ゲームを終了する
                return;
            }
            else
            {
				if (target == pausedSceneType)
				{
					Audio::PlaySE(SoundEffect::RESUME);

                    // ポーズ画面で continue を押したら BGM を再開する
                    std::string savedPath = sceneManager->GetSavedBGMPath();
                    if (!Audio::IsBGMPlaying() && !savedPath.empty())
                    {
                        Audio::PlayBGM(savedPath, true);
                    }
                    else
                    {
                        Audio::ResumeBGM();
                    }
                    sceneManager->ClearSavedBGMPath();
				}

                if (target == HOWTOPLAY || target == STAGESELECT || target == STAGESELECT2 || target == STAGESELECT3)
                {
                    sceneManager->SaveBGMPath(Audio::GetCurrentBGMPath());
                }

                sceneManager->SwitchScene(target);
                return;
            }
        }
    }
}

void PauseScene::Draw()
{
 
    if (underlyingScene) 
    {
        underlyingScene->Draw(); // 背景で止まったステージを描画する
    }

    // 背景のステージを半透明に見せるための描画
    SetColor(0.0f, 0.0f, 0.0f, 0.4f); // blackTexture を半透明にする
    RenderImage(-1, -1, 2, 2, blackTexture, 0, 1, 1);
    SetColor(1, 1, 1, 1); // 描画色を通常に戻す

    if (g_pauseTexture) 
    {
        // ポーズ背景を半透明で描画する
        //SetColor(1, 1, 1, 0.55f);
        RenderImage(-1, -1, 2, 2, g_pauseTexture, 0, 1, 1);
        // UI 要素用に不透明な色へ戻す
        SetColor(1, 1, 1, 1.0f);
    }


    // ボタン描画
    for (const auto& btn : uiButtons)
        btn.Draw(0.65f);
   
}

void PauseScene::Uninit()
{

    if (g_pauseTexture) {
        g_pauseTexture->Release();
        g_pauseTexture = nullptr;
    }

    // 黒背景テクスチャ用
    if (blackTexture) {
        blackTexture->Release();
        blackTexture = nullptr;
    }

    // 続行ボタン用
    if (continueTexture)
    {
        continueTexture->Release();
        continueTexture = nullptr;
    }
    if (continueHoverTexture)
    {
        continueHoverTexture->Release();
        continueHoverTexture = nullptr;
    }

    // 操作説明ボタン用
    if (controlTexture)
    {
        controlTexture->Release();
        controlTexture = nullptr;
    }
    if (controlHoverTexture)
    {
        controlHoverTexture->Release();
        controlHoverTexture = nullptr;
    }

    // ステージ選択ボタン用
    if (selectTexture)
    {
        selectTexture->Release();
        selectTexture = nullptr;
    }
    if (selectHoverTexture)
    {
        selectHoverTexture->Release();
        selectHoverTexture = nullptr;
    }

    // 終了ボタン用
    if (quitTexture)
    {
        quitTexture->Release();
        quitTexture = nullptr;
    }
    if (quitHoverTexture)
    {
        quitHoverTexture->Release();
        quitHoverTexture = nullptr;
    }


    uiButtons.clear();
    g_mouseIndicator.ShowMouseIndicator(true);

    underlyingScene = nullptr;
}
