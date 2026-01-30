#include "GameplayScene.h"
#include "Game.h"
#include "Enemy.h"
#include "Audio.h"

// construct
GameplayScene::GameplayScene(SceneManager* manager, int world, int area)
{
    sceneManager = manager;
    worldNumber = world;        // for the world number ( 1,2 or 3)
    areaNumber = area;          // for the area number (1 to 8)
    isBossStage = (area == 8);  // area 8 is the boss area

    // for the boss
    m_boss = nullptr;
    m_bossCheckpointHP = 0.0f;
    m_checkpoint1Reached = false;
    m_checkpoint2Reached = false;
}

bool GameplayScene::Init()
{
	ShowCursor(FALSE);
    g_gameState = STATE_PLAYING;

    // generates the map name based on world and stage
    StageInfo info(worldNumber, areaNumber);
    std::string mapName = info.GetMapName();

    g_mapManager.SwitchMap(mapName, -1, -1);

    // For boss stages we need a full reload so the boss instance/state is
    // recreated and initialized cleanly. Regular areas use a soft reset to
    // preserve transient state like gauge/particles when moving between maps.
    if (isBossStage) {
        // Ensure scene-local checkpoint tracking is cleared on (re)entering the boss stage.
        m_bossCheckpointHP = 0.0f;
        m_checkpoint1Reached = false;
        m_checkpoint2Reached = false;
        ResetGame(true);
    }
    else {
        ResetGame();
    }

    // Load tutorial overlay for first four World1 areas
    if (worldNumber == 1 && areaNumber >= 1 && areaNumber <= 4) {
        m_showTutorial = true;
        g_tutorialActive = true;
        std::string path = "asset/tutorial/tutorial_" + std::to_string(areaNumber) + ".png";
        LoadTexture(g_pDevice, path.c_str(), &m_tutorialTexture);
        LoadTexture(g_pDevice, "asset/UI/cake/nextbutton_normal.png", &m_tutorialButtonTexture);
        LoadTexture(g_pDevice, "asset/UI/cake/nextbutton_hover.png", &m_tutorialButtonHoverTexture);
        // place button at bottom-right
        m_tutorialButton = UIButton(0.7f, -0.85f, 0.35f, 0.7f, MENU, m_tutorialButtonTexture, m_tutorialButtonHoverTexture);
        m_tutorialButton.SetHitboxScale(0.25f, 0.13f);
        m_tutorialButton.SetHitboxOffset(-0.06f);
    }

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

    // if boss stage, set up the boss
    if (isBossStage) 
    {
        m_boss = nullptr;

        // 在当前敌人列表中查找 BossEnemy
        for (auto* e : g_enemies) {
            if (dynamic_cast<BossEnemy*>(e) != nullptr) {
                m_boss = e;
                break;
            }
        }

        if (m_boss) {
            // Set boss HP to 300 for normal gameplay
            m_boss->SetMaxHealth(300.0f);
            m_boss->SetHealth(300.0f);
        }
    }

    return true;
}

void GameplayScene::Update(float deltaTime)
{
    // If tutorial overlay active, handle tutorial input and don't advance game time
    if (m_showTutorial) {
        // still update raw input
        g_inputSystem.Update();

        // Process tutorial button
        if (m_tutorialButton.Process() == UIButtonResult::Clicked) {
            // hide tutorial and resume normal play
            m_showTutorial = false;
            g_tutorialActive = false;
            // release tutorial textures
            if (m_tutorialTexture) { m_tutorialTexture->Release(); m_tutorialTexture = nullptr; }
            if (m_tutorialButtonTexture) { m_tutorialButtonTexture->Release(); m_tutorialButtonTexture = nullptr; }
            if (m_tutorialButtonHoverTexture) { m_tutorialButtonHoverTexture->Release(); m_tutorialButtonHoverTexture = nullptr; }

            // while tutorial is shown, do not update game logic or timers
            return;
        }
        //// while tutorial is shown, do not update game logic or timers
        //return;
    }

    if (isBossStage) {
        UpdateBossLogic(deltaTime); // update the boss logic
    }
    else {
        UpdateGame(deltaTime); // update the game logic
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

    // Preserve a local pointer to the boss before running the general update
    // because UpdateGame -> UpdateEnemies may delete enemy objects when their
    // death animation completes. Checking membership in g_enemies is safer
    // than dereferencing a potentially deleted pointer.
    BossEnemy* bossPtr = dynamic_cast<BossEnemy*>(m_boss);

    UpdateGame(deltaTime);

    if (bossPtr) {
        // If the boss pointer no longer exists in the global enemy list,
        // it has finished its death animation and was removed — transition.
        bool stillPresent = false;
        for (auto* e : g_enemies) {
            if (e == bossPtr) { stillPresent = true; break; }
        }

        if (!stillPresent) {
            // Boss completed death sequence and was deleted; safe to switch.
            sceneManager->SwitchScene(CAKE);
            m_boss = nullptr;
            return;
        }

        // If still present, check alive state and perform checkpoint logic.
        if (m_boss->IsAlive()) {
            CheckBossCheckpoints();
        }
        else {
            // Boss is dead but still present (death animation playing). Do nothing
            // and allow UpdateGame to continue updating until removal.
        }
    }
}

// for checking if the hp bar of the boss reached the point for the checkpoint
void GameplayScene::CheckBossCheckpoints()
{
    if (!m_boss) return; // if there is not boss, dont do anything

    // Use the boss' actual max HP when computing checkpoints
    float maxHP = m_boss->GetMaxHealth();
    float currentHP = m_boss->GetHealth();
    float healthPercent = (maxHP > 0.0f) ? (currentHP / maxHP) : 0.0f;

    // Checkpoint at 2/3 HP
    if (!m_checkpoint1Reached && healthPercent <= 0.667f)
    {
        m_checkpoint1Reached = true;
        m_bossCheckpointHP = currentHP; // save the boss hp for the respawn

        // erase later. this is jsut for debug
      /* char debugMsg[256];
       sprintf_s(debugMsg, "Boss Checkpoint 1 reached! HP saved at: %.0f (%.1f%%)\n",
           currentHP, healthPercent * 100.0f);
       OutputDebugStringA(debugMsg);*/
    }

    // Checkpoint at 1/3 HP
    if (!m_checkpoint2Reached && healthPercent <= 0.333f)
    {
        m_checkpoint2Reached = true;
        m_bossCheckpointHP = currentHP; // save the boss hp for the respawn

        // erased later. just for debug
       /*char debugMsg[256];
       sprintf_s(debugMsg, "Boss Checkpoint 2 reached! HP saved at: %.0f (%.1f%%)\n",
           currentHP, healthPercent * 100.0f);
       OutputDebugStringA(debugMsg);*/
    }
}

// for the boss to respawn at the last checkpoint (so the hp bar will be full or not) when player dies
void GameplayScene::RespawnBossAtCheckpoint()
{
    // erases projectiles and ememies. might change this later depending on the type of boss
    g_projectileManager.ClearAll();
    CleanupEnemies();

    // Reload the map (this will respawn enemies)
    g_mapManager.ReloadCurrentMap();

    // Reset player state
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

    // Find the respawned boss and set its hp to checkpoint
    if (!g_enemies.empty())
    {
        // Find the actual BossEnemy instance in the respawned enemy list.
        m_boss = nullptr;
        for (auto* e : g_enemies) {
            if (auto* be = dynamic_cast<BossEnemy*>(e)) {
                m_boss = be;
                break;
            }
        }

        if (m_boss) {
            m_boss->SetMaxHealth(300.f);
            // restore checkpoint hp if any
            if (m_bossCheckpointHP > 0.0f) {
                m_boss->SetHealth(m_bossCheckpointHP);
            }
            else {
                m_boss->SetHealth(300.f);
            }
            // Also reset transient boss state so it doesn't carry over
            // behavior flags from a previous attempt.
            m_boss->ResetState();
        }
    }

    g_gameState = STATE_PLAYING;
}

void GameplayScene::Draw()
{
    DrawGame();

    // Draw tutorial overlay on top if active
    if (m_showTutorial && m_tutorialTexture) {
        // draw full-screen centered tutorial image
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, m_tutorialTexture, 0, 1, 1, false, 0.0f, false);
        // Draw next button
        m_tutorialButton.Draw(1.0f);
    }

    if (isBossStage) {
        RenderBossHealthBar();
    }
}

void GameplayScene::RenderBossHealthBar()
{
    if (!m_boss || !m_boss->IsAlive()) return; // if there is no boss or if its not alive, dont do anything

    // Boss HP bar dimensions and position
    InGameUI bossHPBarUI;
    bossHPBarUI.width = 0.6f;    // for the width of the HP bar
    bossHPBarUI.height = 0.6f;  // for the height of the HP bar
    bossHPBarUI.x = -0.3f; // Center horizontally
    bossHPBarUI.y = -1.1f;      // Bottom center of screen

    // for the hp bar when its not full
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    RenderImage(bossHPBarUI.x, bossHPBarUI.y, bossHPBarUI.width, bossHPBarUI.height, g_bossHealthBarTexture, 0, 1, 1);

    // Calculate health
    float healthRatio = m_boss->GetHealth() / m_boss->GetMaxHealth();

    // Color changes depenging on HP percentage. MIGHT NOT NEED IN THE FUTURE
    if (healthRatio > 0.6f) {
        //SetColor(0.2f, 1.0f, 0.2f, 1.0f);  // greeen for more than half of hp
    }
    else if (healthRatio > 0.3f) {
        SetColor(1.0f, 0.8f, 0.0f, 1.0f);  // yellow for half of hp
    }
    else {
        SetColor(1.0f, 0.2f, 0.2f, 1.0f);  // red for almost death
    }

    // for the inner part of the hp bar
    float innerOffsetX = 0.035f; // positive shift right, negative shift left
    float innerWidthScale = 0.87f;

    float hpBarWidth = bossHPBarUI.width * innerWidthScale * healthRatio;

    RenderImageClipped(bossHPBarUI.x + innerOffsetX, bossHPBarUI.y, bossHPBarUI.width * innerWidthScale, bossHPBarUI.height,g_bossInnerHPTexture, healthRatio);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void GameplayScene::Uninit()
{
    if (m_tutorialTexture) { m_tutorialTexture->Release(); m_tutorialTexture = nullptr; }
    if (m_tutorialButtonTexture) { m_tutorialButtonTexture->Release(); m_tutorialButtonTexture = nullptr; }
    if (m_tutorialButtonHoverTexture) { m_tutorialButtonHoverTexture->Release(); m_tutorialButtonHoverTexture = nullptr; }
    // for erasing
}