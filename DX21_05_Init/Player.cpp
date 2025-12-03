#include "Game.h"
#include "Enemy.h"

// 玩家物理更新
void UpdatePlayerPhysics(float deltaTime) {
    // 在硬直状态下忽略重力和移动
    if (g_player.isInDashAftermath) {
        // 只处理垂直碰撞检测（防止掉出地图）
        auto& solidTiles = g_mapManager.GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                tile.posX, tile.posY, tile.width, tile.height)) {
                // 简单的垂直碰撞处理
                g_player.posY = tile.posY + tile.height; // 站在地面上
                g_player.isOnGround = true;
            }
        }
        return; // 硬直状态下跳过正常物理更新
    }
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

    CheckDashAttack();
}

// 修改UpdateDash函数，添加蓄力衰减更新
void UpdateDash(float deltaTime) {
    // 优先更新冲刺状态
    if (g_player.isDashing) {
        g_player.dashTimer -= deltaTime;

        if (g_player.dashTimer <= 0.0f) {
            g_player.isDashing = false;
            g_player.hasMouseTarget = false;
            EnterDashAftermath(); // 冲刺结束进入硬直
        }
    }

    // 然后更新硬直状态
    UpdateDashAftermath(deltaTime);
    // 最后更新点数恢复系统
    UpdateDashPoints(deltaTime);

    // 更新蓄力衰减计时器
    g_player.UpdateChargeDecay(deltaTime);

    // 蓄力逻辑应该独立于硬直状态
    if (g_player.isCharging) {
        g_player.chargeTime += deltaTime;

        // 硬直状态下允许蓄力，但蓄力完成时检查条件
        if (g_player.chargeTime >= g_player.MAX_CHARGE_TIME) {
            // 蓄力完成时，如果处于硬直状态，先清除硬直
            if (g_player.isInDashAftermath) {
                g_player.isInDashAftermath = false;
            }
            ExecuteMouseChargeDash();
        }
    }
}

void CancelChargeDash() {
    if (g_player.isCharging) {
        g_player.isCharging = false;
        g_player.chargeTime = 0.0f;
    }
}

void MovePlayerLeft() {
    // 检查是否处于蓄力状态且不允许移动
    if (g_player.isCharging && !g_player.allowMoveWhileCharging) {
        return;  // 蓄力中不允许移动，直接返回
    }

    // 如果处于硬直状态，移动会打断硬直
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }

    g_player.velocityX = -MOVE_SPEED;
    g_player.isMoving = true;
    g_player.facingRight = false;
}

void MovePlayerRight() {
    // 检查是否处于蓄力状态且不允许移动
    if (g_player.isCharging && !g_player.allowMoveWhileCharging) {
        return;
    }

    // 如果处于硬直状态，移动会打断硬直
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }

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
    // 检查点数是否足够
    if (g_player.dashPoints <= 0) {
        return;
    }

    // 消耗冲刺点数
    if (!ConsumeDashPoint()) {
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

// 修改StartMouseChargeDash函数，添加蓄力继承逻辑
void StartMouseChargeDash() {
    // 检查条件：是否正在冲刺、是否正在蓄力、点数是否足够、是否处于可行动状态
    if (g_player.isDashing || g_player.isCharging || g_player.dashPoints <= 0) {
        return;
    }

    g_player.isCharging = true;

    // 检查是否有保存的蓄力，如果有则继承
    if (g_player.hasSavedCharge) {
        g_player.LoadSavedCharge(); // 加载保存的蓄力时间
        // 不清除保存的蓄力，允许连续继承（直到衰减时间结束）
    }
    else {
        g_player.chargeTime = 0.0f; // 没有保存的蓄力，从头开始
    }

    // 记录初始鼠标位置
    g_inputSystem.GetMousePosition(g_player.mouseTargetX, g_player.mouseTargetY);
    g_player.hasMouseTarget = true;
}

// 修改ExecuteMouseChargeDash函数，在冲刺结束时保存蓄力
void ExecuteMouseChargeDash() {
    if (!g_player.isCharging) return;

    // 允许在硬直状态下进行蓄力冲刺
    if (g_player.dashPoints <= 0) return;

    // 清除硬直状态，允许新的冲刺
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }

    g_player.hitEnemies.clear();
    if (!ConsumeDashPoint()) return;

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

    // 获取当前蓄力等级
    int chargeLevel = g_player.GetChargeLevel();

    // 三段蓄力判定
    float speedMultiplier = 1.0f;
    float durationMultiplier = 1.0f;
    float cooldownMultiplier = 1.0f;

    // 根据蓄力等级设置属性倍率
    switch (chargeLevel) {
    case 1:
        speedMultiplier = 1.3f;
        durationMultiplier = 1.2f;
        cooldownMultiplier = 0.8f;
        break;
    case 2:
        speedMultiplier = 1.6f;
        durationMultiplier = 1.4f;
        cooldownMultiplier = 0.6f;
        break;
    case 3:
        speedMultiplier = 2.0f;
        durationMultiplier = 1.8f;
        cooldownMultiplier = 0.5f;
        break;
    default:
        speedMultiplier = 1.0f;
        durationMultiplier = 1.0f;
        cooldownMultiplier = 1.0f;
        break;
    }

    // 设置冲刺状态
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION * durationMultiplier;
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // 应用冲刺速度
    g_player.velocityX = dirX * DASH_SPEED * speedMultiplier;
    g_player.velocityY = dirY * DASH_SPEED * speedMultiplier;

    // 存储最终鼠标位置
    g_player.mouseTargetX = currentMouseX;
    g_player.mouseTargetY = currentMouseY;

    // === 关键修改：在冲刺结束时保存当前蓄力层数 ===
    // 只有当蓄力时间达到最小阈值时才保存（避免保存无效的短按）
    if (g_player.chargeTime >= g_player.MIN_CHARGE_TIME) {
        g_player.SaveCharge(); // 保存当前蓄力时间
    }

    // 结束蓄力状态
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
}


// 进入冲刺后硬直状态
void EnterDashAftermath() {
    // 清除所有速度，使玩家完全停止
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;

    // 如果没有点数则不进入硬直状态
    if (g_player.dashPoints <= 0) {
        return;
    }

    g_player.isInDashAftermath = true;
    g_player.dashAftermathTimer = g_player.DASH_AFTERMATH_DURATION;
}

// 更新硬直状态
void UpdateDashAftermath(float deltaTime) {
    if (!g_player.isInDashAftermath) return;

    g_player.dashAftermathTimer -= deltaTime;

    // 检查移动输入打断
    if (g_inputSystem.IsMovingLeft() || g_inputSystem.IsMovingRight()) {
        g_player.isInDashAftermath = false;
        g_player.velocityY = 0.0f;
        return;
    }

    // 硬直状态结束
    if (g_player.dashAftermathTimer <= 0.0f) {
        g_player.isInDashAftermath = false;
        g_player.velocityY = 0.0f;
    }
}

// 更新冲刺点数恢复
void UpdateDashPoints(float deltaTime) {
    // 地面恢复点数
    if (g_player.isOnGround && g_player.dashPoints < g_player.MAX_DASH_POINTS) {
        g_player.dashPointRecoverTimer += deltaTime;

        if (g_player.dashPointRecoverTimer >= g_player.DASH_POINT_RECOVER_TIME) {
            g_player.dashPoints++;
            g_player.dashPointRecoverTimer = 0.0f;
        }
    }
    else {
        g_player.dashPointRecoverTimer = 0.0f;
    }
}

// 消耗冲刺点数
bool ConsumeDashPoint() {
    if (g_player.dashPoints > 0) {
        g_player.dashPoints--;
        return true;
    }
    return false;
}

// 击败敌人时恢复点数（预留接口）
void OnEnemyDefeated() {
    if (g_player.dashPoints < g_player.MAX_DASH_POINTS) {
        g_player.dashPoints++;
    }
}


void CheckDashAttack() {
    if (!g_player.isDashing) return; // 仅在冲刺时检测

    // 计算玩家冲刺角度（用于伤害计算）
    float dashAngle = atan2(g_player.dashDirectionY, g_player.dashDirectionX);

    for (auto& enemy : g_enemies) {
        // 跳过已死亡的敌人
        if (!enemy->IsAlive()) continue;

        // 跳过已击中的敌人（避免重复伤害）
        if (std::find(g_player.hitEnemies.begin(), g_player.hitEnemies.end(), enemy) != g_player.hitEnemies.end()) {
            continue;
        }

        // 完整的碰撞检测参数（玩家与敌人）
        bool isColliding = CheckCollision(
            g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight()
        );

        if (isColliding) {
            // 计算实际伤害（基于攻击角度）
            int actualDamage = enemy->CalculateDamageFromPlayer((int)g_player.attackDamage, dashAngle);
            enemy->TakeDamage(actualDamage, dashAngle);

            // 标记敌人进入分裂状态（使用公共方法）
            enemy->SetSplit(true);

            // 记录已击中的敌人
            g_player.hitEnemies.push_back(enemy);
        }
    }
}