#include "Game.h"
// Add to global variable definition section in Game.cpp
float g_slowMoTimer = 0.0f;
float g_slowMoFactor = 1.0f;
bool g_isSlowMotion = false;

// added december 4th
// for the timer of the game
float g_gameElapsedTime = 0.0f;
int g_gameMinutes = 0;
int g_gameSeconds = 0;


// Sound effect instance ID storage
int g_jumpSoundId = -1;
int g_dashSoundId = -1;
int g_chargeSoundId = -1;
int g_shootSoundId = -1;
int g_slowMoTimerSoundId = -1;

static std::vector<HitEffectInstance> g_weakPointHitEffects;
static constexpr int WEAKPOINT_HIT_EFFECT_FRAMES = 8;
static constexpr int WEAKPOINT_HIT_EFFECT_COLUMNS = 4;
static constexpr int WEAKPOINT_HIT_EFFECT_ROWS = 2;

GameStatistics g_gameStats;

void SpawnWeakPointHitEffect(float worldX, float worldY) {
    if (!g_hitEffectTexture) return;

    HitEffectInstance e;
    e.x = worldX;
    e.y = worldY;
    e.timer = 0.0f;
    e.frameTime = 0.08f;
    e.frame = 0;
    e.active = true;
    g_weakPointHitEffects.push_back(e);
}

// Game timer implementation
GameTimer::GameTimer()
{
    __int64 countsPerSec;
    QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
    m_secondsPerCount = 1.0 / static_cast<double>(countsPerSec);
}

void GameTimer::Tick() {
    QueryPerformanceCounter((LARGE_INTEGER*)&m_currTime);
    if (m_prevTime != 0) {
        m_deltaTime = static_cast<float>((m_currTime - m_prevTime) * m_secondsPerCount);
    }
    m_prevTime = m_currTime;
    m_totalTime = (m_currTime - m_startTime) * m_secondsPerCount;
}

float GameTimer::GetDeltaTime() const {
    return m_deltaTime;
}

// Trigger slow motion effect
void TriggerSlowMotion(float duration = 1.0f, float factor = 0.3f) {
    g_isSlowMotion = true;
    g_slowMoTimer = duration;
    g_slowMoFactor = factor;
}

void ResetGame() {
    g_projectileManager.ClearAll();  // New: clear all projectiles
    CleanupEnemies();
    g_weakPointHitEffects.clear();
    if (g_mapManager.IsMapLoaded()) {
        g_mapManager.ReloadCurrentMap();
    }

    g_player.comboCount = 0;
    g_player.comboTimer = 0.0f;

    // Reset gauge bar
    g_player.gaugePoints = 0;
    g_player.isInvincible = false;
    g_player.invincibleTimer = 0.0f;

    g_gameState = STATE_PLAYING;
}

// added december 11th
void CleanUpGameWorld()
{
    g_projectileManager.ClearAll();
    CleanupEnemies();
    g_mouseIndicator.Cleanup();
    g_weakPointHitEffects.clear();

    // 释放所有纹理 - 只保留右边纹理
    if (g_playerTexture) {
        g_playerTexture->Release();
        g_playerTexture = nullptr;
    }

    // 只保留右边的纹理
    if (g_playerIdleTexture) {
        g_playerIdleTexture->Release();
        g_playerIdleTexture = nullptr;
    }
    if (g_playerJumpTexture) {
        g_playerJumpTexture->Release();
        g_playerJumpTexture = nullptr;
    }
    if (g_playerRunTexture) {
        g_playerRunTexture->Release();
        g_playerRunTexture = nullptr;
    }
    if (g_playerSlash1Texture) {
        g_playerSlash1Texture->Release();
        g_playerSlash1Texture = nullptr;
    }
    if (g_playerSlash2Texture) {
        g_playerSlash2Texture->Release();
        g_playerSlash2Texture = nullptr;
    }
    if (g_playerSlash3Texture) {
        g_playerSlash3Texture->Release();
        g_playerSlash3Texture = nullptr;
    }
    if (g_playerSlash4Texture) {
        g_playerSlash4Texture->Release();
        g_playerSlash4Texture = nullptr;
    }
    if (g_playerAirChargeTexture) {
        g_playerAirChargeTexture->Release();
        g_playerAirChargeTexture = nullptr;
    }
    if (g_playerFallingTexture) {
        g_playerFallingTexture->Release();
        g_playerFallingTexture = nullptr;
    }
    if (g_playerGroundChargeTexture) {
        g_playerGroundChargeTexture->Release();
        g_playerGroundChargeTexture = nullptr;
    }
    if (g_playerWallSlideTexture) {
        g_playerWallSlideTexture->Release();
        g_playerWallSlideTexture = nullptr;
    }
    if (g_playerDeathTexture) {
        g_playerDeathTexture->Release();
        g_playerDeathTexture = nullptr;
    }



    if (g_groundTexture) {
        g_groundTexture->Release();
        g_groundTexture = nullptr;
    }
    if (g_backgroundTexture1) {
        g_backgroundTexture1->Release();
        g_backgroundTexture1 = nullptr;
    }
    if (g_dashEffectTexture) {
        g_dashEffectTexture->Release();
        g_dashEffectTexture = nullptr;
    }
    if (g_chargeEffectTexture) {
        g_chargeEffectTexture->Release();
        g_chargeEffectTexture = nullptr;
    }
    //解放击中特效纹理
    if (g_hitEffectTexture) {
        g_hitEffectTexture->Release();
        g_hitEffectTexture = nullptr;
    }
    if (g_numberTexture) {
        g_numberTexture->Release();
        g_numberTexture = nullptr;
    }
    if (g_uiNumberTexture) {
        g_uiNumberTexture->Release();
        g_uiNumberTexture = nullptr;
    }
    if (g_arrowTexture) {
        g_arrowTexture->Release();
        g_arrowTexture = nullptr;
    }
    if (g_cursorTexture) {
        g_cursorTexture->Release();
        g_cursorTexture = nullptr;
    }

    // for the combo texture
    if (g_comboNumberTexture) {
        g_comboNumberTexture->Release();
        g_comboNumberTexture = nullptr;
    }
    if (g_comboXTexture) {
        g_comboXTexture->Release();
        g_comboXTexture = nullptr;
    }

    //for the gague bar when there is one
    if (g_gaugeBarTexture) {
        g_gaugeBarTexture->Release();
        g_gaugeBarTexture = nullptr;
    }

}

// Improved collision detection function
bool CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
        y1 < y2 + h2 && y1 + h1 > y2);
}

// for the combo UI of the player when hitting enemies
void DrawComboUI(void)
{
    if (g_player.comboCount < 1)
    {
        return;
    }

    float comboX = 0.6f;   // right side
    float comboY = 0.7f;   // top side

    // change the size of it
    float xWidth = 0.15f;   // Width of the "X" symbol
    float xHeight = 0.15f;  // Height of the "X" symbol
    float numberWidth = 0.1f;   // Width of the number
    float numberHeight = 0.1f;  // Height of the number
    float spaceBetweenDigits = 0.1f;

    float pixelX = (SCREEN_WIDTH * 0.5f) - 120.0f;
    float pixelY = (SCREEN_HEIGHT * 0.5f) - 40.0f;

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Draw the "X" symbol
    RenderImage(comboX, comboY, xWidth, xHeight, g_comboXTexture, 0, 1, 1);

    char buffer[32];
    sprintf_s(buffer, "%d", g_player.comboCount);

    float numberXaxis = comboX + numberWidth + 0.02f; // for the first number x axis position

    // Draw combo number
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        int digit = buffer[i] - '0';  // 1 for frame 1, 2 for frame 2, 3 for frame 3, etc etc
        RenderImage(numberXaxis, comboY, numberWidth, numberHeight,
            pTextureNum, digit, 1, 10);

        numberXaxis += spaceBetweenDigits;  // Move to next digit position
    }
}

// for the gauge bar UI
void DrawGaugeUI(void)
{
    // Position on left side of screen
    float gaugeX = -0.9f;   // Left side
    float gaugeY = -0.8f;   // Center vertical

    // Gauge bar dimensions (vertical bar)
    float gaugeWidth = 0.08f;
    float gaugeHeight = 1.0f;  // Total height

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Draw the empy bar
    SetColor(0.2f, 0.2f, 0.2f, 0.8f);
    RenderImage(gaugeX, gaugeY, gaugeWidth, gaugeHeight, g_groundTexture, 0, 1, 1);

    // Calculate the fill ratio 
    float fillRatio = static_cast<float>(g_player.gaugePoints) / static_cast<float>(g_player.MAX_GAUGE_POINTS);

    // Filled portion of the bar (height)
    float filledHeight = gaugeHeight * fillRatio;

    // Draw filled bar (yellow when filling, bright yellow when full)
    if (g_player.gaugePoints >= g_player.MAX_GAUGE_POINTS) {
        // when full bar (with pulsing effect so you know you can click in order to be invicible)
        float barBeating = 0.8f + 0.2f * sin(g_gameElapsedTime * 5.0f);
        SetColor(1.0f * barBeating, 1.0f * barBeating, 0.0f, 1.0f);
    }
    else {
        // when filling 
        SetColor(1.0f, 1.0f, 0.0f, 1.0f);
    }

    // Draw filled portion from the bottom to up
    RenderImage(gaugeX, gaugeY, gaugeWidth, filledHeight, g_groundTexture, 0, 1, 1);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

// for the score UI
void DrawScoreUI(void)
{
    if (!g_uiNumberTexture) return;

    // Position on top left, below the timer
    float scoreX = -0.8f;   // left side in the x axis
    float scoreY = 0.6f;    // below the timer in the y axis
    float numberSize = 0.08f; // the size of the number

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Get current score
    int currentScore = g_gameStats.GetTotalScore();

    // Recalculate score in during the game
    // You can comment this out if you only want final score at the end
    int killPoints = (g_gameStats.GetEnemiesKilled() * 10) + (g_gameStats.GetWeakPointKills() * 30);
    //int timeBonus = std::max(0, static_cast<int>(60 - g_gameElapsedTime));
    int timePenalty = static_cast<int>(g_gameElapsedTime);  // current elapsed time in seconds
    int deathPenalty = g_gameStats.GetTotalDeaths() * 5;
    //int liveScore = std::max(0, (killPoints * timeBonus) - deathPenalty);
    int liveScore = std::max(0, killPoints - timePenalty - deathPenalty);

    // Draw the live score
    DrawNumber(liveScore, scoreX, scoreY, numberSize, g_numberTexture);
}


// Game initialization
void InitGameWorld() {
    g_projectileManager.LoadTextures(g_pDevice);

    // 只加载右边的纹理
    LoadTexture(g_pDevice, "asset/character/idle_right.png", &g_playerIdleTexture);
    LoadTexture(g_pDevice, "asset/character/jump_right.png", &g_playerJumpTexture);
    LoadTexture(g_pDevice, "asset/character/run_right.png", &g_playerRunTexture);
    LoadTexture(g_pDevice, "asset/character/slash_right1.png", &g_playerSlash1Texture);
    LoadTexture(g_pDevice, "asset/character/slash_right2.png", &g_playerSlash2Texture);
    LoadTexture(g_pDevice, "asset/character/slash_right3.png", &g_playerSlash3Texture);
    LoadTexture(g_pDevice, "asset/character/slash_right4.png", &g_playerSlash4Texture);
    LoadTexture(g_pDevice, "asset/character/air_charge_right.png", &g_playerAirChargeTexture);
    LoadTexture(g_pDevice, "asset/character/falling_right.png", &g_playerFallingTexture);
    LoadTexture(g_pDevice, "asset/character/ground_charge_right.png", &g_playerGroundChargeTexture);
    LoadTexture(g_pDevice, "asset/character/wall_slide_right.png", &g_playerWallSlideTexture);
    LoadTexture(g_pDevice, "asset/character/death_right.png", &g_playerDeathTexture);

    // 为动画剪辑添加通用名称（不再区分左右）
    g_player.anim.AddClip("Idle", 0, 3, 4, 1, 0.25f, true, g_playerIdleTexture);
    g_player.anim.AddClip("Jump", 0, 10, 11, 1, 0.06f, false, g_playerJumpTexture);
    g_player.anim.AddClip("Run", 0, 3, 4, 1, 0.1f, true, g_playerRunTexture);
    g_player.anim.AddClip("Slash1", 0, 3, 4, 1, 0.06f, false, g_playerSlash1Texture);
    g_player.anim.AddClip("Slash2", 0, 3, 4, 1, 0.06f, false, g_playerSlash2Texture);
    g_player.anim.AddClip("Slash3", 0, 3, 4, 1, 0.06f, false, g_playerSlash3Texture);
    g_player.anim.AddClip("Slash4", 0, 3, 4, 1, 0.06f, false, g_playerSlash4Texture);
    g_player.anim.AddClip("AirCharge", 0, 0, 1, 1, 0.25f, true, g_playerAirChargeTexture);
    g_player.anim.AddClip("Falling", 0, 0, 1, 1, 0.25f, true, g_playerFallingTexture);
    g_player.anim.AddClip("GroundCharge", 0, 0, 1, 1, 0.25f, true, g_playerGroundChargeTexture);
    g_player.anim.AddClip("WallSlide", 0, 0, 1, 1, 0.25f, true, g_playerWallSlideTexture);
    g_player.anim.AddClip("Death", 0, 10, 11, 1, 0.1f, false, g_playerDeathTexture);

    LoadTexture(g_pDevice, "asset/platform/platformtest.png", &g_groundTexture);
    LoadTexture(g_pDevice, "asset/background/1-6background.png", &g_backgroundTexture1);

    LoadTexture(g_pDevice, "asset/UI/number.png", &g_numberTexture);
    LoadTexture(g_pDevice, "asset/UI/time1.png", &g_uiNumberTexture);

    LoadTexture(g_pDevice, "asset/UI/arrow.png", &g_arrowTexture);
    LoadTexture(g_pDevice, "asset/UI/cursor.png", &g_cursorTexture);

    LoadTexture(g_pDevice, "asset/UI/combo/combo_number.png", &g_comboNumberTexture);
    LoadTexture(g_pDevice, "asset/UI/combo/combo_X.png", &g_comboXTexture);

    LoadTexture(g_pDevice, "asset/effect/effect_hit.png", &g_hitEffectTexture);

    // todo: add here the gague bar texture when there is one
    //LoadTexture(g_pDevice, "asset/UI/gauge_bar.png", &g_gaugeBarTexture);

    InitEnemies();
    g_mapManager.InitializeMaps();
    g_mouseIndicator.Initialize();

    g_camera.SetSmoothness(camera_Smoothness);
    g_camera.SetLookAhead(camera_LookAhead);
    g_camera.SetDeadZone(camera_DeadZone);
}


// Modified game update function
void UpdateGame(float deltaTime) {
    if (g_gameState != STATE_PLAYING) {
        return;
    }
    g_player.animLockDuration -= deltaTime;

    // Update audio manager
    g_gameTimer.Tick(); // added december 3rd

    // added december 4th
    g_gameElapsedTime += deltaTime;
    g_gameMinutes = static_cast<int>(g_gameElapsedTime) / 60;
    g_gameSeconds = static_cast<int>(g_gameElapsedTime) % 60;

    g_gameStats.UpdateTime(g_gameElapsedTime); // track total time

    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY);

    // Shoot fireball from player position towards mouse position
    /*g_projectileManager.CreateFireball(
        g_player.posX + PLAYER_WIDTH / 2,  // Shoot from player center
        g_player.posY + PLAYER_HEIGHT / 2,
        mouseX,
        mouseY,
        true  // From player
    );*/

    // Update slow motion effect
    if (g_isSlowMotion) {
        g_slowMoTimer -= deltaTime;
        if (g_slowMoTimer <= 0.0f) {
            g_isSlowMotion = false;
            g_slowMoFactor = 1.0f; // Restore normal time
        }
    }

    // Apply time scaling effect (priority: slow motion > charge effect)
    float timeScale = 1.0f;
    if (g_isSlowMotion) {
        timeScale = g_slowMoFactor; // Use slow motion factor

    }
    else if (g_player.isCharging) {
        float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;
        chargeRatio = std::min(chargeRatio * 8, 1.0f);
        timeScale = 1.0f - chargeRatio * 0.8f;
    }


    // for updating the combo timer
    if (g_player.comboCount > 0) {
        g_player.comboTimer -= deltaTime;
        if (g_player.comboTimer <= 0.0f) {
            g_player.comboCount = 0;
            g_player.comboTimer = 0.0f;
        }
    }

    // for updating the invincibility timer
    if (g_player.isInvincible) {
        g_player.invincibleTimer -= deltaTime;
        if (g_player.invincibleTimer <= 0.0f) {
            g_player.isInvincible = false;
            g_player.invincibleTimer = 0.0f;
        }
    }


    float scaledDeltaTime = deltaTime * timeScale;

    for (auto it = g_weakPointHitEffects.begin(); it != g_weakPointHitEffects.end();) {
        if (!it->active) {
            it = g_weakPointHitEffects.erase(it);
            continue;
        }

        it->timer += scaledDeltaTime;
        while (it->timer >= it->frameTime) {
            it->timer -= it->frameTime;
            it->frame++;
        }

        if (it->frame >= WEAKPOINT_HIT_EFFECT_FRAMES) {
            it = g_weakPointHitEffects.erase(it);
        }
        else {
            ++it;
        }
    }

    g_camera.Update(scaledDeltaTime);
	g_player.hitStopTimer -= scaledDeltaTime;
    if (g_player.hitStopTimer <= 0.0f) {
        // Update game logic using adjusted time
        UpdateDash(deltaTime);
        UpdatePlayerPhysics(scaledDeltaTime);

        UpdateEnemies(scaledDeltaTime, &g_mapManager);
        // Update all projectiles
        g_projectileManager.Update(scaledDeltaTime, &g_mapManager, g_enemies);
        // 在UpdateGame函数中修改动画设置部分
        if (g_player.animLockTimer <= 0.0f)
        {
            if (g_player.isDead) // for when dying
            {
                if (g_player.anim.GetCurrentClipName() != "Death") {
                    g_player.anim.SetClip("Death");
                }
            }

            else if (g_player.isCharging) // 如果玩家正在蓄力
            {
                if (!g_player.isOnGround) // 如果玩家在空中蓄力
                {
                    if (g_player.anim.GetCurrentClipName() != "AirCharge") {
                        g_player.anim.SetClip("AirCharge");
                    }
                }
                else // 如果玩家在地面蓄力
                {
                    if (g_player.anim.GetCurrentClipName() != "GroundCharge") {
                        g_player.anim.SetClip("GroundCharge");
                    }
                }
            }
            else if (g_player.isDashing) // 如果玩家正在冲刺
            {
                int chargeType = g_player.GetChargeLevel();

                if (chargeType == 0) // 斩击1
                {
                    if (g_player.anim.GetCurrentClipName() != "Slash1") {
                        g_player.anim.SetClip("Slash1");
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
                else if (chargeType == 1) // 斩击2
                {
                    if (g_player.anim.GetCurrentClipName() != "Slash2") {
                        g_player.anim.SetClip("Slash2");
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
                else if (chargeType == 2) // 斩击3
                {
                    if (g_player.anim.GetCurrentClipName() != "Slash3") {
                        g_player.anim.SetClip("Slash3");
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
                else  // 斩击4
                {
                    if (g_player.anim.GetCurrentClipName() != "Slash4") {
                        g_player.anim.SetClip("Slash4");
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
            }
            else if (g_player.isWallSliding) {
                if (g_player.anim.GetCurrentClipName() != "WallSlide") {
                    g_player.anim.SetClip("WallSlide");
                }
            }
            else if (!g_player.isOnGround) // 玩家不在地面上
            {
                if (g_player.velocityY < 0.0f) // 下落
                {
                    if (g_player.anim.GetCurrentClipName() != "Falling") {
                        g_player.anim.SetClip("Falling");
                    }
                }
                else // 跳跃
                {
                    if (g_player.anim.GetCurrentClipName() != "Jump") {
                        g_player.anim.SetClip("Jump");
                    }
                }
            }
            else if (g_player.isMoving) // 玩家在移动
            {
                if (g_player.anim.GetCurrentClipName() != "Run") {
                    g_player.anim.SetClip("Run");
                }
            }
            else // 玩家站立
            {
                if (g_player.anim.GetCurrentClipName() != "Idle") {
                    g_player.anim.SetClip("Idle");
                }
            }
        }

        g_player.anim.Update(scaledDeltaTime);
        UpdatePlayerDeath(scaledDeltaTime);
    }
    g_mouseIndicator.Update(scaledDeltaTime);
}

// Helper function: Get texture based on tile code
ID3D11ShaderResourceView* GetTextureForTile(const std::string& tileCode) {
    if (tileCode == "G1" || tileCode == "G2" || tileCode == "G3") {
        return g_groundTexture;
    }
    else if (tileCode == "W1" || tileCode == "W2") {
        return g_groundTexture;
    }
    else if (tileCode == "P1" || tileCode == "P2") {
        return g_groundTexture;
    }
    else if (tileCode == "DF" || tileCode == "DI" || tileCode == "DT" || tileCode == "D4" || tileCode == "D5" || tileCode == "D6" || tileCode == "D7" || tileCode == "DB" ||
             tileCode == "21" || tileCode == "21" || tileCode == "23" || tileCode == "24" || tileCode == "25" || tileCode == "26" || tileCode == "27") {
        return g_groundTexture;
    }
    else if (tileCode == "D1" || tileCode == "D2") {
        return g_backgroundTexture3;
    }
    else if (tileCode == "OP") {
        return g_playerJumpTexture;
    }
    else {
        return g_groundTexture;
    }
}

// Helper function: Set color based on tile code
void SetTileColor(const std::string& tileCode) {
    if (tileCode == "G1") {
        SetColor(0.4f, 0.8f, 0.3f, 1.0f);
    }
    else if (tileCode == "G2") {
        SetColor(0.6f, 0.4f, 0.2f, 1.0f);
    }
    else if (tileCode == "G3") {
        SetColor(0.5f, 0.5f, 0.5f, 1.0f);
    }
    else if (tileCode == "W1") {
        SetColor(0.7f, 0.3f, 0.2f, 1.0f);
    }
    else if (tileCode == "W2") {
        SetColor(0.4f, 0.4f, 0.4f, 1.0f);
    }
    else if (tileCode == "P1") {
        SetColor(0.8f, 0.6f, 0.3f, 1.0f);
    }
    else if (tileCode == "P2") {
        SetColor(0.7f, 0.7f, 0.8f, 1.0f);
    }
    else if (tileCode == "PF") {
        SetColor(0.3f, 0.7f, 0.3f, 1.0f);
    }
    else if (tileCode == "PI") {
        SetColor(0.3f, 0.5f, 0.9f, 1.0f);
    }
    else if (tileCode == "PT") {
        SetColor(0.8f, 0.3f, 0.8f, 1.0f);
    }
    else if (tileCode == "D1") {
        SetColor(0.2f, 0.5f, 0.1f, 1.0f);
    }
    else if (tileCode == "D2") {
        SetColor(0.5f, 0.5f, 0.5f, 1.0f);
    }
    else {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
}
void DrawGame() {
    int currentWidth = g_camera.GetWidth();
    int currentHeight = g_camera.GetHeight();
    float aspectRatio = static_cast<float>(currentWidth) / static_cast<float>(currentHeight);

    float cameraX = g_camera.GetX();
    float cameraY = g_camera.GetY();

    auto worldToScreen = [cameraX, cameraY](float worldX, float worldY) -> std::pair<float, float> {
        return { worldX - cameraX, worldY - cameraY };
        };

    if (g_hitEffectTexture) {
        for (const auto& e : g_weakPointHitEffects) {
            if (!e.active) continue;
            std::pair<float, float> screenPos = worldToScreen(e.x, e.y);
            SetColor(1.0f, 1.0f, 1.0f, 1.0f);
            float size = 0.25f * 1.5f;
            RenderImage(screenPos.first - size * 0.5f, screenPos.second - size * 0.5f,
                size, size, g_hitEffectTexture,
                e.frame, WEAKPOINT_HIT_EFFECT_ROWS, WEAKPOINT_HIT_EFFECT_COLUMNS);
        }
    }

    // Draw background (with parallax effect)
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    float bgOffsetX = cameraX * 0.3f;
    float bgOffsetY = cameraY * 0.3f;
    RenderImage(-1.0f + bgOffsetX, -1.0f + bgOffsetY, 2.0f, 2.0f, g_backgroundTexture1, 0, 1, 1);

    // Use new map system to draw tiles
    if (g_mapManager.IsMapLoaded()) {
        Map* currentMap = g_mapManager.GetCurrentMap();

        // Draw background layer tiles
        auto& bgTiles = currentMap->GetTiles(MapLayer::BACKGROUND);
        for (const auto& tile : bgTiles) {
            if (tile.tileInfo.code == "00") continue;
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
        }

        // Draw midground layer tiles (player activity layer)
        auto& mgTiles = currentMap->GetTiles(MapLayer::MIDGROUND);
        for (const auto& tile : mgTiles) {
            if (tile.tileInfo.code == "00") continue;
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
        }

        // Draw foreground layer tiles
        auto& fgTiles = currentMap->GetTiles(MapLayer::FOREGROUND);
        for (const auto& tile : fgTiles) {
            if (tile.tileInfo.code == "00") continue;
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
        }
    }

    // Draw charge effect
    if (g_player.isCharging && !g_player.isDead) {
        float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;
        float effectSize = PLAYER_WIDTH * (1.0f + chargeRatio * 1.0f);
        float alpha = 0.3f + chargeRatio * 0.7f;

        std::pair<float, float> effectPos = worldToScreen(
            g_player.posX - (effectSize - PLAYER_WIDTH) * 0.5f,
            g_player.posY - (effectSize - PLAYER_HEIGHT) * 0.5f
        );

        float r = 1.0f;
        float g = 1.0f - chargeRatio * 0.5f;
        float b = 0.0f;
        SetColor(r, g, b, alpha);
        RenderImage(effectPos.first, effectPos.second, effectSize, effectSize, g_chargeEffectTexture, 0, 1, 1);
    }

    // Draw dash effect
    if (g_player.isDashing && !g_player.isDead) {
        float dashProgress = 1.0f - (g_player.dashTimer / DASH_DURATION);
        float effectSize = PLAYER_WIDTH * (1.2f + dashProgress * 0.3f);
        float alpha = 0.7f + dashProgress * 0.3f;

        std::pair<float, float> dashPos = worldToScreen(
            g_player.posX - (effectSize - PLAYER_WIDTH) * 0.5f,
            g_player.posY - (effectSize - PLAYER_HEIGHT) * 0.5f
        );

        SetColor(1.0f, 0.3f, 0.3f, alpha);
        RenderImage(dashPos.first, dashPos.second, effectSize, effectSize, g_dashEffectTexture, 0, 1, 1);
    }

    RenderEnemies(g_camera);
    g_mouseIndicator.Render(g_camera.GetX(), g_camera.GetY());

    g_projectileManager.Render(g_camera);
    // Draw player
    if (!g_player.isDead) {
        // Normal drawing when alive
        std::pair<float, float> playerPos = worldToScreen(g_player.posX, g_player.posY);

        // for the size of the character
        float scale = 6.6f;
        float width = PLAYER_WIDTH * scale;
        float height = PLAYER_HEIGHT * scale;

        // center the bigger sprite on collision box
        float offsetX = (width - PLAYER_WIDTH) * 0.5f;
        float offsetY = (height - PLAYER_HEIGHT) * 0.5f;

        // 获取当前动画剪辑的纹理
        ID3D11ShaderResourceView* currentTexture = g_player.anim.GetCurrentClipTexture();
        int frameIndex = g_player.anim.GetCurrentFrame();
        int splitX = g_player.anim.GetSplitX();
        int splitY = g_player.anim.GetSplitY();

        // 根据朝向决定是否水平翻转
        // 如果facingRight为true（面向右），不翻转
        // 如果facingRight为false（面向左），水平翻转
        bool flipHorizontal = !g_player.facingRight;

        // Set color based on invincibility state
        if (g_player.isInvincible) {
            // Yellow when invincible
            SetColor(1.0f, 1.0f, 0.0f, 1.0f);
        }
        else {
            // Normal white
            SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        }

        RenderImage(playerPos.first - offsetX, playerPos.second - offsetY, width, height,
            currentTexture, frameIndex, splitY, splitX, true, 0.0f, flipHorizontal);

        SetColor(1.0f, 1.0f, 1.0f, 1.0f);  // Reset color after being invincible
    }
    else {
        // Death animation
        std::pair<float, float> playerPos = worldToScreen(g_player.posX, g_player.posY);

        float scale = 6.6f;
        float width = PLAYER_WIDTH * scale;
        float height = PLAYER_HEIGHT * scale;

        float offsetX = (width - PLAYER_WIDTH) * 0.5f;
        float offsetY = (height - PLAYER_HEIGHT) * 0.5f;

        // Get death animation texture and frame
        ID3D11ShaderResourceView* currentTexture = g_player.anim.GetCurrentClipTexture();
        int frameIndex = g_player.anim.GetCurrentFrame();
        int splitX = g_player.anim.GetSplitX();
        int splitY = g_player.anim.GetSplitY();

        bool flipHorizontal = !g_player.facingRight;

        // Render death animation
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(playerPos.first - offsetX, playerPos.second - offsetY, width, height,
            currentTexture, frameIndex, splitY, splitX, true, 0.0f, flipHorizontal);
    }

    DrawComboUI();
    DrawGaugeUI();
    DrawScoreUI();
    
}
void HandleInput() {
    if (g_inputSystem.IsResetting()) {
        ResetGame();
    }
   /* if (g_inputSystem.IsMouseRightDown()) {
        CancelChargeDash();
    }*/
    // Right click: Activate invincibility if gauge is full
    if (g_inputSystem.IsMouseRightDown()) {
        if (g_player.gaugePoints >= g_player.MAX_GAUGE_POINTS && !g_player.isInvincible) {
            // Activate invincibility
            g_player.isInvincible = true;
            g_player.invincibleTimer = g_player.INVINCIBLE_DURATION;
            g_player.gaugePoints = 0;  // Reset gauge
        }
        else {
            CancelChargeDash();  // Original right-click behavior
        }
    }
    // for pausing the game press P or Esc key
    if (g_inputSystem.IsTogglePressed(VK_P) || g_inputSystem.IsTogglePressed(VK_ESCAPE))
    {
        SCENE currentScene = sceneManager.GetCurrentSceneType();

        if (currentScene == GAMEPLAY || currentScene == CAKE) // the areas and the cake scene
        {
            sceneManager.SwitchScene(PAUSE);  // you can pause the game at any stage
        }

        // if you press P or Esc key again you can go back to the stage (you can use the mouse and click the continue button
        else if (currentScene == PAUSE)
        {
            SCENE previousScene = sceneManager.GetOriginalPausedScene();
            if (previousScene == GAMEPLAY || previousScene == CAKE) // the areas and the cake scene
            {
                sceneManager.SwitchScene(previousScene);
            }
        }
    }

    // Get mouse input state
    bool isMouseLeftPressed = g_inputSystem.IsMouseLeftPressed();
    bool isMouseLeftDown = g_inputSystem.IsMouseLeftDown();
    bool isMouseLeftReleased = g_inputSystem.IsMouseLeftReleased();

    static bool wasMouseLeftDown = false;

    // Pure mouse control: press to start charging
    if (isMouseLeftPressed) {
        // If we already have a saved charge, pressing should dash immediately
        // (instead of waiting for release). Reuse existing dash execution logic
        // by starting a charge with a near-zero charge time.
        if (g_player.hasSavedCharge && !g_player.isCharging) {
            StartMouseChargeDash();
            g_player.chargeTime = 0.0f;
            ExecuteMouseChargeDash();
        }
        else {
            StartMouseChargeDash();
        }
    }

    // Pure mouse control: release to execute dash
    if (isMouseLeftReleased && wasMouseLeftDown && g_player.isCharging) {
        ExecuteMouseChargeDash();
    }

    // Cancel charging
    if (!isMouseLeftDown && g_player.isCharging) {
        CancelChargeDash();
    }

    wasMouseLeftDown = isMouseLeftDown;

    // Movement control
    bool moving = false;
    if (g_inputSystem.IsMovingLeft()) {
        if (!g_player.isDashing) {
            g_player.velocityX = -MOVE_SPEED;
        }
        g_player.isMoving = true;
        g_player.facingRight = false;
        moving = true;
    }
    if (g_inputSystem.IsMovingRight()) {
        if (!g_player.isDashing) {
            g_player.velocityX = MOVE_SPEED;
        }
        g_player.isMoving = true;
        g_player.facingRight = true;
        moving = true;
    }

    if (!moving && !g_player.isDashing) {
        g_player.velocityX = 0.0f;
        g_player.isMoving = false;
    }

    // Jump control
    static bool wasJumpKeyPressed = false;
    bool isJumpKeyPressed = g_inputSystem.IsJumping();
    if (isJumpKeyPressed && !wasJumpKeyPressed) {
        Jump();
    }
    wasJumpKeyPressed = isJumpKeyPressed;
}

// MouseIndicatorSystem implementation
void MouseIndicatorSystem::Initialize() {
    m_mouseIndicatorTexture = g_chargeEffectTexture;
    m_cursorTexture = g_cursorTexture;
    m_showMouseIndicator = true;
    m_arrowAngle = 0.0f;
}

void MouseIndicatorSystem::Update(float deltaTime) {
    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY);

    m_mouseWorldX = mouseX;
    m_mouseWorldY = mouseY;

    float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;

    float deltaX = m_mouseWorldX - playerCenterX;
    float deltaY = m_mouseWorldY - playerCenterY;

    m_arrowAngle = atan2(deltaY, deltaX);

    static int debugCounter = 0;
    if (debugCounter++ % 60 == 0) {
        printf("Mouse World: (%.2f, %.2f), Player: (%.2f, %.2f)\n",
            m_mouseWorldX, m_mouseWorldY, playerCenterX, playerCenterY);
    }
}

void MouseIndicatorSystem::Render(float cameraX, float cameraY) {
    if (!m_showMouseIndicator) return;
    // Do not show mouse indicator if protagonist is dead
    if (g_player.isDead) return;
    auto worldToScreen = [cameraX, cameraY](float worldX, float worldY) -> std::pair<float, float> {
        return { worldX - cameraX, worldY - cameraY };
        };

    // Draw mouse position indicator (original code)
    float indicatorSize = 0.1f;
    float cursorWidth = 0.1f;
    float cursorHeight = 0.15f;
    auto mousePos = worldToScreen(m_mouseWorldX - indicatorSize / 2, m_mouseWorldY - indicatorSize / 2);

    SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    RenderImage(mousePos.first, mousePos.second, cursorWidth, cursorHeight,
        m_cursorTexture, 0, 1, 1, false, 0);

    // Fixed display of dash points in top right corner of screen
    float dashPointsX = 0.9f; // Right side of screen
    float dashPointsY = 0.1f; // Top of screen
    float digitWidth = 0.08f;
    float digitHeight = 0.12f;

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Render dash points
    RenderNumber(g_player.dashPoints, dashPointsX, dashPointsY, digitWidth, digitHeight, pTextureNum);


    float uiX = -1.0f;
    float uiY = 0.4f;
    float uiWidth = 0.6f;
    float uiHeight = 0.8f;
    RenderImage(uiX, uiY, uiWidth, uiHeight, g_uiNumberTexture, 0, 1, 1);

    // for the timer counting
    float timerX = -0.83f;  // position x axis
    float timerY = 0.77f;   // position y axis
    float timerDigitWidth = 0.05f;  // width
    float timerDigitHeight = 0.08f; // height

    // for the minutes
    int minuteTens = g_gameMinutes / 10;
    int minuteOnes = g_gameMinutes % 10;
    RenderNumber(minuteTens, timerX, timerY, timerDigitWidth, timerDigitHeight, pTextureNum);
    RenderNumber(minuteOnes, timerX + timerDigitWidth * 1.2f, timerY, timerDigitWidth, timerDigitHeight, pTextureNum);

    // for the seconds
    int secondTens = g_gameSeconds / 10;
    int secondOnes = g_gameSeconds % 10;
    RenderNumber(secondTens, timerX + timerDigitWidth * 2.8f, timerY, timerDigitWidth, timerDigitHeight, pTextureNum);
    RenderNumber(secondOnes, timerX + timerDigitWidth * 4.0f, timerY, timerDigitWidth, timerDigitHeight, pTextureNum);

    //SetColor(1.0f, 1.0f, 1.0f, 1.0f); 

    // Draw direction arrow

    if (g_player.isCharging || g_player.hasSavedCharge)
    {
        if (!m_arrowShow) {

        }
        else {

            float centerOffsetX = 0.003f;
            float centerOffsetY = 0.0f;
            float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f + centerOffsetX;
            float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f + centerOffsetY;
            float arrowWidth = 0.15f;

            // Get charge time
            float chargeTime = 0.0f;
            if (g_player.isCharging) {
                chargeTime = g_player.chargeTime;
            }
            else if (g_player.hasSavedCharge) {
                chargeTime = g_player.savedChargeTime;
            }

            // Calculate smooth charge ratio (0.0 to 1.0)
            float chargeRatio = chargeTime / g_player.MAX_CHARGE_TIME;

            // Smoothly interpolate speed multiplier from 1.0 to 2.0 based on charge
            float minSpeedMultiplier = 1.0f;
            float maxSpeedMultiplier = 2.0f;
            float speedMultiplier = minSpeedMultiplier + (maxSpeedMultiplier - minSpeedMultiplier) * chargeRatio;

            float durationMultiplier = 1.0f;

            // Calculate the ACTUAL world distance the player will travel
            // Distance = velocity × time × 60.0 (from your physics calculation)
            float dashSpeed = DASH_SPEED * speedMultiplier;
            float dashDuration = DASH_DURATION * durationMultiplier;
            float arrowLength = dashSpeed * dashDuration * 60.0f; // Added the 60.0f multiplier!

            float tailX = playerCenterX;
            float tailY = playerCenterY;

            // Calculate the center of the arrow (midpoint between tail and head)
            float arrowCenterX = tailX + cosf(m_arrowAngle) * (arrowLength * 0.5f);
            float arrowCenterY = tailY + sinf(m_arrowAngle) * (arrowLength * 0.5f);

            // Position calculation for screen rendering
            auto arrowScreenPos = worldToScreen(arrowCenterX - arrowLength * 0.5f, arrowCenterY - arrowWidth * 0.5f);

            // Get charge level for color display
            int chargeLevel = g_player.GetChargeLevelFromTime(chargeTime);

            // Display different colors based on charge level
            if (chargeLevel >= 3) {
                //SetColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
            }
            else if (chargeLevel >= 2) {
                //SetColor(0.0f, 0.0f, 1.0f, 1.0f); // Dark blue
            }
            else if (chargeLevel >= 1) {
                //SetColor(0.0f, 1.0f, 1.0f, 1.0f); // Blue
            }

            SetColor(1.0f, 1.0f, 1.0f, 1.0f);
            // Render with arrowLength (horizontal stretch) and arrowWidth (vertical size)
            RenderImage(arrowScreenPos.first, arrowScreenPos.second, arrowLength, arrowWidth,
                g_arrowTexture, 0, 1, 1, false, m_arrowAngle);

            SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}

void MouseIndicatorSystem::Cleanup() {
    m_mouseIndicatorTexture = nullptr;
}

void MouseIndicatorSystem::ShowMouseIndicator(bool i) {
}


// Reset all statistics
void GameStatistics::Reset() {
    enemiesKilled = 0;
    weakPointKills = 0;
    totalDeaths = 0;
    totalTime = 0.0f;
    totalScore = 0;
}

// Increment kill counter
void GameStatistics::IncrementKills() {
    enemiesKilled++;
}

// Increment weak point kill counter
void GameStatistics::IncrementWeakPointKills() {
    weakPointKills++;
}

// Increment death counter
void GameStatistics::IncrementDeaths() {
    totalDeaths++;
}

// Update total time
void GameStatistics::UpdateTime(float time) {
    totalTime = time;
}

// Calculate the final score based on kills, time, and deaths
void GameStatistics::CalculateFinalScore() {
    // the kill points: normal kills = 10, weak point kills = 30
    int killPoints = (enemiesKilled * 10) + (weakPointKills * 30);

    // the time bonus. 0 is the minimum
    int timePenalty = static_cast<int>(totalTime); // -1 point per seconds that has elapsed

    // the death penalty: deaths * 5
    int deathPenalty = totalDeaths * 5;

    totalScore = std::max(0, killPoints - timePenalty - deathPenalty);

    // the debug
 /*   char debugMsg[512];
    sprintf_s(debugMsg,
        "=== SCORE CALCULATION ===\n"
        "Enemies Killed: %d (normal)\n"
        "Weak Point Kills: %d\n"
        "Total Deaths: %d\n"
        "Total Time: %.2f seconds\n"
        "---\n"
        "Kill Points: (%d * 10) + (%d * 30) = %d\n"
        "Time Bonus: max(0, 60 - %.2f) = %d\n"
        "Death Penalty: %d * 5 = %d\n"
        "---\n"
        "Final Score: (%d * %d) - %d = %d\n"
        "========================\n",
        enemiesKilled, weakPointKills,
        totalDeaths, totalTime,
        killPoints,
        timePenalty,
        deathPenalty,
        killPoints, timePenalty, deathPenalty, totalScore
    );
    OutputDebugStringA(debugMsg);*/
}

void GameStatistics::AddScore(int points) {
    totalScore += points;
    if (totalScore < 0) {
        totalScore = 0; // so there will not be negative score
    }
}
