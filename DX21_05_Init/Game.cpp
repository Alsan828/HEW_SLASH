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
    LoadTexture(g_pDevice, "asset/character/karen_small48.png", &g_playerTexture);
    g_player.anim.Init(10, 1, 0.15f, 0);
    g_player.anim.AddClip("Idle", 0, 9, 0.25f, true, g_playerTexture);

    LoadTexture(g_pDevice, "asset/platform/platformtest.png", &g_groundTexture);
    LoadTexture(g_pDevice, "asset/background/1-6background.png", &g_backgroundTexture1);
    LoadTexture(g_pDevice, "asset/effect/dash.png", &g_dashEffectTexture);
    LoadTexture(g_pDevice, "asset/effect/hit.png", &g_chargeEffectTexture);
    LoadTexture(g_pDevice, "asset/effect/hit.png", &g_hitEffectTexture);

    LoadTexture(g_pDevice, "asset/UI/number.png", &g_numberTexture);
    LoadTexture(g_pDevice, "asset/UI/time.png", &g_uiNumberTexture);

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
    // Animation state update
    if (g_player.isCharging) {
        if (g_player.anim.GetCurrentClipName() != "Charge") {
            g_player.anim.SetClip("Charge");
        }
    }
    else if (g_player.isDashing) {
        if (g_player.anim.GetCurrentClipName() != "Dash") {
            g_player.anim.SetClip("Dash");
        }
    }
    else if (!g_player.isOnGround) {
        if (g_player.anim.GetCurrentClipName() != "Jump") {
            g_player.anim.SetClip("Jump");
        }
    }
    else if (g_player.isMoving) {
        if (g_player.facingRight) {
            if (g_player.anim.GetCurrentClipName() != "RunRight") {
                g_player.anim.SetClip("RunRight");
            }
        }
        else {
            if (g_player.anim.GetCurrentClipName() != "RunLeft") {
                g_player.anim.SetClip("RunLeft");
            }
        }
    }
    else {
        if (g_player.anim.GetCurrentClipName() != "Idle") {
            g_player.anim.SetClip("Idle");
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
        return g_fastEnemyTexture;
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

    // Draw player
    if (!g_player.isDead) {
        // Normal drawing when alive
        std::pair<float, float> playerPos = worldToScreen(g_player.posX, g_player.posY);
        int frameIndex = 0;

        if (g_player.isCharging) {
            SetColor(0.0f, 1.0f, 0.0f, 1.0f);
            frameIndex = 4;
        }
        else if (g_player.isDashing) {
            SetColor(1.0f, 0.0f, 0.0f, 1.0f);
            frameIndex = 3;
        }
        else if (!g_player.isOnGround) {
            SetColor(1.0f, 0.5f, 0.0f, 1.0f);
            frameIndex = 2;
        }
        else if (g_player.isMoving) {
            frameIndex = 1;
            if (g_player.facingRight) {
                SetColor(0.0f, 0.0f, 1.0f, 1.0f);
            }
            else {
                SetColor(1.0f, 0.0f, 1.0f, 1.0f);
            }
        }
        else {
            SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        }

        frameIndex = g_player.anim.GetCurrentFrame();

        RenderImage(playerPos.first, playerPos.second, PLAYER_WIDTH, PLAYER_HEIGHT,
            g_player.anim.GetCurrentClipTexture(), frameIndex, 1, 1);
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

            RenderImage(playerPos.first, playerPos.second, PLAYER_WIDTH, PLAYER_HEIGHT,
                g_playerTexture, 0, 1, 10);

            g_projectileManager.Render(g_camera);

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


    float uiX = -1.1f;
    float uiY = 0.3f;
    float uiWidth = 0.78f;
    float uiHeight = 1.0f;
    RenderImage(uiX, uiY, uiWidth, uiHeight, g_uiNumberTexture, 0, 1, 1);

    // for the timer counting
    float timerX = -0.83f;  // position x axis
    float timerY = 0.75f;   // position y axis
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
    float arrowDistance = 0.08f;
    float arrowSize = 0.15f;

    float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;

    float arrowX = playerCenterX + cosf(m_arrowAngle) * arrowDistance;
    float arrowY = playerCenterY + sinf(m_arrowAngle) * arrowDistance;

    auto arrowScreenPos = worldToScreen(arrowX - arrowSize / 2, arrowY - arrowSize / 2);

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
        SetColor(0.0f, 1.0f, 1.0f, 1.0f); // Blue
    }
    if (chargeLevel >= 2) {
        SetColor(0.0f, 0.0f, 1.0f, 1.0f); // Dark blue
    }
    if (chargeLevel >= 3) {
        SetColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
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