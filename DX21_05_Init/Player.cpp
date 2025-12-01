#include "Game.h"

// 玩家物理更新
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

    // 传送门检测
    static float portalCooldown = 0.0f;
    if (portalCooldown > 0.0f) {
        portalCooldown -= deltaTime;
    }

    if (portalCooldown <= 0.0f) {
        std::string targetMap;
        int portalId, linkedSpawnId;
        if (g_mapManager.GetCurrentMap()->CheckPortalCollision(
            g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            targetMap, portalId, linkedSpawnId)) {

            g_mapManager.SwitchMap(targetMap, portalId, linkedSpawnId);
            portalCooldown = 1.0f;
        }
    }

    // 边界检查
    if (g_player.posY < -2.0f) {
        ResetGame();
    }
}

// Dash system implementation
void UpdateDash(float deltaTime) {
    // 更新蓄力状态
    if (g_player.isCharging) {
        g_player.chargeTime += deltaTime;
        if (g_player.chargeTime >= g_player.MAX_CHARGE_TIME) {
            ExecuteMouseChargeDash();
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
            g_player.hasMouseTarget = false;

            // 垂直速度处理
            if (!g_player.isOnGround) {
                g_player.velocityY *= 0.5f;
            }
            else {
                g_player.velocityY = 0.0f;
            }

            // 水平惯性调整
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
    chargeRatio = std::min(chargeRatio, 1.0f);

    float speedMultiplier = 1.0f + chargeRatio * 2.0f;
    float durationMultiplier = 1.0f + chargeRatio * 1.5f;

    // Set dash state
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION * durationMultiplier;
    g_player.dashCooldown = DASH_COOLDOWN * (0.5f + chargeRatio * 0.5f);
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

    // 存储鼠标目标位置
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

    float speedMultiplier = 1.0f + chargeRatio * 2.0f;
    float durationMultiplier = 1.0f + chargeRatio * 2.0f;

    // 设置冲刺状态
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION * durationMultiplier;
    g_player.dashCooldown = DASH_COOLDOWN * (0.5f + chargeRatio * 0.5f);
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // 应用冲刺速度
    g_player.velocityX = dirX * DASH_SPEED * speedMultiplier;
    g_player.velocityY = dirY * DASH_SPEED * speedMultiplier;

    // 存储最终鼠标位置
    g_player.mouseTargetX = currentMouseX;
    g_player.mouseTargetY = currentMouseY;

    // 结束蓄力状态
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
}