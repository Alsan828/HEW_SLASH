#include "Game.h"
#include "Enemy.h"
#include "SimpleAudio.h"

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
    if (g_mapManager.IsMapLoaded()) {
        g_mapManager.ReloadCurrentMap();
    }
    g_gameState = STATE_PLAYING;

}

// added december 11th
void CleanUpGameWorld()
{
    g_projectileManager.ClearAll();
    CleanupEnemies();
    g_mouseIndicator.Cleanup();

    // Release all textures
    if (g_playerTexture) {
        g_playerTexture->Release();
        g_playerTexture = nullptr;
    }
    
    // for the character
    if (g_playerIdleLeftTexture) {
        g_playerIdleLeftTexture->Release();
        g_playerIdleLeftTexture = nullptr;
    }
    if (g_playerIdleRightTexture) {
        g_playerIdleRightTexture->Release();
        g_playerIdleRightTexture = nullptr;
    }
    if (g_playerJumpRightTexture) {
        g_playerJumpRightTexture->Release();
        g_playerJumpRightTexture = nullptr;
    }
    if (g_playerJumpLeftTexture) {
        g_playerJumpLeftTexture->Release();
        g_playerJumpLeftTexture = nullptr;
    }
    if (g_playerRunRightTexture) {
        g_playerRunRightTexture->Release();
        g_playerRunRightTexture = nullptr;
    }
    if (g_playerRunLeftTexture) {
        g_playerRunLeftTexture->Release();
        g_playerRunLeftTexture = nullptr;
    }
    if (g_playerSlashRight1Texture) {
        g_playerSlashRight1Texture->Release();
        g_playerSlashRight1Texture = nullptr;
    }
    if (g_playerSlashLeft1Texture) {
        g_playerSlashLeft1Texture->Release();
        g_playerSlashLeft1Texture = nullptr;
    }
    if (g_playerSlashLeft2Texture) {
        g_playerSlashLeft2Texture->Release();
        g_playerSlashLeft2Texture = nullptr;
    }
    if (g_playerSlashRight2Texture) {
        g_playerSlashRight2Texture->Release();
        g_playerSlashRight2Texture = nullptr;
    }
    if (g_playerSlashLeft3Texture) {
        g_playerSlashLeft3Texture->Release();
        g_playerSlashLeft3Texture = nullptr;
    }
    if (g_playerSlashRight3Texture) {
        g_playerSlashRight3Texture->Release();
        g_playerSlashRight3Texture = nullptr;
    }
    if (g_playerSlashLeft4Texture) {
        g_playerSlashLeft4Texture->Release();
        g_playerSlashLeft4Texture = nullptr;
    }
    if (g_playerSlashRight4Texture) {
        g_playerSlashRight4Texture->Release();
        g_playerSlashRight4Texture = nullptr;
    }
    if (g_playerAirChargeLeftTexture) {
        g_playerAirChargeLeftTexture->Release();
        g_playerAirChargeLeftTexture = nullptr;
    }
    if (g_playerAirChargeRightTexture) {
        g_playerAirChargeRightTexture->Release();
        g_playerAirChargeRightTexture = nullptr;
    }
    if (g_playerFallingLeftTexture) {
        g_playerFallingLeftTexture->Release();
        g_playerFallingLeftTexture = nullptr;
    }
    if (g_playerFallingRightTexture) {
        g_playerFallingRightTexture->Release();
        g_playerFallingRightTexture = nullptr;
    }
    if (g_playerGroundChargeLeftTexture) {
        g_playerGroundChargeLeftTexture->Release();
        g_playerGroundChargeLeftTexture = nullptr;
    }
    if (g_playerGroundChargeRightTexture) {
        g_playerGroundChargeRightTexture->Release();
        g_playerGroundChargeRightTexture = nullptr;
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

}

// Improved collision detection function
bool CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
        y1 < y2 + h2 && y1 + h1 > y2);
}
// Game initialization
void InitGameWorld() {

    g_projectileManager.LoadTextures(g_pDevice);
    // for the character
    LoadTexture(g_pDevice, "asset/character/idle_left.png", &g_playerIdleLeftTexture);    // when looking left
    LoadTexture(g_pDevice, "asset/character/idle_right.png", &g_playerIdleRightTexture);  // when looking right
    LoadTexture(g_pDevice, "asset/character/jump_left.png", &g_playerJumpLeftTexture);  // when jumping left
    LoadTexture(g_pDevice, "asset/character/jump_right.png", &g_playerJumpRightTexture);  // when jumping right
    LoadTexture(g_pDevice, "asset/character/run_left.png", &g_playerRunLeftTexture);  // when running left
    LoadTexture(g_pDevice, "asset/character/run_right.png", &g_playerRunRightTexture);  // when running right
    LoadTexture(g_pDevice, "asset/character/slash_left1.png", &g_playerSlashLeft1Texture);  // when slagh 1 left
    LoadTexture(g_pDevice, "asset/character/slash_right1.png", &g_playerSlashRight1Texture);  // when salsh1 right
    LoadTexture(g_pDevice, "asset/character/slash_left2.png", &g_playerSlashLeft2Texture);  // when slash2 left
    LoadTexture(g_pDevice, "asset/character/slash_right2.png", &g_playerSlashRight2Texture);  // when slash2 right
    LoadTexture(g_pDevice, "asset/character/slash_left3.png", &g_playerSlashLeft3Texture);  // when slash3 left
    LoadTexture(g_pDevice, "asset/character/slash_right3.png", &g_playerSlashRight3Texture);  // when slash3 right
    LoadTexture(g_pDevice, "asset/character/slash_left4.png", &g_playerSlashLeft4Texture);  // when slash4 left
    LoadTexture(g_pDevice, "asset/character/slash_right4.png", &g_playerSlashRight4Texture);  // when slash4 right
    LoadTexture(g_pDevice, "asset/character/air_charge_left.png", &g_playerAirChargeLeftTexture);  // when air charge left
    LoadTexture(g_pDevice, "asset/character/air_charge_right.png", &g_playerAirChargeRightTexture);  // when air charge right
    LoadTexture(g_pDevice, "asset/character/falling_left.png", &g_playerFallingLeftTexture);  // when falling left
    LoadTexture(g_pDevice, "asset/character/falling_right.png", &g_playerFallingRightTexture);  // when falling right
    LoadTexture(g_pDevice, "asset/character/ground_charge_left.png", &g_playerGroundChargeLeftTexture);  // when ground charge left
    LoadTexture(g_pDevice, "asset/character/ground_charge_right.png", &g_playerGroundChargeRightTexture);  // when ground charge right
    g_player.anim.AddClip("IdleLeft", 0, 3, 4, 1, 0.25f, true, g_playerIdleLeftTexture); // ※ splitX is number of columns and splitY is number of rows
    g_player.anim.AddClip("IdleRight", 0, 3, 4, 1, 0.25f, true, g_playerIdleRightTexture);
    g_player.anim.AddClip("JumpLeft", 0, 3, 4, 1, 0.25f, false, g_playerJumpLeftTexture);
    g_player.anim.AddClip("JumpRight", 0, 3, 4, 1, 0.25f, false, g_playerJumpRightTexture);
    g_player.anim.AddClip("RunLeft", 0, 3, 4, 1, 0.1f, true, g_playerRunLeftTexture);
    g_player.anim.AddClip("RunRight", 0, 3, 4, 1, 0.1f, true, g_playerRunRightTexture);
    g_player.anim.AddClip("SlashLeft1", 0, 3, 4, 1, 0.06f, false, g_playerSlashLeft1Texture);
    g_player.anim.AddClip("SlashRight1", 0, 3, 4, 1, 0.06f, false, g_playerSlashRight1Texture);
    g_player.anim.AddClip("SlashLeft2", 0, 3, 4, 1, 0.06f, false, g_playerSlashLeft2Texture);
    g_player.anim.AddClip("SlashRight2", 0, 3, 4, 1, 0.06f, false, g_playerSlashRight2Texture);
    g_player.anim.AddClip("SlashLeft3", 0, 3, 4, 1, 0.06f, false, g_playerSlashLeft3Texture);
    g_player.anim.AddClip("SlashRight3", 0, 3, 4, 1, 0.06f, false, g_playerSlashRight3Texture);
    g_player.anim.AddClip("SlashLeft4", 0, 3, 4, 1, 0.06f, false, g_playerSlashLeft4Texture);
    g_player.anim.AddClip("SlashRight4", 0, 3, 4, 1, 0.06f, false, g_playerSlashRight4Texture);
    g_player.anim.AddClip("AirChargeLeft", 0, 0, 1, 1, 0.25f, true, g_playerAirChargeLeftTexture);
    g_player.anim.AddClip("AirChargeRight", 0, 0, 1, 1, 0.25f, true, g_playerAirChargeRightTexture);
    g_player.anim.AddClip("FallingLeft", 0, 0, 1, 1, 0.25f, true, g_playerFallingLeftTexture);
    g_player.anim.AddClip("FallingRight", 0, 0, 1, 1, 0.25f, true, g_playerFallingRightTexture);
    g_player.anim.AddClip("GroundChargeLeft", 0, 0, 1, 1, 0.25f, true, g_playerGroundChargeLeftTexture);
    g_player.anim.AddClip("GroundChargeRight", 0, 0, 1, 1, 0.25f, true, g_playerGroundChargeRightTexture);

    LoadTexture(g_pDevice, "asset/platform/platformtest.png", &g_groundTexture);
    LoadTexture(g_pDevice, "asset/background/1-6background.png", &g_backgroundTexture1);
    LoadTexture(g_pDevice, "asset/effect/dash.png", &g_dashEffectTexture);
    LoadTexture(g_pDevice, "asset/effect/hit.png", &g_chargeEffectTexture);
    LoadTexture(g_pDevice, "asset/effect/hit.png", &g_hitEffectTexture);

    LoadTexture(g_pDevice, "asset/UI/number.png", &g_numberTexture);
    LoadTexture(g_pDevice, "asset/UI/time1.png", &g_uiNumberTexture);

    LoadTexture(g_pDevice, "asset/UI/arrow.png", &g_arrowTexture);
    LoadTexture(g_pDevice, "asset/UI/cursor.png", &g_cursorTexture);

    InitEnemies();
    g_mapManager.InitializeMaps();
    g_mouseIndicator.Initialize();

    g_camera.SetSmoothness(camera_Smoothness);
    g_camera.SetLookAhead(camera_LookAhead);
    g_camera.SetDeadZone(camera_DeadZone);

    g_audioManager.Initialize();
    g_audioManager.PlayBGM("asset/Music/level1.wav");
    ResetGame();
}

// Modified game update function
void UpdateGame(float deltaTime) {
    if (g_gameState != STATE_PLAYING) {
        return;
    }

    // Update audio manager
    g_audioManager.Update(deltaTime);
    g_gameTimer.Tick(); // added december 3rd

    // added december 4th
    g_gameElapsedTime += deltaTime;
    g_gameMinutes = static_cast<int>(g_gameElapsedTime) / 60;
    g_gameSeconds = static_cast<int>(g_gameElapsedTime) % 60;

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

    float scaledDeltaTime = deltaTime * timeScale;

    // Update game logic using adjusted time
    UpdateDash(deltaTime);
    g_camera.Update(scaledDeltaTime);
    UpdatePlayerPhysics(scaledDeltaTime);
    UpdateEnemies(scaledDeltaTime, &g_mapManager);
    // Update all projectiles
    g_projectileManager.Update(scaledDeltaTime, &g_mapManager, g_enemies);

    // for the character animation
    if (g_player.animLockTimer > 0.0f) 
    { 
        g_player.animLockTimer -= deltaTime; 
    }
    if (g_player.animLockTimer <= 0.0f)
    {
        if (g_player.isCharging) // if player is charging
        {
            if (!g_player.isOnGround) // if the player is not on the ground while charging
            {
                if (g_player.facingRight) // when looking right
                {
                    if (g_player.anim.GetCurrentClipName() != "AirChargeRight") { // if not doing the animation
                        g_player.anim.SetClip("AirChargeRight"); // do the animation
                    }
                }
                else // when looking left
                {
                    if (g_player.anim.GetCurrentClipName() != "AirChargeLeft") { // if not doing the animation
                        g_player.anim.SetClip("AirChargeLeft"); // do the animation
                    }
                }
            }
            else // if the player is on the ground while charging
            {
                if (g_player.facingRight) // when looking right
                {
                    if (g_player.anim.GetCurrentClipName() != "GroundChargeRight") { // if not doing the animation
                        g_player.anim.SetClip("GroundChargeRight"); // do the animation
                    }
                }
                else  // when looking left
                {
                    if (g_player.anim.GetCurrentClipName() != "GroundChargeLeft") { // if not doing the animation
                        g_player.anim.SetClip("GroundChargeLeft"); // do the animation
                    }
                }
            }
        }
        else if (g_player.isDashing) // if the character is dashing
        {
            // for the types of slash
            int chargeType = g_player.GetChargeLevel();

            if (g_player.facingRight) // when the character is facing right
            {
                if (chargeType == 0) // for the type 1 slash 
                {
                    if (g_player.anim.GetCurrentClipName() != "SlashRight1") { // if not doing the animation
                        g_player.anim.SetClip("SlashRight1"); // do the animation
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
                else if (chargeType == 1) // for the type 2 slash
                {
                    if (g_player.anim.GetCurrentClipName() != "SlashRight2") { // if not doing the animation
                        g_player.anim.SetClip("SlashRight2"); // do the animation
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
                else if (chargeType == 2) // for the type 3 slash
                {
                    if (g_player.anim.GetCurrentClipName() != "SlashRight3") { // if not doing the animation
                        g_player.anim.SetClip("SlashRight3"); // do the animation
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
                else  // for the type 4 slash
                {
                    if (g_player.anim.GetCurrentClipName() != "SlashRight4") { // if not doing the animation
                        g_player.anim.SetClip("SlashRight4"); // do the animation
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
            }
            else  // when the character is looking left
            {
                if (chargeType == 0) // for the type 1 slash
                {
                    if (g_player.anim.GetCurrentClipName() != "SlashLeft1") { // if not doing the animation
                        g_player.anim.SetClip("SlashLeft1"); // do the animation
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
                else if (chargeType == 1) // for the type 2 slash
                {
                    if (g_player.anim.GetCurrentClipName() != "SlashLeft2") { // if not doing the animation
                        g_player.anim.SetClip("SlashLeft2"); // do the animation
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
                else if (chargeType == 2) // for the type 3 slash
                {
                    if (g_player.anim.GetCurrentClipName() != "SlashLeft3") { // if not doing the animation
                        g_player.anim.SetClip("SlashLeft3"); // do the animation
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
                else  // for the type 4 slash
                {
                    if (g_player.anim.GetCurrentClipName() != "SlashLeft4") { // if not doing the animation
                        g_player.anim.SetClip("SlashLeft4"); // do the animation
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
            }
        }
        else if (!g_player.isOnGround) // the character is not on the ground
        {
            // if the player is falling
            if (g_player.velocityY < 0.0f)
            {
                if (g_player.facingRight)  // when looking right
                {
                    if (g_player.anim.GetCurrentClipName() != "FallingRight") { // if not doing the aniamtion
                        g_player.anim.SetClip("FallingRight"); // do the animation
                    }
                }
                else  // when looking left
                {
                    if (g_player.anim.GetCurrentClipName() != "FallingLeft") { // if not doing the animation
                        g_player.anim.SetClip("FallingLeft"); // do the animation
                    }
                }
            }

            else // when player is jumping
            {
                if (g_player.facingRight) // when looking right
                {
                    if (g_player.anim.GetCurrentClipName() != "JumpRight") {
                        g_player.anim.SetClip("JumpRight");
                    }
                }
                else // when looking left
                {
                    if (g_player.anim.GetCurrentClipName() != "JumpLeft") {
                        g_player.anim.SetClip("JumpLeft");
                    }
                }
            }

        }
        else if (g_player.isMoving) // the character is moving
        {
            if (g_player.facingRight) // when looking right right
            {
                if (g_player.anim.GetCurrentClipName() != "RunRight") {
                    g_player.anim.SetClip("RunRight");
                }
            }
            else // when looking left
            {
                if (g_player.anim.GetCurrentClipName() != "RunLeft") {
                    g_player.anim.SetClip("RunLeft");
                }
            }
        }
        else // player is not running (not moving)
        {
            if (g_player.facingRight) // when looking right
            {
                if (g_player.anim.GetCurrentClipName() != "IdleRight") {
                    g_player.anim.SetClip("IdleRight");
                }
            }
            else // when looking left
            {
                if (g_player.anim.GetCurrentClipName() != "IdleLeft") {
                    g_player.anim.SetClip("IdleLeft");
                }
            }
        }
    }

    g_mouseIndicator.Update(scaledDeltaTime);
    g_player.anim.Update(scaledDeltaTime);

    UpdatePlayerDeath(scaledDeltaTime);
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
    else if (tileCode == "DF" || tileCode == "DI" || tileCode == "DT") {
        return g_groundTexture;
    }
    else if (tileCode == "D1" || tileCode == "D2") {
        return g_backgroundTexture3;
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
        int frameIndex = 0;

        if (g_player.isCharging) {
            //SetColor(0.0f, 1.0f, 0.0f, 1.0f);
            frameIndex = 4;
        }
        else if (g_player.isDashing) {
            //SetColor(1.0f, 0.0f, 0.0f, 1.0f);
            frameIndex = 3;
        }
        else if (!g_player.isOnGround) {
           // SetColor(1.0f, 0.5f, 0.0f, 1.0f);
            frameIndex = 2;
        }
        else if (g_player.isMoving) {
            frameIndex = 1;
            if (g_player.facingRight) {
               // SetColor(0.0f, 0.0f, 1.0f, 1.0f);
            }
            else {
               // SetColor(1.0f, 0.0f, 1.0f, 1.0f);
            }
        }
        else {
            //SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        }

        
        // for the size of the character
        float scale = 2.8f;
        float width = PLAYER_WIDTH * scale;
        float height = PLAYER_HEIGHT * scale;

        // center the bigger sprite on collision box
        float offsetX = (width - PLAYER_WIDTH) * 0.5f;
        float offsetY = (height - PLAYER_HEIGHT) * 0.5f;

        frameIndex = g_player.anim.GetCurrentFrame();

        // with this I get the spritesheet. (the row and columns)
        int splitX = g_player.anim.GetSplitX();
        int splitY = g_player.anim.GetSplitY();

    

        // for the idle when looking left
        RenderImage(playerPos.first - offsetX, playerPos.second - offsetY, width, height,
            g_player.anim.GetCurrentClipTexture(), frameIndex, splitY, splitX/*1, 4*/);
    }
    else {
        // Flickering disappearance animation when dead
        std::pair<float, float> playerPos = worldToScreen(g_player.posX, g_player.posY);

        // Death animation duration
        const float DEATH_ANIM_DURATION = 1.0f;

        if (g_player.deathTimer < DEATH_ANIM_DURATION) {
            float animProgress = g_player.deathTimer / DEATH_ANIM_DURATION;

            // Flicker frequency gradually slows down
            float flickerFreq = 20.0f * (1.0f - animProgress);

            // Control flicker effect, gradually disappear in latter half of animation
            if (animProgress < 0.7f) {
                // First half: rapid flickering
                float sinValue = sin(g_player.deathTimer * flickerFreq);
                float alpha = 0.5f + 0.5f * sinValue;

                if (animProgress > 0.3f) {
                    // Middle segment: add red flickering
                    float redIntensity = 0.5f + 0.5f * sin(g_player.deathTimer * 10.0f);
                    SetColor(1.0f, 1.0f - redIntensity, 1.0f - redIntensity, alpha);
                }
                else {
                    // Initial segment: white flickering
                    SetColor(1.0f, 1.0f, 1.0f, alpha);
                }
            }
            else {
                // Latter half: gradually disappear
                float fadeOut = 1.0f - ((animProgress - 0.7f) / 0.3f);
                float alpha = fadeOut * 0.5f;
                SetColor(1.0f, 0.3f, 0.3f, alpha);
            }

            // Gradually shrink
            float scale = 1.0f - animProgress * 0.5f;
            float width = PLAYER_WIDTH * scale;
            float height = PLAYER_HEIGHT * scale;

            // Center position adjustment
            playerPos.first += (PLAYER_WIDTH - width) * 0.5f;
            playerPos.second += (PLAYER_HEIGHT - height) * 0.5f;

            // for this I get the spritesheet. (the row and columns)
            int splitX = g_player.anim.GetSplitX();
            int splitY = g_player.anim.GetSplitY();

            // for the character
            int frameIndex = g_player.anim.GetCurrentFrame();
            RenderImage(playerPos.first, playerPos.second, width, height,
                g_player.anim.GetCurrentClipTexture(), frameIndex, splitY, splitX/*1, 4*/);


            float uiScale = std::min(currentWidth / 1920.0f, currentHeight / 1080.0f);
        }
    }

}
void HandleInput() {
    if (g_inputSystem.IsResetting()) {
        ResetGame();
    }

    // for pausing the game press P or Esc key
    if (g_inputSystem.IsTogglePressed(VK_P) || g_inputSystem.IsTogglePressed(VK_ESCAPE))
    {
        SCENE currentScene = sceneManager.GetCurrentSceneType();

        if (currentScene == STAGE || currentScene == STAGE2 || currentScene == STAGE3) // add more stages here depending on how many stages there are 
        {
            sceneManager.SwitchScene(PAUSE);  // you can pause the game at any stage
        }

        // if you press P or Esc key again you can go back to the stage (you can use the mouse and click the continue button
        else if (currentScene == PAUSE)
        {
            SCENE previousScene = sceneManager.GetOriginalPausedScene();
            if (previousScene == STAGE || previousScene == STAGE2 || previousScene == STAGE3) // add more stages here depending on how many stages there are 
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
        StartMouseChargeDash();
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
    m_arrowTexture = g_arrowTexture;
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
    float originalArrowSize = 0.15f;
    float arrowSize = originalArrowSize;

    // If charging, make arrow longer based on charge level
   float arrowDistance = 0.0f;

   // with this you can change the center position of the arrow (the start point of the arrow "tail")
    float centerOffsetX = 0.003f;
    float centerOffsetY = 0.0f;

    if (g_player.isCharging)  // if the player is charging 
    {
        // Calculate charge ratio (0.0 to 1.0)
        float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;

        // depending on the charcer, the wrrow gets bigger
        arrowSize = originalArrowSize * (1.0f + chargeRatio * 2.0f);
    }

    float playerCenterX = g_player.posX + PLAYER_WIDTH *0.5f + centerOffsetX;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT *0.5f + centerOffsetY;

    float tailX = playerCenterX; 
    float tailY = playerCenterY;

    // so the tail of the arrow stays in place and the head extends forward
    float arrowCenterX = tailX + cosf(m_arrowAngle) * (arrowSize *0.5f);
    float arrowCenterY = tailY + sinf(m_arrowAngle) * (arrowSize *0.5f);

    auto arrowScreenPos = worldToScreen(arrowCenterX - arrowSize *0.5, arrowCenterY - arrowSize *0.5);
  
    //SetColor(0.0f, 1.0f, 0.0f, 1.0f);
    float levelX = 0.85f;
    float levelY = 0.2f;
    float levelSize = 0.05f;
    int chargeLevel = g_player.GetChargeLevelFromTime(g_player.chargeTime);
    if (g_player.hasSavedCharge) {
        chargeLevel = g_player.GetChargeLevelFromTime(g_player.savedChargeTime);
    }
    // Display different colors based on charge level
    if (chargeLevel >= 1) {
        //SetColor(0.0f, 1.0f, 1.0f, 1.0f); // Blue
    }
    if (chargeLevel >= 2) {
        //SetColor(0.0f, 0.0f, 1.0f, 1.0f); // Dark blue
    }
    if (chargeLevel >= 3) {
        //SetColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
    }

    RenderImage(arrowScreenPos.first, arrowScreenPos.second, arrowSize, arrowSize,
        m_arrowTexture, 0, 1, 1, false, m_arrowAngle);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void MouseIndicatorSystem::Cleanup() {
    m_mouseIndicatorTexture = nullptr;
    m_arrowTexture = nullptr;
}

void MouseIndicatorSystem::ShowMouseIndicator(bool i) {
}

// Sound effect functions
void PlayJumpSound() {
    g_audioManager.PlaySFX(SoundEffect::JUMP, 0.5f);
}

void PlayDashSound() {
    g_audioManager.PlaySFX(SoundEffect::DASH, 0.7f);
}

void PlayChargeStartSound() {
    g_audioManager.PlaySFX(SoundEffect::CHARGE_START, 0.4f, true);
}

void PlayChargeReleaseSound() {
    g_audioManager.PlaySFX(SoundEffect::CHARGE_RELEASE, 0.6f);
}

void PlayShootSound() {
    g_audioManager.PlaySFX(SoundEffect::SHOOT, 0.6f);
}

void PlayEnemyHitSound() {
    g_audioManager.PlaySFX(SoundEffect::ENEMY_HIT, 0.5f);
}

void PlayEnemyDeathSound() {
    g_audioManager.PlaySFX(SoundEffect::ENEMY_DEATH, 0.7f);
}

void PlaySlowMotionSound(bool start) {
    if (start) {
        g_audioManager.PlaySFX(SoundEffect::SLOWMO_START, 0.8f);
    }
    else {
        g_audioManager.PlaySFX(SoundEffect::SLOWMO_END, 0.8f);
    }
}

// Background music functions
void PlayStageMusic(int stage) {
    switch (stage) {
    case 1:
        g_audioManager.PlayBGM(BackgroundMusic::LEVEL1, 0.7f, true);
        break;
    case 2:
        g_audioManager.PlayBGM(BackgroundMusic::LEVEL2, 0.7f, true);
        break;
    case 3:
        g_audioManager.PlayBGM(BackgroundMusic::LEVEL3, 0.7f, true);
        break;
    default:
        g_audioManager.PlayBGM(BackgroundMusic::LEVEL1, 0.7f, true);
    }
}

void PlayBossMusic() {
    g_audioManager.PlayBGM(BackgroundMusic::BOSS_BATTLE, 0.8f, true);
}

void PlayVictoryMusic() {
    g_audioManager.StopBGM();
    g_audioManager.PlayBGM(BackgroundMusic::VICTORY, 0.8f, false);
}

void PlayGameOverMusic() {
    g_audioManager.StopBGM();
    g_audioManager.PlayBGM(BackgroundMusic::GAME_OVER, 0.8f, false);
}