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

    ResetGame();

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
    if (isBossStage) {
        UpdateBossLogic(deltaTime); // update the boss logic
    }
    else {
        UpdateGame(deltaTime); // update the game logic
    }
}

void GameplayScene::UpdateBossLogic(float deltaTime)
{
    // for checking if the hp bar reahed the checkpoint or not
    if (m_boss && m_boss->IsAlive()) {
        CheckBossCheckpoints();
    }

    UpdateGame(deltaTime);

    // If the player dies, wait for the normal death animation duration,
    // then respawn at the last boss checkpoint.
    // Note: `g_player.deathTimer` counts DOWN from `DEATH_RESPAWN_TIME` to 0.
    if (g_player.isDead && g_player.deathTimer <= 0.0f)
    {
        RespawnBossAtCheckpoint();
    }

    // if boss died, the hp bar disspears as well as the boss.
    if (m_boss && !m_boss->IsAlive())
    {
        m_boss = nullptr;

        ////after killing the boss in world 1
        //if (worldNumber == 1) { 
        //    
        //    sceneManager->SwitchScene(CAKE); // go to cake scene
        //}
        // //after killing the boss in world 2. add it later when there is one
        //else if (worldNumber == 2) {
        //    sceneManager->SwitchScene(CAKE);
        //}
        // // after killing the boss in world 3. add it later when there is one
        //else if (worldNumber == 3) {  
        //    sceneManager->SwitchScene(CAKE);
        //}
        char dbg[256];
        sprintf_s(dbg, "BOSS DEAD: world=%d, area=%d\n", worldNumber, areaNumber);
        OutputDebugStringA(dbg);

        sceneManager->SwitchScene(CAKE);

        return;
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
        m_boss = g_enemies[0];
        m_boss->SetMaxHealth(300.f);// tore boss stats for the current test enemy

        // it has reached a checkpoint, restores the checkpoint hp 
        if (m_bossCheckpointHP > 0.0f) 
        {
            m_boss->SetHealth(m_bossCheckpointHP);
        }
        else // if it hasnt reached any checkpoint
        {
            m_boss->SetHealth(300.f);
        }
    }

    g_gameState = STATE_PLAYING;
}

void GameplayScene::Draw()
{
    DrawGame();

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

    // Draw filled HP bar
    //RenderImage(bossHPBarUI.x, bossHPBarUI.y, bossHPBarUI.width * healthRatio, bossHPBarUI.height, g_bossInnerHPTexture, 0, 1, 1);

    float hpBarWidth = bossHPBarUI.width * healthRatio;
    RenderImageClipped(bossHPBarUI.x, bossHPBarUI.y, hpBarWidth, bossHPBarUI.height, g_bossInnerHPTexture, healthRatio);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void GameplayScene::Uninit()
{
    // for erasing
}