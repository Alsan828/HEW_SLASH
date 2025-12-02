#include "Game.h"
#include "Enemy.h"

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

void ResetGame() {
    if (g_mapManager.IsMapLoaded()) {
        g_mapManager.RespawnPlayer(-1);
    }
    g_gameState = STATE_PLAYING;
    CleanupEnemies();
}

// Improved collision detection function
bool CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
        y1 < y2 + h2 && y1 + h1 > y2);
}

// Game initialization
void InitGameWorld() {
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_playerTexture);
    g_player.anim.Init(10, 1, 0.15f, 0);
    g_player.anim.AddClip("Idle", 0, 9, 0.25f, true);

    LoadTexture(g_pDevice, "asset/blockB.png", &g_groundTexture);
    LoadTexture(g_pDevice, "asset/Space.png", &g_backgroundTexture);
    LoadTexture(g_pDevice, "asset/block.png", &g_dashEffectTexture);
    LoadTexture(g_pDevice, "asset/completed.png", &g_chargeEffectTexture);

    InitEnemies();
    g_mapManager.InitializeMaps();
    g_mouseIndicator.Initialize();

    g_camera.SetSmoothness(camera_Smoothness);
    g_camera.SetLookAhead(camera_LookAhead);
    g_camera.SetDeadZone(camera_DeadZone);

    ResetGame();
}

// Modified game update function
void UpdateGame(float deltaTime) {
    if (g_gameState != STATE_PLAYING) {
        return;
    }

    // 应用时间减缓效果（如果正在蓄力）
    float timeScale = 1.0f;
    if (g_player.isCharging) {
        float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;
        chargeRatio = std::min(chargeRatio * 8, 1.0f);
        timeScale = 1.0f - chargeRatio * 0.8f;
    }
    float scaledDeltaTime = deltaTime * timeScale;

    // 使用调整后的时间更新游戏逻辑
    UpdateDash(deltaTime);
    g_camera.Update(scaledDeltaTime);
    UpdatePlayerPhysics(scaledDeltaTime);
    UpdateEnemies(scaledDeltaTime, &g_mapManager);

    // 动画状态更新
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
}

// 辅助函数：根据瓦片代码获取纹理
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
        return g_backgroundTexture;
    }
    else {
        return g_groundTexture;
    }
}

// 辅助函数：根据瓦片代码设置颜色
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

    // 绘制背景（使用视差效果）
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    float bgOffsetX = cameraX * 0.3f;
    float bgOffsetY = cameraY * 0.3f;
    RenderImage(-1.0f + bgOffsetX, -1.0f + bgOffsetY, 2.0f, 2.0f, g_backgroundTexture, 0, 1, 1);

    // 使用新的地图系统绘制瓦片
    if (g_mapManager.IsMapLoaded()) {
        Map* currentMap = g_mapManager.GetCurrentMap();

        // 绘制背景层瓦片
        auto& bgTiles = currentMap->GetTiles(MapLayer::BACKGROUND);
        for (const auto& tile : bgTiles) {
            if (tile.tileInfo.code == "00") continue;
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
        }

        // 绘制中间层瓦片（玩家活动层）
        auto& mgTiles = currentMap->GetTiles(MapLayer::MIDGROUND);
        for (const auto& tile : mgTiles) {
            if (tile.tileInfo.code == "00") continue;
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
        }

        // 绘制前景层瓦片
        auto& fgTiles = currentMap->GetTiles(MapLayer::FOREGROUND);
        for (const auto& tile : fgTiles) {
            if (tile.tileInfo.code == "00") continue;
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
        }
    }

    // 绘制充能效果
    if (g_player.isCharging) {
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

    // 绘制冲刺效果
    if (g_player.isDashing) {
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

    // 绘制玩家
    std::pair<float, float> playerPos = worldToScreen(g_player.posX, g_player.posY);
    ID3D11ShaderResourceView* playerTexture = g_playerTexture;

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

    RenderEnemies(g_camera);
    frameIndex = g_player.anim.GetCurrentFrame();
    g_mouseIndicator.Render(g_camera.GetX(), g_camera.GetY());

    RenderImage(playerPos.first, playerPos.second, PLAYER_WIDTH, PLAYER_HEIGHT,
        g_playerTexture, frameIndex, 1, 10);

    float uiScale = std::min(currentWidth / 1920.0f, currentHeight / 1080.0f);
}

void HandleInput() {
    if (g_inputSystem.IsResetting()) {
        ResetGame();
    }

    if (g_inputSystem.IsTogglePressed(VK_P)) {
        if (sceneManager.GetCurrentSceneType() == STAGE) {
            sceneManager.SwitchScene(PAUSE);
        }
        else if (sceneManager.GetCurrentSceneType() == PAUSE) {
            sceneManager.SwitchScene(STAGE);
        }
    }

    // 获取鼠标输入状态
    bool isMouseLeftPressed = g_inputSystem.IsMouseLeftPressed();
    bool isMouseLeftDown = g_inputSystem.IsMouseLeftDown();
    bool isMouseLeftReleased = g_inputSystem.IsMouseLeftReleased();

    static bool wasMouseLeftDown = false;

    // 纯鼠标控制：按下开始蓄力
    if (isMouseLeftPressed) {
        StartMouseChargeDash();
    }

    // 纯鼠标控制：释放执行冲刺
    if (isMouseLeftReleased && wasMouseLeftDown && g_player.isCharging) {
        ExecuteMouseChargeDash();
    }

    // 取消蓄力
    if (!isMouseLeftDown && g_player.isCharging) {
        CancelChargeDash();
    }

    wasMouseLeftDown = isMouseLeftDown;

    // 移动控制
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

    // 跳跃控制
    static bool wasJumpKeyPressed = false;
    bool isJumpKeyPressed = g_inputSystem.IsJumping();
    if (isJumpKeyPressed && !wasJumpKeyPressed) {
        Jump();
    }
    wasJumpKeyPressed = isJumpKeyPressed;
}

// MouseIndicatorSystem 实现
void MouseIndicatorSystem::Initialize() {
    m_mouseIndicatorTexture = g_chargeEffectTexture;
    m_arrowTexture = g_dashEffectTexture;
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

    auto worldToScreen = [cameraX, cameraY](float worldX, float worldY) -> std::pair<float, float> {
        return { worldX - cameraX, worldY - cameraY };
        };

    // 绘制鼠标位置指示器（原有代码）
    float indicatorSize = 0.1f;
    auto mousePos = worldToScreen(m_mouseWorldX - indicatorSize / 2, m_mouseWorldY - indicatorSize / 2);

    SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    RenderImage(mousePos.first, mousePos.second, indicatorSize, indicatorSize,
        m_mouseIndicatorTexture, 0, 1, 1);

    // 在屏幕右上角固定显示冲刺点数
    float dashPointsX = 0.9f; // 屏幕右侧
    float dashPointsY = 0.1f; // 屏幕顶部
    float digitWidth = 0.08f;
    float digitHeight = 0.12f;

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // 渲染冲刺点数
    RenderNumber(g_player.dashPoints, dashPointsX, dashPointsY, digitWidth, digitHeight, pTextureNum);


    // 绘制方向箭头
    float arrowDistance = 0.08f;
    float arrowSize = 0.15f;

    float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;

    float arrowX = playerCenterX + cosf(m_arrowAngle) * arrowDistance;
    float arrowY = playerCenterY + sinf(m_arrowAngle) * arrowDistance;

    auto arrowScreenPos = worldToScreen(arrowX - arrowSize / 2, arrowY - arrowSize / 2);

    SetColor(0.0f, 1.0f, 0.0f, 1.0f);
    // 更新颜色设置代码
    if (g_player.chargeTime >= g_player.CHARGE_THRESHOLD_LOW && g_player.chargeTime < g_player.CHARGE_THRESHOLD_MID) {
        SetColor(0.0f, 0.0f, 1.0f, 1.0f);
    }
    else if (g_player.chargeTime >= g_player.CHARGE_THRESHOLD_MID && g_player.chargeTime < g_player.CHARGE_THRESHOLD_HIGH) {
        SetColor(0.0f, 1.0f, 1.0f, 1.0f);
    }
    else if (g_player.chargeTime >= g_player.CHARGE_THRESHOLD_HIGH) {
        SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    }

    RenderImage(arrowScreenPos.first, arrowScreenPos.second, arrowSize, arrowSize,
        m_arrowTexture, 0, 1, 1);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void MouseIndicatorSystem::Cleanup() {
    m_mouseIndicatorTexture = nullptr;
    m_arrowTexture = nullptr;
}