#include "StageSelect2.h"


StageSelect2::StageSelect2(SceneManager* manager, SCENE returnTo)
{
    backgroundTexture = nullptr;
    sceneManager = manager;
    returnScene = returnTo;
}


bool StageSelect2::Init()
{
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_background.png", &backgroundTexture);      // 背景テクスチャ

    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_stage_nomalsize.png", &buttonTexture); // ボタン用
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_stage_bigsize.png", &buttonHoverTexture);
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_Boss_nomalsize.png", &bossButtonTexture);
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_Boss_bigsize.png", &bossButtonHoverTexture);

    // 次のステージ選択画面へ移動する矢印用
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_next_normalsize.png", &arrowTexture);
    LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_next_bigsize.png", &arrowHoverTexture);

    // 上段
    uiButtons.emplace_back(-0.65f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // 2-1 へ移動
    uiButtons.emplace_back(-0.3f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // 2-3 へ移動
    uiButtons.emplace_back(0.06f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // 2-5 へ移動
    uiButtons.emplace_back(0.43f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // 2-7 へ移動

    // 下段
    uiButtons.emplace_back(-0.5f, -0.33f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // 2-2 へ移動
    uiButtons.emplace_back(-0.12f, -0.33f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // 2-4 へ移動
    uiButtons.emplace_back(0.26f, -0.33f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // 2-6 へ移動
    uiButtons.emplace_back(0.63f, -0.33f, 0.4f, 0.8f, GAMEPLAY, bossButtonTexture, bossButtonHoverTexture);  // ボスへ移動
    for (auto& btn : uiButtons)
    {
        btn.SetHitboxScale(0.7f, 0.2f);
        btn.SetHitboxOffset(-0.05f);
    }

    // world 1 のステージ選択へ戻る左矢印
    uiButtons.emplace_back(-0.905f, 0.0f, 0.4f, 0.6f, STAGESELECT, arrowTexture, arrowHoverTexture);
    uiButtons.back().SetHitboxScale(0.4f, 0.6f);
    uiButtons.back().SetHitboxOffset(-0.04f);
    uiButtons.back().SetRotation(-180.0f); // 左向きになるよう回転する

    // world 3 のステージ選択へ進む右矢印
    uiButtons.emplace_back(0.9f, 0.0f, 0.4f, 0.6f, STAGESELECT3, arrowTexture, arrowHoverTexture);
    uiButtons.back().SetHitboxScale(0.4f, 0.6f);
    uiButtons.back().SetHitboxOffset(-0.04f);

    LoadTexture(g_pDevice, "asset/UI/back/back_normal.png", &backTexture); // ボタン用
    LoadTexture(g_pDevice, "asset/UI/back/back_hover.png", &backHoverTexture);

    uiButtons.emplace_back(0.8f, -0.9f, 0.4f, 0.8f, returnScene, backTexture, backHoverTexture);
    uiButtons.back().SetHitboxScale(0.25f, 0.13f);  // ボタンサイズに応じて必要なら調整する
    uiButtons.back().SetHitboxOffset(-0.06f);

    return true;
}

// 更新処理
void StageSelect2::Update(float deltaTime)
{
    g_inputSystem.Update();

    // 各ボタンのクリックを確認する
    for (int i = 0; i < uiButtons.size(); i++)
    {
  
        if (uiButtons[i].Process() == UIButtonResult::Clicked)
        {
            // エリア（ステージ）ボタン
            if (i >= 0 && i < 8)
            {
                // ボタン番号をステージ番号へ対応付ける
                int stageNumbers[8] = { 1, 3, 5, 7, 2, 4, 6, 8 }; // 8 はボス
                sceneManager->SwitchToStage(2, stageNumbers[i]);
                return;
            }
            // world1 の StageSelect へ戻る左矢印
            if (i == 8) // エリア（ステージ）追加後も i = 8 のまま
            {
                sceneManager->SwitchScene(STAGESELECT);
                return;
            }
            // world3 の StageSelect へ進む右矢印
            else if (i == 9) // エリア（ステージ）追加後も i = 9 のまま
            {
                sceneManager->SwitchScene(STAGESELECT3);
                return;
            }
            // 戻るボタン
            else
            {
                sceneManager->SwitchScene(returnScene);
                return;
            }
        }
  
    }
}

// ステージ選択画面を描画する
void StageSelect2::Draw()
{
    // 背景を描画する
    if (backgroundTexture)
    {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, backgroundTexture, 0, 1, 1);
    }

    // すべてのボタンを描画する
    for (const auto& btn : uiButtons)
        btn.Draw(0.65f);


    if (g_numberTexture)
    {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);

        float numberWidth = 0.03f;
        float numberHeight = 0.05f;
        float spaceBetweenNumbers = 0.045f;     // 数字間の間隔

        // 上段
        DrawAreaNumber(2, 1, -0.693f, 0.155f, numberWidth, numberHeight, spaceBetweenNumbers);     // 2-1
        DrawAreaNumber(2, 3, -0.343f, 0.155f, numberWidth, numberHeight, spaceBetweenNumbers);    // 2-3
        DrawAreaNumber(2, 5, 0.018f, 0.155f, numberWidth, numberHeight, spaceBetweenNumbers);    // 2-5
        DrawAreaNumber(2, 7, 0.39f,  0.155f, numberWidth, numberHeight, spaceBetweenNumbers);     // 2-7

        // 下段
        DrawAreaNumber(2, 2, -0.54f, -0.325f, numberWidth, numberHeight, spaceBetweenNumbers);   // 2-2
        DrawAreaNumber(2, 4, -0.16f, -0.325f, numberWidth, numberHeight, spaceBetweenNumbers);   // 2-4
        DrawAreaNumber(2, 6, 0.22f, -0.325f, numberWidth, numberHeight, spaceBetweenNumbers);    // 2-6
    }
}

void StageSelect2::DrawAreaNumber(int world, int stage, float x, float y, float width, float height, float space)
{
    // ワールド番号（1 桁目）を描画する
    RenderImage(x, y, width, height, g_numberTexture, world, 1, 10, false, 0.0f, false);

    // 間隔を空けてステージ番号（2 桁目: 1-8）を描画する
    RenderImage(x + space, y, width, height, g_numberTexture, stage, 1, 10, false, 0.0f, false);
}

// 解放処理
void StageSelect2::Uninit()
{
    if (backgroundTexture)
    {
        backgroundTexture->Release();
        backgroundTexture = nullptr;
    }
    if (buttonTexture)
    {
        buttonTexture->Release();
        buttonTexture = nullptr;
    }
    if (buttonHoverTexture)
    {
        buttonHoverTexture->Release();
        buttonHoverTexture = nullptr;
    }
    if (bossButtonTexture)
    {
        bossButtonTexture->Release();
        bossButtonTexture = nullptr;
    }
    if (bossButtonHoverTexture)
    {
        bossButtonHoverTexture->Release();
        bossButtonHoverTexture = nullptr;
    }

    if (backTexture)
    {
        backTexture->Release();
        backTexture = nullptr;
    }
    if (backHoverTexture)
    {
        backHoverTexture->Release();
        backHoverTexture = nullptr;
    }


    if (arrowTexture)
    {
        arrowTexture->Release();
        arrowTexture = nullptr;
    }
    if (arrowHoverTexture)
    {
        arrowHoverTexture->Release();
        arrowHoverTexture = nullptr;
    }

    // ボタンをクリアする
    uiButtons.clear();
    g_mouseIndicator.Cleanup();
}