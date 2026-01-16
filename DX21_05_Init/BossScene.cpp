#include "BossScene.h"

//the construct
BossScene::BossScene(SceneManager* manager)
{
    sceneManager = manager;
    m_boss = nullptr;
    m_bossCheckpointHP = 0.0f;
    m_checkpoint1Reached = false;  // for the first checkpoint
    m_checkpoint2Reached = false;  // for the second checkpoint
}

// Initialize the stage
bool BossScene::Init()
{
    CleanupEnemies();

    g_mapManager.SwitchMap("boss", 0, 1);

    g_gameState = STATE_PLAYING;

    m_boss = nullptr;

    // this is temporary for the testboss which is a normal enemy we have already
    if (!g_enemies.empty()) {
        m_boss = g_enemies[0];

        // this is just for a test so the current enemy has a lot of hp so i can test the boss hp
        m_boss->SetMaxHealth(500000.0f); 
        m_boss->SetHealth(500000.0f);
    }


    return true;
}

// Update the stage logic
void BossScene::Update(float deltaTime)
{
    static bool wasPlayerAlive = true;
    bool isPlayerAlive = !g_player.isDead;

    // for checking if the hp bar reahed the checkpoint or not
    if (m_boss && m_boss->IsAlive()) {
        CheckBossCheckPoints();
    }

    UpdateGame(deltaTime);

    // if the player dies, the animation od death is over, the player respawns
    if (g_player.isDead && g_player.deathTimer > 1.0f) 
    {
        RespawnBossAtCheckpoint();
    }

    // if boss died, the hp bar disspears as well as the boss.
    if (m_boss && !m_boss->IsAlive()) 
    {
        m_boss = nullptr;  

        // Switch to CakeScene after boss death 
        sceneManager->SwitchScene(CAKE); 
        return; // Stop updating this scene
    }
 
}

// for checking if the hp bar of the boss reached the point for the checkpoint
void BossScene::CheckBossCheckPoints()
{
    if (!m_boss) return; // if there is not boss, dont do anything

    float maxHP = 500000.0f; // this is just for a test. the actual hp is not this
    float currentHP = m_boss->GetHealth();
    float healthPercent = currentHP / maxHP;

    // if the hp bar is 2/3
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

    // if the hp bar is 1/3
    if (!m_checkpoint2Reached && healthPercent <= 0.333f) 
    {
        m_checkpoint2Reached = true;
        m_bossCheckpointHP = currentHP; // saves the boss hp for the respawn

        // erased later. just for debug
        /*char debugMsg[256];
        sprintf_s(debugMsg, "Boss Checkpoint 2 reached! HP saved at: %.0f (%.1f%%)\n",
            currentHP, healthPercent * 100.0f);
        OutputDebugStringA(debugMsg);*/
    }
}

// for the boss to respawn at the last checkpoint (so the hp bar will be full or not) when player dies
void BossScene::RespawnBossAtCheckpoint()
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

    // Respawn player at spawn point
    float spawnX;
    float spawnY;
    if (g_mapManager.GetCurrentMap()->GetDefaultSpawnPoint(spawnX, spawnY)) 
    {
        g_player.posX = spawnX;
        g_player.posY = spawnY;
    }

    // Find the respawned boss and set its hp to checkpoint
    if (!g_enemies.empty()) 
    {
        m_boss = g_enemies[0]; 
        m_boss->SetMaxHealth(500000.0f);  // Restore boss stats for the current test enemy 

        if (m_bossCheckpointHP > 0.0f)  // it has reached a checkpoint, restores the checkpoint hp 
        { 
            m_boss->SetHealth(m_bossCheckpointHP); 
        } 
        else // if hasnt reached any checkpoint
        { 
            m_boss->SetHealth(500000.0f); 
        } 
    }

    g_gameState = STATE_PLAYING;
}

// Draw the stage
void BossScene::Draw()
{
    DrawGame();

    RenderBossHealthBar();
}

void BossScene::RenderBossHealthBar()
{
    // if there is no boss or if its not alive, dont do anything
    if (!m_boss || !m_boss->IsAlive()) return;

    // Boss HP bar dimensions and position
    float barWidth = 1.2f;      // for the width of the HP bar
    float barHeight = 0.06f;    // for the height of the HP bar
    float barX = -barWidth * 0.5f; // Center horizontally
    float barY = -0.85f;        // Bottom center of screen

    // for around the hp bar color
    SetColor(0.1f, 0.1f, 0.1f, 0.9f);
    RenderImage(barX - 0.01f, barY - 0.01f, barWidth + 0.02f, barHeight + 0.02f,
        g_groundTexture, 0, 1, 1);

    // for when the the hp bar is not full
    SetColor(0.3f, 0.1f, 0.1f, 0.8f);
    RenderImage(barX, barY, barWidth, barHeight, g_groundTexture, 0, 1, 1);

    // Calculate health
    float healthRatio = m_boss->GetHealth() / m_boss->GetMaxHealth();

    // Color changes depenging on HP percentage
    if (healthRatio > 0.6f) {
        SetColor(0.2f, 1.0f, 0.2f, 1.0f); // greeen for more than half of hp
    }
    else if (healthRatio > 0.3f) {
        SetColor(1.0f, 0.8f, 0.0f, 1.0f); // orange/yello for half of hp
    }
    else {
        SetColor(1.0f, 0.2f, 0.2f, 1.0f); // red for almost death
    }

    // Draw filled HP bar
    RenderImage(barX, barY, barWidth * healthRatio, barHeight, g_groundTexture, 0, 1, 1);

    // Reset color
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

// Cleanup
void BossScene::Uninit()
{
    //ResetGame();  // Reset game state

    //CleanUpGameWorld();  // Release all textures and cleanup

}