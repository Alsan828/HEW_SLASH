#include "Pause.h"
#include "Audio.h"

// construct
PauseScene::PauseScene(SceneManager* manager, SceneBase* stage, SCENE PAUSE)
{
    sceneManager = manager;
    underlyingScene = stage; // keep pointer to StageScene
    pausedSceneType = PAUSE;
}

bool PauseScene::Init()
{
	SetInGameCursorEnabled(true);
    LoadTexture(g_pDevice, "asset/UI/pause/background.png", &g_pauseTexture); // Load the pause background texture

    LoadTexture(g_pDevice, "asset/UI/pause/pause_black.png", &blackTexture); // so when in pause, I see the stage part transparent
    
    // for continue button
    LoadTexture(g_pDevice, "asset/UI/pause/continue_normal.png", &continueTexture);
    LoadTexture(g_pDevice, "asset/UI/pause/continue_hover.png", &continueHoverTexture);

    // for control button
    LoadTexture(g_pDevice, "asset/UI/pause/control_normal.png", &controlTexture); // for the button
    LoadTexture(g_pDevice, "asset/UI/pause/control_hover.png", &controlHoverTexture);

    // for select stage button
    LoadTexture(g_pDevice, "asset/UI/pause/select_normal.png", &selectTexture); // for the button
    LoadTexture(g_pDevice, "asset/UI/pause/select_hover.png", &selectHoverTexture);

    // for quit button
    LoadTexture(g_pDevice, "asset/UI/pause/quit_normal.png", &quitTexture); // for the button
    LoadTexture(g_pDevice, "asset/UI/pause/quit_hover.png", &quitHoverTexture);


    // because each puase button size is different so the hitbox will be different as well.
    uiButtons.emplace_back(0.3f, -0.3f, 1.0f, 1.5f, pausedSceneType, continueTexture, continueHoverTexture);
    uiButtons.back().SetHitboxScale(0.27f, 0.1f);
    uiButtons.back().SetHitboxOffset(-0.03f);

    uiButtons.emplace_back(0.34f, -0.53f, 1.0f, 1.5f, HOWTOPLAY, controlTexture, controlHoverTexture);
    uiButtons.back().SetHitboxScale(0.35f, 0.1f);
    uiButtons.back().SetHitboxOffset(-0.03f);
                                                     // so I go to stage select 1,2 or 3 depending on the area I was at.
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
    // resume game
    g_inputSystem.Update();

    for (auto& btn : uiButtons)
    {
        if (btn.Process() == UIButtonResult::Clicked)
        {
			Audio::PlaySE(SoundEffect::UI_CLICK);
            SCENE target = btn.GetTargetScene();

            if (target == QUIT_GAME)
            {
                PostQuitMessage(0);// it quits the game
                return;
            }
            else
            {
				if (target == pausedSceneType)
				{
					Audio::PlaySE(SoundEffect::RESUME);
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
        underlyingScene->Draw(); // draw the stage frozen so I can see it in the background
    }

    // this is for the back (the stage) so when you pause it, the stage part looks transparent
    SetColor(0.0f, 0.0f, 0.0f, 0.4f); // makes the blackTexture transparent
    RenderImage(-1, -1, 2, 2, blackTexture, 0, 1, 1);
    SetColor(1, 1, 1, 1); // resets the blackTexture to normal

    if (g_pauseTexture) 
    {
        SetColor(1, 1, 1, 1.0f);
        RenderImage(-1, -1, 2, 2, g_pauseTexture, 0, 1, 1);
    }


    // for the buttons
    for (const auto& btn : uiButtons)
        btn.Draw(0.65f);
   
}

void PauseScene::Uninit()
{

    if (g_pauseTexture) {
        g_pauseTexture->Release();
        g_pauseTexture = nullptr;
    }

    // for the black texture
    if (blackTexture) {
        blackTexture->Release();
        blackTexture = nullptr;
    }

    // for continue button
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

    // for control button
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

    // for select stage button
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

    // for quit button
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
