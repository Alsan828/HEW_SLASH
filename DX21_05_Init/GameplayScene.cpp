#include "GameplayScene.h"
#include "Game.h"
#include "Enemy.h"
#include "Audio.h"

// コンストラクタ
GameplayScene::GameplayScene(SceneManager* manager, int world, int area)
{
    sceneManager = manager;
    worldNumber = world;        // ワールド番号（1、2、3）
    areaNumber = area;          // エリア番号（1～8）
    isBossStage = (area == 8);  // エリア 8 はボスステージ

    // ボス用初期化
    m_boss = nullptr;
    m_bossCheckpointHP = 0.0f;
    m_checkpoint1Reached = false;
    m_checkpoint2Reached = false;
}

bool GameplayScene::Init()
{
    ShowCursor(FALSE);
    g_gameState = STATE_PLAYING;

    // ワールドとステージ番号からマップ名を生成する
    StageInfo info(worldNumber, areaNumber);
    std::string mapName = info.GetMapName();

    g_mapManager.SwitchMap(mapName, -1, -1);

    // ボスステージではボスのインスタンスや状態をきれいに作り直すため、
    // フルリロードが必要になる。通常エリアではソフトリセットにして、
    // マップ遷移時もゲージやパーティクルなどの一時状態を保持する。
    if (isBossStage) {
        // ボスステージへ入るたびに、シーン内のチェックポイント追跡を初期化する。
        m_bossCheckpointHP = 0.0f;
        m_checkpoint1Reached = false;
        m_checkpoint2Reached = false;
        ResetGame(true);
    }
    else {
        ResetGame();
    }

    LoadTexture(g_pDevice, "asset/UI/cake/nextbutton_normal.png", &m_tutorialButtonTexture);
    LoadTexture(g_pDevice, "asset/UI/cake/nextbutton_hover.png", &m_tutorialButtonHoverTexture);
    m_tutorialButton = UIButton(0.7f, -0.85f, 0.35f, 0.7f, MENU, m_tutorialButtonTexture, m_tutorialButtonHoverTexture);
    m_tutorialButton.SetHitboxScale(0.25f, 0.13f);
    m_tutorialButton.SetHitboxOffset(-0.06f);

    // チュートリアルの表示状態をリセットする
    for (int i = 0; i < 4; i++) {
        m_tutorialTriggered[i] = false;
    }

    m_showTutorial = false;
    g_tutorialActive = false;

    if (isBossStage)
    {
        Audio::PlayBGM(BackgroundMusic::BOSS_BATTLE, true);
    }
    else
    {
        switch (worldNumber)
        {
        case 1: Audio::PlayBGM(BackgroundMusic::LEVEL1, true); break;
        case 2: Audio::PlayBGM(BackgroundMusic::LEVEL2, true); break;
        case 3: Audio::PlayBGM(BackgroundMusic::LEVEL3, true); break;
        default: Audio::PlayBGM(BackgroundMusic::LEVEL1, true); break;
        }
    }

    // ボスステージならボス情報を設定する
    if (isBossStage)
    {
        m_boss = nullptr;

        // 現在の敵リストから BossEnemy を探す
        for (auto* e : g_enemies) {
            if (dynamic_cast<BossEnemy*>(e) != nullptr) {
                m_boss = e;
                break;
            }
        }

        if (m_boss) {
            // 各ボスクラスの調整済み最大 HP を尊重し、テスト用に全快で開始する。
            m_boss->SetHealth(m_boss->GetMaxHealth());
        }
    }

    return true;
}

// これはトリガーなしでチュートリアルを表示する旧実装
//bool GameplayScene::Init()
//{
//	ShowCursor(FALSE);
//    g_gameState = STATE_PLAYING;
//
//    // ワールドとステージ番号からマップ名を生成する
//    StageInfo info(worldNumber, areaNumber);
//    std::string mapName = info.GetMapName();
//
//    g_mapManager.SwitchMap(mapName, -1, -1);
//
//    // ボスステージではフルリロードし、通常エリアでは一時状態を保つためソフトリセットする。
//    if (isBossStage) {
//        // ボスステージ再入場時にシーン内チェックポイント追跡を初期化する。
//        m_bossCheckpointHP = 0.0f;
//        m_checkpoint1Reached = false;
//        m_checkpoint2Reached = false;
//        ResetGame(true);
//    }
//    else {
//        ResetGame();
//    }
//
//    // World1 の最初の 4 エリア用チュートリアルオーバーレイを読み込む
//    if (worldNumber == 1 && areaNumber >= 1 && areaNumber <= 4) {
//        m_showTutorial = true;
//        g_tutorialActive = true;
//        std::string path = "asset/tutorial/tutorial_" + std::to_string(areaNumber) + ".png";
//        LoadTexture(g_pDevice, path.c_str(), &m_tutorialTexture);
//        LoadTexture(g_pDevice, "asset/UI/cake/nextbutton_normal.png", &m_tutorialButtonTexture);
//        LoadTexture(g_pDevice, "asset/UI/cake/nextbutton_hover.png", &m_tutorialButtonHoverTexture);
//        // ボタンを右下に配置する
//        m_tutorialButton = UIButton(0.7f, -0.85f, 0.35f, 0.7f, MENU, m_tutorialButtonTexture, m_tutorialButtonHoverTexture);
//        m_tutorialButton.SetHitboxScale(0.25f, 0.13f);
//        m_tutorialButton.SetHitboxOffset(-0.06f);
//    }
//
//	if (isBossStage)
//	{
//		Audio::PlayBGM(BackgroundMusic::BOSS_BATTLE, true);
//	}
//	else
//	{
//		switch (worldNumber)
//		{
//		case 1: Audio::PlayBGM(BackgroundMusic::LEVEL1, true); break;
//		case 2: Audio::PlayBGM(BackgroundMusic::LEVEL2, true); break;
//		case 3: Audio::PlayBGM(BackgroundMusic::LEVEL3, true); break;
//		default: Audio::PlayBGM(BackgroundMusic::LEVEL1, true); break;
//		}
//	}
//
//    // ボスステージならボスを設定する
//    if (isBossStage) 
//    {
//        m_boss = nullptr;
//
//        // 現在の敵リストから BossEnemy を探す
//        for (auto* e : g_enemies) {
//            if (dynamic_cast<BossEnemy*>(e) != nullptr) {
//                m_boss = e;
//                break;
//            }
//        }
//
//        if (m_boss) {
//            // 通常プレイ用にボス HP を 300 に設定する
//            m_boss->SetMaxHealth(300.0f);
//            m_boss->SetHealth(300.0f);
//        }
//    }
//
//    return true;
//}
// これはトリガーなしチュートリアル用の旧 Update 実装
//void GameplayScene::Update(float deltaTime)
//{
//    // チュートリアル表示中は入力だけ処理し、ゲーム時間は進めない
//    if (m_showTutorial) {
//        // 生入力だけは更新する
//        g_inputSystem.Update();
//
//        // チュートリアルボタンを処理する
//        if (m_tutorialButton.Process() == UIButtonResult::Clicked) {
//            // チュートリアルを閉じて通常プレイへ戻る
//            m_showTutorial = false;
//            g_tutorialActive = false;
//            // チュートリアル用テクスチャを解放する
//            if (m_tutorialTexture) { m_tutorialTexture->Release(); m_tutorialTexture = nullptr; }
//            if (m_tutorialButtonTexture) { m_tutorialButtonTexture->Release(); m_tutorialButtonTexture = nullptr; }
//            if (m_tutorialButtonHoverTexture) { m_tutorialButtonHoverTexture->Release(); m_tutorialButtonHoverTexture = nullptr; }
//
//            // チュートリアル表示中はゲームロジックやタイマーを更新しない
//            return;
//        }
//        //// チュートリアル表示中はゲームロジックやタイマーを更新しない
//        //return;
//    }
//
//    if (isBossStage) {
//        UpdateBossLogic(deltaTime); // ボスロジックを更新する
//    }
//    else {
//        UpdateGame(deltaTime); // ゲームロジックを更新する
//    }
//}

// こちらはトリガー付きでチュートリアルを表示する実装
void GameplayScene::Update(float deltaTime)
{
    // チュートリアル表示中は入力だけ処理し、ゲーム時間は進めない
    if (m_showTutorial) {
        g_inputSystem.Update();

        if (m_tutorialButton.Process() == UIButtonResult::Clicked) {
            m_showTutorial = false;
            g_tutorialActive = false;

            // チュートリアル画像だけ解放する（ボタン用は再利用する）
            if (m_tutorialTexture) {
                m_tutorialTexture->Release();
                m_tutorialTexture = nullptr;
            }

            return;
        }
        /*return;*/
    }

    CheckTutorialTriggers();

    if (isBossStage) {
        UpdateBossLogic(deltaTime);
    }
    else {
        UpdateGame(deltaTime);
    }
}

void GameplayScene::CheckTutorialTriggers()
{
    // すでにチュートリアル表示中なら新しいトリガーは確認しない
    if (m_showTutorial) return;

    // 現在のマップを取得する
    Map* currentMap = g_mapManager.GetCurrentMap();
    if (!currentMap) return;

    // ミッドグラウンド層（メインゲームプレイ層）の全タイルを取得する
    const std::vector<MapTile>& tiles = currentMap->GetTiles(MapLayer::MIDGROUND);

    // 各タイルについて、プレイヤーがチュートリアルトリガーに触れているか調べる
    for (const auto& tile : tiles) {
        // チュートリアル用タイルか確認する
        if (tile.tileInfo.type != "tutorial") continue;

        // プレイヤーとタイルの簡易 AABB 衝突判定
        bool collision = (
            g_player.posX < tile.posX + tile.width &&
            g_player.posX + PLAYER_WIDTH > tile.posX &&
            g_player.posY < tile.posY + tile.height &&
            g_player.posY + PLAYER_HEIGHT > tile.posY
            );

        if (collision) {
            int tutorialIndex = -1;

            // タイルコードからどのチュートリアルか判定する
            if (tile.tileInfo.code == "T1") tutorialIndex = 0;
            else if (tile.tileInfo.code == "T2") tutorialIndex = 1;
            else if (tile.tileInfo.code == "T3") tutorialIndex = 2;
            else if (tile.tileInfo.code == "T4") tutorialIndex = 3;

            // If valid tutorial and not yet triggered
            if (tutorialIndex >= 0 && tutorialIndex < 4 && !m_tutorialTriggered[tutorialIndex]) {
                m_tutorialTriggered[tutorialIndex] = true;
                m_currentTutorialIndex = tutorialIndex + 1; // 1-based for filename

                // Load the tutorial texture
                std::string path = "asset/tutorial/tutorial_" + std::to_string(m_currentTutorialIndex) + ".png";

                // Release old texture if any
                if (m_tutorialTexture) {
                    m_tutorialTexture->Release();
                    m_tutorialTexture = nullptr;
                }

                // 新しいチュートリアル画像を読み込む
                if (SUCCEEDED(LoadTexture(g_pDevice, path.c_str(), &m_tutorialTexture))) {
                    m_showTutorial = true;
                    g_tutorialActive = true;
                }

                // 一度に 1 つだけ発動し、見つけたら抜ける
                break;
            }
        }
    }
}
void GameplayScene::UpdateBossLogic(float deltaTime)
{
    // 死亡中でもタイマーだけは更新する
    if (g_player.isDead)
    {
        g_player.anim.Update(deltaTime);

        g_player.deathTimer -= deltaTime;

        if (g_player.deathTimer <= 0.0f)
        {
            RespawnBossAtCheckpoint();
        }
        return;
    }

    // 一般更新の前にボスへのローカルポインタを保持しておく。
    // UpdateGame -> UpdateEnemies の流れで死亡アニメ完了後に敵オブジェクトが削除される可能性があるため、
    // 解放済みかもしれないポインタを参照するより、g_enemies 内に存在するか確認するほうが安全。
    BossEnemy* bossPtr = dynamic_cast<BossEnemy*>(m_boss);

    UpdateGame(deltaTime);

    if (bossPtr) {
            // ボスポインタがグローバル敵リスト内に存在しないなら、
            // 死亡アニメーション完了後に削除された可能性がある。
            // 複数ボス戦（例: boss2）では、すべての BossEnemy が消えた時だけ遷移する。
        bool stillPresent = false;
        for (auto* e : g_enemies) {
            if (e == bossPtr) { stillPresent = true; break; }
        }

        if (!stillPresent) {
                // 残っているボス個体数を数える
            int remainingBosses = 0;
            BossEnemy* firstRemaining = nullptr;
            for (auto* e : g_enemies) {
                if (auto* be = dynamic_cast<BossEnemy*>(e)) {
                    ++remainingBosses;
                    if (!firstRemaining) firstRemaining = be;
                }
            }

            if (remainingBosses == 0) {
                // すべてのボスが死亡アニメを終えて削除された。
                sceneManager->SwitchScene(CAKE);
                m_boss = nullptr;
                return;
            }
            else {
                // まだ生存中、または死亡演出中の別ボスが残っている。
                // チェックポイント処理を継続できるよう、残っているボスへ m_boss を差し替える。
                m_boss = firstRemaining;
            }
        }
        else {
            // まだ存在しているなら、生存状態を確認してチェックポイント処理を行う。
            if (m_boss->IsAlive()) {
                CheckBossCheckpoints();
            }
            else {
                // ボスは死亡しているが、まだ存在している（死亡アニメ再生中）。
                // ここでは何もしないで、削除されるまで UpdateGame に更新を任せる。
            }
        }
    }
}

// ボス HP バーがチェックポイント到達条件を満たしたか確認する
void GameplayScene::CheckBossCheckpoints()
{
    if (!m_boss) return; // ボスがいなければ何もしない

    // チェックポイント計算にはボス本来の最大 HP を使う
    float maxHP = m_boss->GetMaxHealth();
    float currentHP = m_boss->GetHealth();
    float healthPercent = (maxHP > 0.0f) ? (currentHP / maxHP) : 0.0f;

    // HP が 2/3 に到達したときのチェックポイント
    if (!m_checkpoint1Reached && healthPercent <= 0.667f)
    {
        m_checkpoint1Reached = true;
        m_bossCheckpointHP = currentHP; // 復活用にボス HP を保存する

        // 後で削除予定。デバッグ用。
      /* char debugMsg[256];
       sprintf_s(debugMsg, "Boss Checkpoint 1 reached! HP saved at: %.0f (%.1f%%)\n",
           currentHP, healthPercent * 100.0f);
       OutputDebugStringA(debugMsg);*/
    }

    // HP が 1/3 に到達したときのチェックポイント
    if (!m_checkpoint2Reached && healthPercent <= 0.333f)
    {
        m_checkpoint2Reached = true;
        m_bossCheckpointHP = currentHP; // 復活用にボス HP を保存する

        // 後で削除予定。デバッグ用。
       /*char debugMsg[256];
       sprintf_s(debugMsg, "Boss Checkpoint 2 reached! HP saved at: %.0f (%.1f%%)\n",
           currentHP, healthPercent * 100.0f);
       OutputDebugStringA(debugMsg);*/
    }
}

// プレイヤー死亡時、最後のチェックポイントからボス戦を再開する
void GameplayScene::RespawnBossAtCheckpoint()
{
    // 射弹と敵を消去する。ボスの種類によっては後で見直す可能性がある。
    g_projectileManager.ClearAll();
    CleanupEnemies();

    // マップを再読み込みする（これで敵も再出現する）
    g_mapManager.ReloadCurrentMap();

    // プレイヤー状態をリセットする
    g_player.isDead = false;
    g_player.deathTimer = 0.0f;
    g_player.health = g_player.maxHealth;
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;
    g_player.isDashing = false;
    g_player.isCharging = false;

    float spawnX, spawnY;
    if (g_mapManager.GetCurrentMap()->GetDefaultSpawnPoint(spawnX, spawnY))
    {
        g_player.posX = spawnX;
        g_player.posY = spawnY;
    }

    // 再出現したボスを探し、チェックポイント HP を設定する
    if (!g_enemies.empty())
    {
        // 再出現後の敵リストから実際の BossEnemy を探す。
        m_boss = nullptr;
        for (auto* e : g_enemies) {
            if (auto* be = dynamic_cast<BossEnemy*>(e)) {
                m_boss = be;
                break;
            }
        }

        if (m_boss) {
            // チェックポイント HP があれば復元する
            if (m_bossCheckpointHP > 0.0f) {
                m_boss->SetHealth(m_bossCheckpointHP);
            }
            else {
                m_boss->SetHealth(m_boss->GetMaxHealth());
            }
            // 前回挑戦時の一時状態フラグを引き継がないよう、
            // ボスのランタイム状態も初期化する。
            m_boss->ResetState();
        }
    }

    g_gameState = STATE_PLAYING;
}

void GameplayScene::Draw()
{
    DrawGame();

    // 有効なら最前面にチュートリアルオーバーレイを描画する
    if (m_showTutorial && m_tutorialTexture) {
        // 全画面中央にチュートリアル画像を描画する
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, m_tutorialTexture, 0, 1, 1, false, 0.0f, false);
        // 次へボタンを描画する
        m_tutorialButton.Draw(1.0f);
    }

    if (isBossStage) {
        RenderBossHealthBar();
    }
}

void GameplayScene::RenderBossHealthBar()
{
    if (!m_boss || !m_boss->IsAlive()) return; // ボスがいない、または生きていなければ何もしない

    // ボス HP バーのサイズと位置
    InGameUI bossHPBarUI;
    bossHPBarUI.width = 0.6f;    // HP バーの幅
    bossHPBarUI.height = 0.6f;  // HP バーの高さ
    bossHPBarUI.x = -0.3f; // 水平方向中央
    bossHPBarUI.y = -1.1f;      // 画面下中央

    // HP バー枠を描画する
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    RenderImage(bossHPBarUI.x, bossHPBarUI.y, bossHPBarUI.width, bossHPBarUI.height, g_bossHealthBarTexture, 0, 1, 1);

    // HP 割合を計算する
    float healthRatio = m_boss->GetHealth() / m_boss->GetMaxHealth();

    // HP 比率に応じて色を変える。将来的に不要になるかもしれない。
    if (healthRatio > 0.6f) {
        //SetColor(0.2f, 1.0f, 0.2f, 1.0f);  // HP 半分以上なら緑
    }
    else if (healthRatio > 0.3f) {
        SetColor(1.0f, 0.8f, 0.0f, 1.0f);  // 中程度なら黄色
    }
    else {
        SetColor(1.0f, 0.2f, 0.2f, 1.0f);  // 瀕死なら赤
    }

    // HP バー内側の描画設定
    float innerOffsetX = 0.035f; // 正なら右、負なら左へずらす
    float innerWidthScale = 0.87f;

    float hpBarWidth = bossHPBarUI.width * innerWidthScale * healthRatio;

    RenderImageClipped(bossHPBarUI.x + innerOffsetX, bossHPBarUI.y, bossHPBarUI.width * innerWidthScale, bossHPBarUI.height,g_bossInnerHPTexture, healthRatio);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}


void GameplayScene::Uninit()
{
    //if (m_tutorialTexture) { m_tutorialTexture->Release(); m_tutorialTexture = nullptr; }
    //if (m_tutorialButtonTexture) { m_tutorialButtonTexture->Release(); m_tutorialButtonTexture = nullptr; }
    //if (m_tutorialButtonHoverTexture) { m_tutorialButtonHoverTexture->Release(); m_tutorialButtonHoverTexture = nullptr; }
    // 解放処理

    if (m_tutorialTexture) {
        m_tutorialTexture->Release();
        m_tutorialTexture = nullptr;
    }
    if (m_tutorialButtonTexture) {
        m_tutorialButtonTexture->Release();
        m_tutorialButtonTexture = nullptr;
    }
    if (m_tutorialButtonHoverTexture) {
        m_tutorialButtonHoverTexture->Release();
        m_tutorialButtonHoverTexture = nullptr;
    }
}
