#define NOMINMAX
#include "Game.h"

#pragma once

// Dash type enumeration
enum DashType {
    DASH_INSTANT,    // Dash immediately on key press
    DASH_CHARGE      // Charge dash on hold
};

DashType g_currentDashType = DASH_INSTANT; // Currently used dash type

GameTimer g_gameTimer;
Player g_player;
// Game state enumeration
enum GameState {
    STATE_PLAYING,
    STATE_GAME_OVER
};

GameState g_gameState = STATE_PLAYING;
std::vector<MapBlock> g_mapBlocks;
ID3D11ShaderResourceView* g_playerTexture = nullptr;
ID3D11ShaderResourceView* g_groundTexture = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture = nullptr;
ID3D11ShaderResourceView* g_dashEffectTexture = nullptr;
ID3D11ShaderResourceView* g_chargeEffectTexture = nullptr; // Charge effect texture

// Game timer implementation
GameTimer::GameTimer() {
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
}

float GameTimer::GetDeltaTime() const {
    return m_deltaTime;
}

void ResetGame() {
    // Reset player state - place player on ground
    g_player.posX = 0.0f;
    g_player.posY = -0.7f;  // Place above ground
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;
    g_player.isOnGround = false;
    g_player.isMoving = false;
    g_player.facingRight = true;

    // Reset dash state
    g_player.isDashing = false;
    g_player.dashTimer = 0.0f;
    g_player.dashCooldown = 0.0f;
    g_player.dashDirectionX = 0.0f;
    g_player.dashDirectionY = 0.0f;

    // Reset charge state
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;

    g_gameState = STATE_PLAYING;
}

// Improved collision detection function
bool CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
        y1 < y2 + h2 && y1 + h1 > y2);
}

// Create simple map
void CreateTestMap() {
    g_mapBlocks.clear();

    // Create continuous ground
    for (int i = 0; i < 20; i++) {
        MapBlock ground;
        ground.posX = -1.0f + i * GRID_WIDTH * 8; // Ensure no gaps
        ground.posY = -0.9f;
        ground.width = GRID_WIDTH * 8;
        ground.height = GRID_HEIGHT;
        ground.isSolid = true;
        g_mapBlocks.push_back(ground);
    }

    // Adjust platform positions to give player more jumping space
    MapBlock platform1;
    platform1.posX = -0.5f;
    platform1.posY = -0.6f;  // Raise platform position
    platform1.width = 0.3f;
    platform1.height = 0.05f;
    platform1.isSolid = true;
    g_mapBlocks.push_back(platform1);

    MapBlock platform2;
    platform2.posX = 0.2f;
    platform2.posY = -0.4f;  // Raise platform position
    platform2.width = 0.3f;
    platform2.height = 0.05f;
    platform2.isSolid = true;
    g_mapBlocks.push_back(platform2);

    MapBlock platform3;
    platform3.posX = -0.2f;
    platform3.posY = -0.2f;  // Raise platform position
    platform3.width = 0.3f;
    platform3.height = 0.05f;
    platform3.isSolid = true;
    g_mapBlocks.push_back(platform3);

    // Add walls and obstacles for collision testing
    MapBlock leftWall;
    leftWall.posX = -1.0f;
    leftWall.posY = -0.9f;
    leftWall.width = 0.05f;
    leftWall.height = 1.8f;  // From ground to top
    leftWall.isSolid = true;
    g_mapBlocks.push_back(leftWall);

    MapBlock rightWall;
    rightWall.posX = 0.95f;  // Right side of screen
    rightWall.posY = -0.9f;
    rightWall.width = 0.05f;
    rightWall.height = 1.8f;
    rightWall.isSolid = true;
    g_mapBlocks.push_back(rightWall);
}

// Game initialization
void InitGameWorld() {
    // Load textures (temporarily using existing textures)
    LoadTexture(g_pDevice, "asset/block.png", &g_playerTexture);      // Player texture
    LoadTexture(g_pDevice, "asset/blockB.png", &g_groundTexture);     // Ground texture
    LoadTexture(g_pDevice, "asset/Space.png", &g_backgroundTexture);  // Background texture
    LoadTexture(g_pDevice, "asset/block.png", &g_dashEffectTexture);   // Dash effect texture
    LoadTexture(g_pDevice, "asset/blockB.png", &g_chargeEffectTexture); // Charge effect texture
    InitEnemies();
    CreateTestMap();
    ResetGame();
}

void UpdatePlayerPhysics(float deltaTime) {
    // Apply gravity (if not dashing)
    if (!g_player.isDashing) {
        float fixedDeltaTime = std::min(deltaTime, 0.033f);
        g_player.velocityY += GRAVITY * fixedDeltaTime * 60.0f;

        // Limit maximum fall speed
        if (g_player.velocityY < -0.3f) {
            g_player.velocityY = -0.3f;
        }
    }

    // Save current position for collision detection
    float originalX = g_player.posX;
    float originalY = g_player.posY;

    // Apply horizontal movement first
    g_player.posX += g_player.velocityX * deltaTime * 60.0f;

    // Horizontal collision detection
    for (const auto& block : g_mapBlocks) {
        if (!block.isSolid) continue;

        if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            block.posX, block.posY, block.width, block.height)) {
            g_player.velocityX = 0.0f;
        }
    }

    // Apply vertical movement
    g_player.posY += g_player.velocityY * deltaTime * 60.0f;

    g_player.isOnGround = false;

    for (const auto& block : g_mapBlocks) {
        if (!block.isSolid) continue;

        if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            block.posX, block.posY, block.width, block.height)) {

            // Calculate player and block center points
            float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
            float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;
            float blockCenterX = block.posX + block.width / 2;
            float blockCenterY = block.posY + block.height / 2;

            // Calculate overlap amount
            float overlapX = (PLAYER_WIDTH / 2 + block.width / 2) - fabs(playerCenterX - blockCenterX);
            float overlapY = (PLAYER_HEIGHT / 2 + block.height / 2) - fabs(playerCenterY - blockCenterY);

            // Resolve collision based on minimum overlap direction
            if (overlapX < overlapY) {
                // X-axis collision
                if (playerCenterX < blockCenterX) {
                    g_player.posX = block.posX - PLAYER_WIDTH;
                }
                else {
                    g_player.posX = block.posX + block.width;
                }
                g_player.velocityX = 0.0f;
            }
            else {
                // Y-axis collision
                if (playerCenterY < blockCenterY) {
                    g_player.posY = block.posY - PLAYER_HEIGHT;
                    g_player.velocityY = 0.0f;
                }
                else {
                    // Collision from above (landing)
                    g_player.posY = block.posY + block.height;
                    g_player.velocityY = 0.0f;
                    g_player.isOnGround = true;
                }
            }
        }
    }

    // Boundary check - fall detection
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

// Update dash state
void UpdateDash(float deltaTime) {
    // Update charge state
    if (g_player.isCharging) {
        g_player.chargeTime += deltaTime;
        if (g_player.chargeTime >= g_player.MAX_CHARGE_TIME) {
            // Auto trigger dash at max charge time
            ExecuteChargeDash();
        }
    }

    if (g_player.dashCooldown > 0.0f) {
        g_player.dashCooldown -= deltaTime;
    }

    if (g_player.isDashing) {
        g_player.dashTimer -= deltaTime;

        if (g_player.dashTimer <= 0.0f) {
            // Dash ended
            g_player.isDashing = false;

            // If not on ground, maintain Y velocity, otherwise stop Y movement
            if (!g_player.isOnGround) {
                g_player.velocityY *= 0.5f; // Keep some velocity
            }
            else {
                g_player.velocityY = 0.0f;
            }

            // Stop horizontal movement unless there's input
            if (!g_player.isMoving) {
                g_player.velocityX *= 0.3f; // Keep some inertia
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

    // Update dash state
    UpdateDash(deltaTime);

    // Update physics
    UpdatePlayerPhysics(deltaTime);
}

// Modified render function
void Draw() {
    RendererDrawF();

    // Draw background
    RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, g_backgroundTexture, 0, 1, 1);

    // Draw map blocks
    for (const auto& block : g_mapBlocks) {
        ID3D11ShaderResourceView* texture = g_groundTexture;
        RenderImage(block.posX, block.posY, block.width, block.height, texture, 0, 1, 1);
    }

    // Draw charge effect (if charging)
    if (g_player.isCharging) {
        float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;
        float effectSize = PLAYER_WIDTH * (1.0f + chargeRatio * 1.0f);
        float alpha = 0.3f + chargeRatio * 0.7f;

        RenderImage(g_player.posX - (effectSize - PLAYER_WIDTH) * 0.5f,
            g_player.posY - (effectSize - PLAYER_HEIGHT) * 0.5f,
            effectSize, effectSize, g_chargeEffectTexture, 0, alpha, 1);
    }

    // Draw dash effect (if dashing)
    if (g_player.isDashing) {
        // Draw dash effect at player position
        RenderImage(g_player.posX, g_player.posY, PLAYER_WIDTH * 1.5f, PLAYER_HEIGHT * 1.5f,
            g_dashEffectTexture, 0, 1, 1);
    }


    ID3D11ShaderResourceView* playerTexture = g_playerTexture;

    // Select different frame or color based on player state
    int frameIndex = 0;
    if (g_player.isCharging) {
        frameIndex = 4; // Charging state
    }
    else if (g_player.isDashing) {
        frameIndex = 3; // Dashing state
    }
    else if (!g_player.isOnGround) {
        frameIndex = 2; // Airborne state
    }
    else if (g_player.isMoving) {
        frameIndex = 1; // Moving state
    }

    RenderImage(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
        playerTexture, frameIndex, 1, 5); // 5 frames: idle, moving, jumping, dashing, charging

    // Draw UI information
    // Can add text to show current dash type here

    RendererDrawB();
}

// Keyboard detection
bool IsKeyDown(int key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}
void HandleInput() {
    // Update input system
    g_inputSystem.Update();

    if (g_inputSystem.IsResetting()) {
        ResetGame();
    }

    // Toggle dash type
    static bool wasToggleKeyPressed = false;
    bool isToggleKeyPressed = g_inputSystem.IsToggling();
    if (isToggleKeyPressed && !wasToggleKeyPressed) {
        ToggleDashType();
    }
    wasToggleKeyPressed = isToggleKeyPressed;

    // Horizontal movement
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

    // Jump input
    static bool wasJumpKeyPressed = false;
    bool isJumpKeyPressed = g_inputSystem.IsJumping();

    if (isJumpKeyPressed && !wasJumpKeyPressed) {
        Jump();
    }
    wasJumpKeyPressed = isJumpKeyPressed;

    // Dash input handling
    static bool wasDashKeyPressed = false;
    bool isDashKeyPressed = g_inputSystem.IsDashing();

    if (g_currentDashType == DASH_INSTANT) {
        // Method 1: Dash immediately on press
        if (isDashKeyPressed && !wasDashKeyPressed) {
            Dash1();
        }
    }
    else {
        // Method 2: Charge dash on hold
        if (isDashKeyPressed && !wasDashKeyPressed) {
            // Press to start charging
            StartChargeDash();
        }
        else if (!isDashKeyPressed && wasDashKeyPressed) {
            // Release to trigger dash
            ExecuteChargeDash();
        }
        else if (!isDashKeyPressed && g_player.isCharging) {
            // Prevent abnormal situations
            CancelChargeDash();
        }
    }

    wasDashKeyPressed = isDashKeyPressed;
}

// Modified game loop
void GameLoop() {
    g_gameTimer.Tick();
    float delta = g_gameTimer.GetDeltaTime();

    HandleInput();  // Handle keyboard input
    // Update game state
    UpdateGame(delta);
    Draw();
}