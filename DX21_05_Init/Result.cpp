// ================================
//            RESULT
// ================================

#include "Result.h"
#include "SceneManager.h" // シーン切り替え用

// コンストラクタ
ResultScene::ResultScene(SceneManager* manager, int completedWorld)
{
    sceneManager = manager;
    m_completedWorld = completedWorld;
    m_nextScene = MENU; // 既定値は MENU
}

// オブジェクトを初期化する
bool ResultScene::Init()
{
    Audio::PlayBGM(BackgroundMusic::VICTORY, true);

    SetInGameCursorEnabled(true);

    LoadTexture(g_pDevice, "asset/UI/result/background.png", &backgroundTexture);
    LoadTexture(g_pDevice, "asset/UI/result/normal_score.png", &normalScoreTexture);

    LoadTexture(g_pDevice, "asset/UI/number.png", &numberTexture);
    LoadTexture(g_pDevice, "asset/UI/dot.png", &dotTexture);

    LoadTexture(g_pDevice, "asset/UI/result/title_normal.png", &titleTexture); // ボタン用
    LoadTexture(g_pDevice, "asset/UI/result/title_hover.png", &titleHoverTexture);
    uiButtons.emplace_back(-0.3f, -0.8f, 0.6f, 1.0f, TITLE, titleTexture, titleHoverTexture);
    uiButtons.back().SetHitboxScale(0.45f, 0.1f);  // ボタンサイズに応じて必要なら調整する
    uiButtons.back().SetHitboxOffset(0.02f);

    LoadTexture(g_pDevice, "asset/UI/result/next_normal.png", &continueTexture); // ボタン用
    LoadTexture(g_pDevice, "asset/UI/result/next_hover.png", &continueHoverTexture);
    uiButtons.emplace_back(0.3f, -0.8f, 0.6f, 1.0f, m_nextScene/*MENU*/, continueTexture, continueHoverTexture); // todo: 次のワールドができたら MENU を変更する
    uiButtons.back().SetHitboxScale(0.63f, 0.1f);  // ボタン 3 のサイズに応じて必要なら調整する
    uiButtons.back().SetHitboxOffset(0.02f);

    return true;
}

// オブジェクトを更新する
void ResultScene::Update(float deltaTime)
{
    g_inputSystem.Update();

    // ボタン処理
    for (auto& btn : uiButtons)
    {
        if (btn.Process() == UIButtonResult::Clicked)
        {
            SCENE targetScene = btn.GetTargetScene();

            // Title を押したらタイトル画面へ移動する
            if (targetScene == TITLE)
            {
                sceneManager->SwitchScene(TITLE);
                Audio::StopBGM();
            }
            // Next を押したら次のステージ選択へ移動する
            else
            {
                if (m_completedWorld == 1)
                {
                    sceneManager->SwitchScene(STAGESELECT2);
                    Audio::StopBGM();
                }
                else if (m_completedWorld == 2)
                {
                    sceneManager->SwitchScene(STAGESELECT3);
                    Audio::StopBGM();
                }
                else if (m_completedWorld == 3)
                {
                    sceneManager->SwitchScene(MENU);
                    Audio::StopBGM();
                }
            }
            return;
        }
    }
}

// オブジェクトを描画する
void ResultScene::Draw()
{
    if (backgroundTexture) {
        // テクスチャが見えるように描画前に必ず色を設定する
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, backgroundTexture, 0, 1, 1);
    }

    if (normalScoreTexture) {
        // テクスチャが見えるように描画前に必ず色を設定する
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-0.7f, -1.1f, 1.3f, 1.5f, normalScoreTexture, 0, 1, 1);
    }

    // 統計情報の数値を描画する
    if (numberTexture) {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);

        // 撃破数
        int totalKills = g_gameStats.GetEnemiesKilled() + g_gameStats.GetWeakPointKills();
        DrawNumber(totalKills, -0.55f, 0.47f, 0.1f, 0.1f, numberTexture);

        // 死亡数
        DrawNumber(g_gameStats.GetTotalDeaths(), -0.55f, 0.07f, 0.1f, 0.1f, numberTexture);

        // 経過時間
        int minutes = (int)(g_gameStats.GetTotalTime() / 60.0f);
        int seconds = (int)g_gameStats.GetTotalTime() % 60;
        DrawTime(minutes, seconds, -0.55f, 0.27f, 0.1f, numberTexture);
        RenderImage(-0.4f, 0.27f, 0.1f, 0.16f, dotTexture, 0, 1, 1);

        // 最終計算スコアを表示する
        DrawNumber(g_gameStats.GetTotalScore(), 0.3f, 0.07f, 0.1f, 0.1f, numberTexture);
    }

    for (const auto& btn : uiButtons)
        btn.Draw(0.65f);
}


void DrawNumber(int number, float x, float y, float width, float height, ID3D11ShaderResourceView* texture) {
    // 数字が 0 の場合はそのまま 0 を描画する
    if (number == 0) {
        RenderImage(x, y, width, height, texture, 0, 1, 10, false, 0.0f, false);
        return;
    }

    std::vector<int> digits;
    int temp = number;
    while (temp > 0) {
        digits.push_back(temp % 10);  // 末尾の桁を取得する
        temp /= 10;                    // 末尾の桁を取り除く
    }
    std::reverse(digits.begin(), digits.end()); // 描画順にするため反転する

    // 各桁を左から右へ描画する
    float digitX = x;  // X 座標の開始位置
    for (int digit : digits) {
        // 現在位置に桁を描画する
        RenderImage(digitX, y, width, height, texture, digit, 1, 10, false, 0.0f, false);
        // 次の桁のために X 座標を右へ進める
        digitX += width * 0.7f; // 少し隙間が空くようにする
    }
}

void DrawTime(int minutes, int seconds, float x, float y, float size, ID3D11ShaderResourceView* texture) {
    float digitX = x; // 開始位置

    // 分の 1 桁目
    RenderImage(digitX, y, size, size, texture, minutes / 10, 1, 10, false, 0.0f, false);
    digitX += size * 0.7f; // 少し隙間が空くようにする
    // 分の 2 桁目
    RenderImage(digitX, y, size, size, texture, minutes % 10, 1, 10, false, 0.0f, false);
    digitX += size * 1.2f; // コロン分の余白を少し広めに取る

    // 秒の 1 桁目
    RenderImage(digitX, y, size, size, texture, seconds / 10, 1, 10, false, 0.0f, false);
    digitX += size * 0.7f; // 少し隙間が空くようにする
    // 秒の 2 桁目
    RenderImage(digitX, y, size, size, texture, seconds % 10, 1, 10, false, 0.0f, false);
}


// オブジェクトを解放する
void ResultScene::Uninit()
{
    ReleaseTexture(backgroundTexture);

    ReleaseTexture(normalScoreTexture);

    ReleaseTexture(numberTexture);

    ReleaseTexture(titleTexture);
    ReleaseTexture(titleHoverTexture);

    ReleaseTexture(continueTexture);
    ReleaseTexture(continueHoverTexture);

    ReleaseTexture(dotTexture);

    uiButtons.clear();
    g_mouseIndicator.Cleanup();
}