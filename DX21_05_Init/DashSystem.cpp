#include "DashSystem.h"
#include <algorithm>
#include <cmath>

void DashSystem::UpdateDash(Player& player, float deltaTime) {
    // 更新蓄力状态
    if (player.isCharging) {
        player.chargeTime += deltaTime;
        if (player.chargeTime >= player.MAX_CHARGE_TIME) {
            // 达到最大蓄力时间，自动触发冲刺
            ExecuteChargeDash(player);
        }
    }

    if (player.dashCooldown > 0.0f) {
        player.dashCooldown -= deltaTime;
    }

    if (player.isDashing) {
        player.dashTimer -= deltaTime;

        if (player.dashTimer <= 0.0f) {
            // 冲刺结束
            player.isDashing = false;

            // 如果不是在地面上，保持Y轴速度，否则停止Y轴移动
            if (!player.isOnGround) {
                player.velocityY *= 0.5f;
            }
            else {
                player.velocityY = 0.0f;
            }

            // 停止水平移动，除非有输入
            if (!player.isMoving) {
                player.velocityX *= 0.3f;
            }
        }
    }
}

void DashSystem::DashInstant(Player& player) {
    if (player.dashCooldown > 0.0f || player.isDashing) {
        return;
    }

    // 获取输入方向
    float dirX = 0.0f;
    float dirY = 0.0f;

    if (IsKeyDown(VK_LEFT) || IsKeyDown('A')) dirX -= 1.0f;
    if (IsKeyDown(VK_RIGHT) || IsKeyDown('D')) dirX += 1.0f;
    if (IsKeyDown(VK_UP) || IsKeyDown('W')) dirY += 1.0f;
    if (IsKeyDown(VK_DOWN) || IsKeyDown('S')) dirY -= 1.0f;

    // 如果没有方向输入，使用玩家朝向
    if (dirX == 0.0f && dirY == 0.0f) {
        dirX = player.facingRight ? 1.0f : -1.0f;
    }

    // 标准化方向向量
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }

    // 设置冲刺状态
    player.isDashing = true;
    player.dashTimer = DASH_DURATION;
    player.dashCooldown = DASH_COOLDOWN;
    player.dashDirectionX = dirX;
    player.dashDirectionY = dirY;

    // 设置冲刺速度
    player.velocityX = dirX * DASH_SPEED;
    player.velocityY = dirY * DASH_SPEED;
}

void DashSystem::StartChargeDash(Player& player) {
    if (player.dashCooldown > 0.0f || player.isDashing || player.isCharging) {
        return;
    }

    player.isCharging = true;
    player.chargeTime = 0.0f;
}

void DashSystem::ExecuteChargeDash(Player& player) {
    if (!player.isCharging || player.chargeTime < player.MIN_CHARGE_TIME) {
        return;
    }

    // 获取输入方向
    float dirX = 0.0f;
    float dirY = 0.0f;

    if (IsKeyDown(VK_LEFT) || IsKeyDown('A')) dirX -= 1.0f;
    if (IsKeyDown(VK_RIGHT) || IsKeyDown('D')) dirX += 1.0f;
    if (IsKeyDown(VK_UP) || IsKeyDown('W')) dirY += 1.0f;
    if (IsKeyDown(VK_DOWN) || IsKeyDown('S')) dirY -= 1.0f;

    // 如果没有方向输入，使用玩家朝向
    if (dirX == 0.0f && dirY == 0.0f) {
        dirX = player.facingRight ? 1.0f : -1.0f;
    }

    // 标准化方向向量
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }

    // 根据蓄力时间计算冲刺参数
    float chargeRatio = player.chargeTime / player.MAX_CHARGE_TIME;
    chargeRatio = std::min(chargeRatio, 1.0f);

    // 速度和持续时间随蓄力时间增加
    float speedMultiplier = 1.0f + chargeRatio * 2.0f;
    float durationMultiplier = 1.0f + chargeRatio * 1.5f;

    // 设置冲刺状态
    player.isDashing = true;
    player.dashTimer = DASH_DURATION * durationMultiplier;
    player.dashCooldown = DASH_COOLDOWN * (0.5f + chargeRatio * 0.5f);
    player.dashDirectionX = dirX;
    player.dashDirectionY = dirY;

    // 设置冲刺速度
    player.velocityX = dirX * DASH_SPEED * speedMultiplier;
    player.velocityY = dirY * DASH_SPEED * speedMultiplier;

    // 结束蓄力状态
    player.isCharging = false;
    player.chargeTime = 0.0f;
}

void DashSystem::CancelChargeDash(Player& player) {
    if (player.isCharging) {
        player.isCharging = false;
        player.chargeTime = 0.0f;
    }
}

void DashSystem::ToggleDashType() {
    if (m_currentDashType == DASH_INSTANT) {
        m_currentDashType = DASH_CHARGE;
    }
    else {
        m_currentDashType = DASH_INSTANT;
    }
}

DashType DashSystem::GetCurrentDashType() const {
    return m_currentDashType;
}

bool DashSystem::IsKeyDown(int key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}