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

        // this is temporary for the testboss which is a normal enemy we have already
        if (!g_enemies.empty()) 
        {
            m_boss = g_enemies[0]; // make the boss to be the enemy

            // this is just for a test so the current enemy has a lot of hp so i can test the boss hp
            m_boss->SetMaxHealth(500000.0f);
            m_boss->SetHealth(500000.0f);
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

    // if the player dies, the animation of death is over, the player respawns
    if (g_player.isDead && g_player.deathTimer > 1.0f)
    {
        RespawnBossAtCheckpoint();
    }

    // if boss died, the hp bar disspears as well as the boss.
    if (m_boss && !m_boss->IsAlive())
    {
        m_boss = nullptr;

        // after killing the boss in world 1
        //if (worldNumber == 1) { 
        //    
        //    sceneManager->SwitchScene(CAKE); // go to cake scene
        //}
        // after killing the boss in world 2. add it later when there is one
        //else if (worldNum == 2) {
        //    sceneManager->SwitchToStage(CAKE);
        //}
        // // after killing the boss in world 3. add it later when there is one
        //else if (worldNum == 3) {  
        //    sceneManager->SwitchScene(CAKE);
        //}

        sceneManager->SwitchScene(CAKE);

        return;
    }
}

// for checking if the hp bar of the boss reached the point for the checkpoint
void GameplayScene::CheckBossCheckpoints()
{
    if (!m_boss) return; // if there is not boss, dont do anything

    float maxHP = 500000.0f; // this is just for a test. the actual hp is not this
    float currentHP = m_boss->GetHealth();
    float healthPercent = currentHP / maxHP;

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
        m_boss->SetMaxHealth(500000.0f); // Restore boss stats for the current test enemy 

        // it has reached a checkpoint, restores the checkpoint hp 
        if (m_bossCheckpointHP > 0.0f) 
        {
            m_boss->SetHealth(m_bossCheckpointHP);
        }
        else // if it hasnt reached any checkpoint
        {
            m_boss->SetHealth(500000.0f);
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
    float barWidth = 1.2f;    // for the width of the HP bar
    float barHeight = 0.06f;  // for the height of the HP bar
    float barX = -barWidth * 0.5f; // Center horizontally
    float barY = -0.85f;      // Bottom center of screen

    // for around the hp bar color
    SetColor(0.1f, 0.1f, 0.1f, 0.9f);
    RenderImage(barX - 0.01f, barY - 0.01f, barWidth + 0.02f, barHeight + 0.02f,
        g_bossHealthBarTexture, 0, 1, 1);

    // for when the the hp bar is not full
    SetColor(0.3f, 0.1f, 0.1f, 0.8f);
    RenderImage(barX, barY, barWidth, barHeight, g_bossHealthBarTexture, 0, 1, 1);

    // Calculate health
    float healthRatio = m_boss->GetHealth() / m_boss->GetMaxHealth();

    // Color changes depenging on HP percentage
    if (healthRatio > 0.6f) {
        SetColor(0.2f, 1.0f, 0.2f, 1.0f);  // greeen for more than half of hp
    }
    else if (healthRatio > 0.3f) {
        SetColor(1.0f, 0.8f, 0.0f, 1.0f);  // yellow for half of hp
    }
    else {
        SetColor(1.0f, 0.2f, 0.2f, 1.0f);  // red for almost death
    }

    // Draw filled HP bar
    RenderImage(barX, barY, barWidth * healthRatio, barHeight, g_bossHealthBarTexture, 0, 1, 1);
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void GameplayScene::Uninit()
{
    // for erasing
}