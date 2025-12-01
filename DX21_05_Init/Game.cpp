#define NOMINMAX
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
    // 计算总时间（从计时器创建开始）
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
     // added november 19th
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_playerTexture);      // Player texture testing for animation
    g_player.anim.Init(10, 1, 0.15f, 0); // (10 columns, 1 row)
    g_player.anim.AddClip("Idle", 0, 9, 0.25f, true); // frames 0–9 loop


    LoadTexture(g_pDevice, "asset/blockB.png", &g_groundTexture);     // Ground texture
    LoadTexture(g_pDevice, "asset/Space.png", &g_backgroundTexture);  // Background texture
    LoadTexture(g_pDevice, "asset/block.png", &g_dashEffectTexture);   // Dash effect texture
    LoadTexture(g_pDevice, "asset/completed.png", &g_chargeEffectTexture); // Charge effect texture
    InitEnemies();
    g_mapManager.InitializeMaps();

    g_mouseIndicator.Initialize();

    g_camera.SetSmoothness(camera_Smoothness);
    g_camera.SetLookAhead(camera_LookAhead);
    g_camera.SetDeadZone(camera_DeadZone);

    ResetGame();
}

void UpdatePlayerPhysics(float deltaTime) {
    // 应用重力...
    if (!g_player.isDashing) {
        float fixedDeltaTime = std::min(deltaTime, 0.033f);
        g_player.velocityY += GRAVITY * fixedDeltaTime * 60.0f;
        if (g_player.velocityY < -0.3f) {
            g_player.velocityY = -0.3f;
        }
    }

    float originalX = g_player.posX;
    float originalY = g_player.posY;

    // 水平移动
    g_player.posX += g_player.velocityX * deltaTime * 60.0f;

    // 使用新的地图系统进行碰撞检测
    auto& solidTiles = g_mapManager.GetCurrentMap()->GetSolidTiles();
    for (const auto& tile : solidTiles) {
        if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            tile.posX, tile.posY, tile.width, tile.height)) {
            g_player.velocityX = 0.0f;
        }
    }

    // 垂直移动
    g_player.posY += g_player.velocityY * deltaTime * 60.0f;
    g_player.isOnGround = false;

    for (const auto& tile : solidTiles) {
        if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            tile.posX, tile.posY, tile.width, tile.height)) {

            float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
            float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;
            float tileCenterX = tile.posX + tile.width / 2;
            float tileCenterY = tile.posY + tile.height / 2;

            float overlapX = (PLAYER_WIDTH / 2 + tile.width / 2) - fabs(playerCenterX - tileCenterX);
            float overlapY = (PLAYER_HEIGHT / 2 + tile.height / 2) - fabs(playerCenterY - tileCenterY);

            if (overlapX < overlapY) {
                if (playerCenterX < tileCenterX) {
                    g_player.posX = tile.posX - PLAYER_WIDTH;
                }
                else {
                    g_player.posX = tile.posX + tile.width;
                }
                g_player.velocityX = 0.0f;
            }
            else {
                if (playerCenterY < tileCenterY) {
                    g_player.posY = tile.posY - PLAYER_HEIGHT;
                    g_player.velocityY = 0.0f;
                }
                else {
                    g_player.posY = tile.posY + tile.height;
                    g_player.velocityY = 0.0f;
                    g_player.isOnGround = true;
                }
            }
        }
    }

    // 传送门检测 - 修正参数
    static float portalCooldown = 0.0f;
    if (portalCooldown > 0.0f) {
        portalCooldown -= deltaTime;
    }

    // 修改传送门检测部分
    if (portalCooldown <= 0.0f) {
        std::string targetMap;
        int portalId, linkedSpawnId;
        if (g_mapManager.GetCurrentMap()->CheckPortalCollision(
            g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            targetMap, portalId, linkedSpawnId)) {

            g_mapManager.SwitchMap(targetMap, portalId, linkedSpawnId);
            portalCooldown = 1.0f;
            // 敌人会在SwitchMap中自动重新生成
        }
    }

    // 边界检查
    if (g_player.posY < -2.0f) {
        ResetGame();
    }
}

// Method 1: Dash immediately on press
void Dash1() {
    if (g_player.dashCooldown > 0.0f || g_player.isDashing) {
        return;
    }

    // Get direction from input system
    float dirX = 0.0f, dirY = 0.0f;
    g_inputSystem.GetMoveDirection(dirX, dirY);

    // If no direction input, use player facing direction
    if (dirX == 0.0f && dirY == 0.0f) {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
    }

    // Set dash state
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION;
    g_player.dashCooldown = DASH_COOLDOWN;
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // Set dash speed
    g_player.velocityX = dirX * DASH_SPEED;
    g_player.velocityY = dirY * DASH_SPEED;
}

// Method 2: Charge dash on hold
void StartChargeDash() {
    if (g_player.dashCooldown > 0.0f || g_player.isDashing || g_player.isCharging) {
        return;
    }

    g_player.isCharging = true;
    g_player.chargeTime = 0.0f;
}

void ExecuteChargeDash() {
    if (!g_player.isCharging || g_player.chargeTime < g_player.MIN_CHARGE_TIME) {
        return;
    }

    // Get direction from input system
    float dirX = 0.0f, dirY = 0.0f;
    g_inputSystem.GetMoveDirection(dirX, dirY);

    // If no direction input, use player facing direction
    if (dirX == 0.0f && dirY == 0.0f) {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
    }

    // Normalize direction vector
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }

    // Calculate dash parameters based on charge time
    float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;
    chargeRatio = std::min(chargeRatio, 1.0f); // Cap at 1.0

    // Speed and duration increase with charge time (non-linear, fast early, slow later)
    float speedMultiplier = 1.0f + chargeRatio * 2.0f; // 1.0x to 3.0x
    float durationMultiplier = 1.0f + chargeRatio * 1.5f; // 1.0x to 2.5x

    // Set dash state
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION * durationMultiplier;
    g_player.dashCooldown = DASH_COOLDOWN * (0.5f + chargeRatio * 0.5f); // Cooldown also increases with charge
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // Set dash speed
    g_player.velocityX = dirX * DASH_SPEED * speedMultiplier;
    g_player.velocityY = dirY * DASH_SPEED * speedMultiplier;

    // End charge state
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
}

void CancelChargeDash() {
    if (g_player.isCharging) {
        g_player.isCharging = false;
        g_player.chargeTime = 0.0f;
    }
}
void UpdateDash(float deltaTime) {
    // 更新蓄力状态
    if (g_player.isCharging) {
        g_player.chargeTime += deltaTime;
        if (g_player.chargeTime >= g_player.MAX_CHARGE_TIME) {
            ExecuteMouseChargeDash(); // 达到最大蓄力自动触发
        }
    }

    // 更新冷却时间
    if (g_player.dashCooldown > 0.0f) {
        g_player.dashCooldown -= deltaTime;
    }

    // 更新冲刺状态
    if (g_player.isDashing) {
        g_player.dashTimer -= deltaTime;

        if (g_player.dashTimer <= 0.0f) {
            // 冲刺结束
            g_player.isDashing = false;
            g_player.hasMouseTarget = false; // 清除鼠标目标显示

            // 垂直速度处理
            if (!g_player.isOnGround) {
                g_player.velocityY *= 0.5f; // 空中保持部分Y速度
            }
            else {
                g_player.velocityY = 0.0f;
            }

            // 水平惯性调整：保留10%速度，让过渡更自然
            g_player.velocityX *= 0.1f;
            g_player.velocityY *= 0.1f;

            // 如果玩家有输入，覆盖惯性
            if (g_player.isMoving) {
                if (g_player.facingRight) {
                    g_player.velocityX = MOVE_SPEED;
                }
                else {
                    g_player.velocityX = -MOVE_SPEED;
                }
            }
        }
    }
}

// Toggle dash type (for testing)
void ToggleDashType() {
    if (g_currentDashType == DASH_INSTANT) {
        g_currentDashType = DASH_CHARGE;
    }
    else {
        g_currentDashType = DASH_INSTANT;
    }

    // Cancel current charge state
    if (g_player.isCharging) {
        CancelChargeDash();
    }
}

// Player movement control
void MovePlayerLeft() {
    g_player.velocityX = -MOVE_SPEED;
    g_player.isMoving = true;
    g_player.facingRight = false;
}

void MovePlayerRight() {
    g_player.velocityX = MOVE_SPEED;
    g_player.isMoving = true;
    g_player.facingRight = true;
}

void StopPlayer() {
    if (!g_player.isDashing) {
        g_player.velocityX = 0.0f;
    }
    g_player.isMoving = false;
}

// Improved jump function
void Jump() {
    if (g_player.isOnGround && !g_player.isDashing && !g_player.isCharging) {
        g_player.velocityY = JUMP_FORCE;
        g_player.isOnGround = false;
    }
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
        timeScale = 1.0f - chargeRatio * 0.8f; // 最大减缓到0.2倍速度
    }
    float scaledDeltaTime = deltaTime * timeScale;

    // 使用调整后的时间更新游戏逻辑
    UpdateDash(scaledDeltaTime);
    g_camera.Update(scaledDeltaTime);
    UpdatePlayerPhysics(scaledDeltaTime);
    UpdateEnemies(scaledDeltaTime, &g_mapManager);

    // added november 19th
    // Check player state and set animation clip only if it changed
    if (g_player.isCharging) 
    {
        if (g_player.anim.GetCurrentClipName() != "Charge")   // charging state, for charge clip
        {
            g_player.anim.SetClip("Charge");
        }
    }
    else if (g_player.isDashing) 
    {
        // dashing state, for dash clip
        if (g_player.anim.GetCurrentClipName() != "Dash")
        {
            g_player.anim.SetClip("Dash");
        }   
    }
    else if (!g_player.isOnGround) 
    {
        // jumping state, for jump clip
        if (g_player.anim.GetCurrentClipName() != "Jump")
        {
            g_player.anim.SetClip("Jump");
        } 
    }
    else if (g_player.isMoving) 
    {
        // running state, for right
        if (g_player.facingRight) 
        {
            if (g_player.anim.GetCurrentClipName() != "RunRight")
            {
                g_player.anim.SetClip("RunRight");
            }  
        }
        else  // for left
        {
            if (g_player.anim.GetCurrentClipName() != "RunLeft")
            {
                g_player.anim.SetClip("RunLeft");
            }  
        }
    }
    else 
    {
        if (g_player.anim.GetCurrentClipName() != "Idle") // idle state,  loop through idle frames
        {
            g_player.anim.SetClip("Idle");
        }
    }

    g_mouseIndicator.Update(scaledDeltaTime);
    // Advance the current clips frame and updates it
    g_player.anim.Update(scaledDeltaTime);
}

// 辅助函数：根据瓦片代码获取纹理
ID3D11ShaderResourceView* GetTextureForTile(const std::string& tileCode) {
    if (tileCode == "G1" || tileCode == "G2" || tileCode == "G3") {
        return g_groundTexture;  // 地面使用地面纹理
    }
    else if (tileCode == "W1" || tileCode == "W2") {
        return g_groundTexture;  // 墙壁也使用地面纹理，但颜色不同
    }
    else if (tileCode == "P1" || tileCode == "P2") {
        return g_groundTexture;  // 平台使用地面纹理
    }
    else if (tileCode == "DF" || tileCode == "DI" || tileCode == "DT") {
        return g_fastEnemyTexture;  // 传送门使用冲刺效果纹理
    }
    else if (tileCode == "D1" || tileCode == "D2") {
        return g_backgroundTexture;  // 装饰物使用背景纹理
    }
    else {
        return g_groundTexture;  // 默认使用地面纹理
    }
}

// 辅助函数：根据瓦片代码设置颜色
void SetTileColor(const std::string& tileCode) {
    if (tileCode == "G1") { // 草土地面
        SetColor(0.4f, 0.8f, 0.3f, 1.0f);
    }
    else if (tileCode == "G2") { // 泥土地面
        SetColor(0.6f, 0.4f, 0.2f, 1.0f);
    }
    else if (tileCode == "G3") { // 石质地面的
        SetColor(0.5f, 0.5f, 0.5f, 1.0f);
    }
    else if (tileCode == "W1") { // 砖墙
        SetColor(0.7f, 0.3f, 0.2f, 1.0f);
    }
    else if (tileCode == "W2") { // 石墙
        SetColor(0.4f, 0.4f, 0.4f, 1.0f);
    }
    else if (tileCode == "P1") { // 木质平台
        SetColor(0.8f, 0.6f, 0.3f, 1.0f);
    }
    else if (tileCode == "P2") { // 金属平台
        SetColor(0.7f, 0.7f, 0.8f, 1.0f);
    }
    else if (tileCode == "PF") { // 森林传送门
        SetColor(0.3f, 0.7f, 0.3f, 1.0f);
    }
    else if (tileCode == "PI") { // 冰传送门
        SetColor(0.3f, 0.5f, 0.9f, 1.0f);
    }
    else if (tileCode == "PT") { // 测试传送门
        SetColor(0.8f, 0.3f, 0.8f, 1.0f);
    }
    else if (tileCode == "D1") { // 树装饰
        SetColor(0.2f, 0.5f, 0.1f, 1.0f);
    }
    else if (tileCode == "D2") { // 石头装饰
        SetColor(0.5f, 0.5f, 0.5f, 1.0f);
    }
    else {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f); // 默认白色
    }
}

void DrawGame() {
    //RendererDrawF();

    // 获取相机位置
    int currentWidth = g_camera.GetWidth(); // 或者使用 g_windowWidth
    int currentHeight = g_camera.GetHeight(); // 或者使用 g_windowHeight

    float aspectRatio = static_cast<float>(currentWidth) / static_cast<float>(currentHeight);

    float cameraX = g_camera.GetX();
    float cameraY = g_camera.GetY();

    // 辅助函数：将世界坐标转换为屏幕坐标
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

            // 修复：使用传统方式获取坐标
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            float screenX = screenPos.first;
            float screenY = screenPos.second;

            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            float alpha = 0.7f;

            RenderImage(screenX, screenY, tile.width, tile.height, texture, 0, alpha, 1);
        }

        // 绘制中间层瓦片（玩家活动层）
        auto& mgTiles = currentMap->GetTiles(MapLayer::MIDGROUND);
        for (const auto& tile : mgTiles) {
            if (tile.tileInfo.code == "00") continue;

            // 修复：使用传统方式获取坐标
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            float screenX = screenPos.first;
            float screenY = screenPos.second;

            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);

            float alpha = 1.0f;
            if (tile.tileInfo.isPortal) {
                alpha = 0.8f + 0.2f * sin(g_gameTimer.GetTotalTime() * 3.0f);
            }

            RenderImage(screenX, screenY, tile.width, tile.height, texture, 0, alpha, 1);
        }

        // 绘制前景层瓦片
        auto& fgTiles = currentMap->GetTiles(MapLayer::FOREGROUND);
        for (const auto& tile : fgTiles) {
            if (tile.tileInfo.code == "00") continue;

            // 修复：使用传统方式获取坐标
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            float screenX = screenPos.first;
            float screenY = screenPos.second;

            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            float alpha = 0.8f;

            RenderImage(screenX, screenY, tile.width, tile.height, texture, 0, alpha, 1);
        }
    }

    // 绘制充能效果
    if (g_player.isCharging) {
        float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;
        float effectSize = PLAYER_WIDTH * (1.0f + chargeRatio * 1.0f);
        float alpha = 0.3f + chargeRatio * 0.7f;

        // 修复：使用传统方式获取坐标
        std::pair<float, float> effectPos = worldToScreen(
            g_player.posX - (effectSize - PLAYER_WIDTH) * 0.5f,
            g_player.posY - (effectSize - PLAYER_HEIGHT) * 0.5f
        );
        float effectX = effectPos.first;
        float effectY = effectPos.second;

        float r = 1.0f;
        float g = 1.0f - chargeRatio * 0.5f;
        float b = 0.0f;
        SetColor(r, g, b, alpha);

        RenderImage(effectX, effectY, effectSize, effectSize, g_chargeEffectTexture, 0, alpha, 1);
    }

    // 绘制冲刺效果
    if (g_player.isDashing) {
        float dashProgress = 1.0f - (g_player.dashTimer / DASH_DURATION);
        float effectSize = PLAYER_WIDTH * (1.2f + dashProgress * 0.3f);
        float alpha = 0.7f + dashProgress * 0.3f;

        // 修复：使用传统方式获取坐标
        std::pair<float, float> dashPos = worldToScreen(
            g_player.posX - (effectSize - PLAYER_WIDTH) * 0.5f,
            g_player.posY - (effectSize - PLAYER_HEIGHT) * 0.5f
        );
        float dashX = dashPos.first;
        float dashY = dashPos.second;

        SetColor(1.0f, 0.3f, 0.3f, alpha);
        RenderImage(dashX, dashY, effectSize, effectSize, g_dashEffectTexture, 0, alpha, 1);
    }

    // 绘制玩家
    // 修复：使用传统方式获取玩家坐标
    std::pair<float, float> playerPos = worldToScreen(g_player.posX, g_player.posY);
    float playerScreenX = playerPos.first;
    float playerScreenY = playerPos.second;

    ID3D11ShaderResourceView* playerTexture = g_playerTexture;

    // 根据玩家状态选择不同的帧和颜色
    int frameIndex = 0;
    if (g_player.isCharging) {
        SetColor(1.0f, 1.0f, 0.0f, 1.0f);
        frameIndex = 4;
    }
    //int frameIndex = 0;
    if (g_player.isCharging)
    {
        SetColor(0.0f, 1.0f, 0.0f, 1.0f); // bright green
        //frameIndex = 4; // 充能状态帧
    }
    else if (g_player.isDashing) {
        SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        frameIndex = 3;
    }
    else if (g_player.isDashing)
    {
        SetColor(1.0f, 0.0f, 0.0f, 1.0f); // bright red 
        //frameIndex = 3; // 冲刺状态帧
    }
    else if (!g_player.isOnGround) {
        SetColor(1.0f, 0.5f, 0.0f, 1.0f);
        frameIndex = 2;
    }
    else if (!g_player.isOnGround)
    {
        SetColor(1.0f, 1.0f, 0.0f, 1.0f); // bright yellow
       // frameIndex = 2; // 空中状态帧
    }
    else if (g_player.isMoving) {
        frameIndex = 1;
        if (g_player.facingRight) {
            SetColor(0.0f, 0.0f, 1.0f, 1.0f); // 蓝色向右移动
        }
        else if (g_player.isMoving)
        {
            SetColor(1.0f, 0.0f, 1.0f, 1.0f); // bright magenta
        }   
    }
    else
    {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f); // bright white 
        //frameIndex = 0; // 待机状态帧
    }

    RenderEnemies(g_camera);
    // added november 19th
    frameIndex = g_player.anim.GetCurrentFrame();

    g_mouseIndicator.Render(g_camera.GetX(), g_camera.GetY());

    RenderImage(playerScreenX, playerScreenY, PLAYER_WIDTH, PLAYER_HEIGHT,
        g_playerTexture, frameIndex, 1, 10); // 10 total frames

    float uiScale = std::min(currentWidth / 1920.0f, currentHeight / 1080.0f);

    RendererDrawB();
}

void HandleInput() {
    //g_inputSystem.Update(); 

    if (g_inputSystem.IsResetting()) {
        ResetGame();
    }

    // for the pause
    /*if (g_inputSystem.IsTogglePressed(VK_P)) {
        if (g_gameState == STATE_PLAYING) {
            g_gameState = STATE_PAUSED;
        }
        else if (g_gameState == STATE_PAUSED) {
            g_gameState = STATE_PLAYING;
        }
    }*/
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

    // 取消蓄力（如果中途右键点击或其他取消条件）
    if (!isMouseLeftDown && g_player.isCharging) {
        // 可以添加取消条件，比如按下右键取消
        CancelChargeDash();
    }

    wasMouseLeftDown = isMouseLeftDown;

    // 移动控制保持不变
    bool moving = false;
    if (g_inputSystem.IsMovingLeft()) {
        if (!g_player.isDashing && !g_player.isCharging) {
            g_player.velocityX = -MOVE_SPEED;
        }
        g_player.isMoving = true;
        g_player.facingRight = false;
        moving = true;
    }
    if (g_inputSystem.IsMovingRight()) {
        if (!g_player.isDashing && !g_player.isCharging) {
            g_player.velocityX = MOVE_SPEED;
        }
        g_player.isMoving = true;
        g_player.facingRight = true;
        moving = true;
    }

    if (!moving && !g_player.isDashing && !g_player.isCharging) {
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
// 方法3: 鼠标方向冲刺
void DashToMouse() {
    if (g_player.dashCooldown > 0.0f || g_player.isDashing) {
        return;
    }

    // 获取鼠标世界坐标
    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY);

    // 计算从玩家指向鼠标的方向向量
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dirX = mouseX - playerCenterX;
    float dirY = mouseY - playerCenterY;

    // 归一化方向向量
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }
    else {
        // 如果鼠标就在玩家位置上，使用玩家面向方向
        dirX = g_player.facingRight ? 1.0f : -1.0f;
        dirY = 0.0f;
    }

    // 设置冲刺状态
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION;
    g_player.dashCooldown = DASH_COOLDOWN;
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // 设置冲刺速度
    g_player.velocityX = dirX * DASH_SPEED;
    g_player.velocityY = dirY * DASH_SPEED;

    // 存储鼠标目标位置（用于可视化）
    g_player.mouseTargetX = mouseX;
    g_player.mouseTargetY = mouseY;
    g_player.hasMouseTarget = true;
}

// 方法4: 鼠标蓄力冲刺
void StartMouseChargeDash() {
    if (g_player.dashCooldown > 0.0f || g_player.isDashing || g_player.isCharging) {
        return;
    }

    g_player.isCharging = true;
    g_player.chargeTime = 0.0f;

    // 记录初始鼠标位置
    g_inputSystem.GetMousePosition(g_player.mouseTargetX, g_player.mouseTargetY);
    g_player.hasMouseTarget = true;
}

void ExecuteMouseChargeDash() {
    if (!g_player.isCharging || g_player.chargeTime < g_player.MIN_CHARGE_TIME) {
        return;
    }

    // 获取当前鼠标位置
    float currentMouseX, currentMouseY;
    g_inputSystem.GetMousePosition(currentMouseX, currentMouseY);

    // 计算从玩家指向鼠标的方向
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dirX = currentMouseX - playerCenterX;
    float dirY = currentMouseY - playerCenterY;

    // 归一化
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }
    else {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
        dirY = 0.0f;
    }

    // 根据蓄力时间计算冲刺参数
    float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;
    chargeRatio = std::min(chargeRatio, 1.0f);

    // 冲刺距离和速度增强
    float speedMultiplier = 1.0f + chargeRatio * 2.0f;
    float durationMultiplier = 1.0f + chargeRatio * 2.0f;

    // 设置冲刺状态
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION * durationMultiplier ;
    g_player.dashCooldown = DASH_COOLDOWN * (0.5f + chargeRatio * 0.5f);
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // 应用时间减缓效果到冲刺速度
    g_player.velocityX = dirX * DASH_SPEED * speedMultiplier ;
    g_player.velocityY = dirY * DASH_SPEED * speedMultiplier ;

    // 存储最终鼠标位置
    g_player.mouseTargetX = currentMouseX;
    g_player.mouseTargetY = currentMouseY;

    // 结束蓄力状态
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
}

void MouseIndicatorSystem::Initialize() {
    // 加载指示器纹理（可以使用现有纹理或创建新的）
    m_mouseIndicatorTexture = g_chargeEffectTexture; // 临时使用充能效果纹理
    m_arrowTexture = g_dashEffectTexture; // 临时使用冲刺效果纹理
    m_showMouseIndicator = true;
    m_arrowAngle = 0.0f;
}

void MouseIndicatorSystem::Update(float deltaTime) {
    // 采用与冲刺功能相同的方式直接获取鼠标世界坐标
    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY); // 此方式与ExecuteMouseChargeDash中一致

    // 直接使用获取到的鼠标世界坐标，避免额外的复杂转换
    m_mouseWorldX = mouseX;
    m_mouseWorldY = mouseY;

    // 计算从玩家指向鼠标的方向
    float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;

    float deltaX = m_mouseWorldX - playerCenterX;
    float deltaY = m_mouseWorldY - playerCenterY;

    // 使用atan2计算角度（弧度制），与常见数学库一致
    m_arrowAngle = atan2(deltaY, deltaX);

    // 调试输出（完成后可移除）
    static int debugCounter = 0;
    if (debugCounter++ % 60 == 0) {
        printf("Mouse World: (%.2f, %.2f), Player: (%.2f, %.2f)\n",
            m_mouseWorldX, m_mouseWorldY, playerCenterX, playerCenterY);
    }
}
// 在Render函数中，确保箭头正确指向鼠标位置
void MouseIndicatorSystem::Render(float cameraX, float cameraY) {
    if (!m_showMouseIndicator) return;

    auto worldToScreen = [cameraX, cameraY](float worldX, float worldY) -> std::pair<float, float> {
        return { worldX - cameraX, worldY - cameraY };
        };

    // 绘制鼠标位置指示器
    float indicatorSize = 0.1f;
    auto mousePos = worldToScreen(m_mouseWorldX - indicatorSize / 2, m_mouseWorldY - indicatorSize / 2);

    SetColor(1.0f, 0.0f, 0.0f, 1.0f); // 红色指示器
    RenderImage(mousePos.first, mousePos.second, indicatorSize, indicatorSize,
        m_mouseIndicatorTexture, 0, 1.0f, 1);

    // 绘制方向箭头（从玩家指向鼠标）
    float arrowDistance = 0.08f;
    float arrowSize = 0.15f;

    float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;

    // 箭头位置：从玩家位置向鼠标方向偏移固定距离
    float arrowX = playerCenterX + cosf(m_arrowAngle) * arrowDistance;
    float arrowY = playerCenterY + sinf(m_arrowAngle) * arrowDistance;

    auto arrowScreenPos = worldToScreen(arrowX - arrowSize / 2, arrowY - arrowSize / 2);

    SetColor(0.0f, 1.0f, 0.0f, 1.0f); // 绿色箭头
    RenderImage(arrowScreenPos.first, arrowScreenPos.second, arrowSize, arrowSize,
        m_arrowTexture, 0, 1.0f, 1);
}

void MouseIndicatorSystem::Cleanup() {
    // 清理资源
    m_mouseIndicatorTexture = nullptr;
    m_arrowTexture = nullptr;
}