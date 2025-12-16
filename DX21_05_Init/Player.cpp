#include "Game.h"
#include "Enemy.h"

// 玩家蝸E丒�
void UpdatePlayerPhysics(float deltaTime) {
    // 在硬直状态下忽略重力和移动
    if (g_player.isInDashAftermath) {
        // 只处历泄直碰撞紒E猓ǚ乐沟舫龅赝迹�
        auto& solidTiles = g_mapManager.GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                tile.posX, tile.posY, tile.width, tile.height)) {
                // 简单的垂直碰撞处纴E
                g_player.posY = tile.posY + tile.height; // 站在地面上
                g_player.isOnGround = true;
            }
        }
        return; // 硬直状态下跳过正常蝸E丒�
    }

    // 应用重力
    if (!g_player.isDashing) {
        float fixedDeltaTime = std::min(deltaTime, 0.033f);
        g_player.velocityY += GRAVITY * fixedDeltaTime * 60.0f;
        if (g_player.velocityY < -0.3f) {
            g_player.velocityY = -0.3f;
        }
    }

    // 保存原始位置用于碰撞检测
    float oldX = g_player.posX;
    float oldY = g_player.posY;

    // 计算要移动的距离
    float moveX = g_player.velocityX * deltaTime * 60.0f;
    float moveY = g_player.velocityY * deltaTime * 60.0f;

    // 获取当前地图的空间网格
    SpatialGrid* spatialGrid = g_mapManager.GetCurrentMap()->GetSpatialGrid();
    if (!spatialGrid) {
        // 如果空间网格未构建，回退到原始方法
        auto& solidTiles = g_mapManager.GetCurrentMap()->GetSolidTiles();

        // === 修复：无论是否冲刺，只要速度超过阈值就使用连续碰撞检测 ===
        float speedSquared = g_player.velocityX * g_player.velocityX + g_player.velocityY * g_player.velocityY;
        float speedThreshold = 0.5f; // 速度阈值，超过这个值就使用连续碰撞检测

        if (g_player.isDashing || speedSquared > speedThreshold * speedThreshold) {
            int steps = 4; // 将移动路径分成4步进行检测
            float stepX = moveX / steps;
            float stepY = moveY / steps;

            for (int i = 0; i < steps; i++) {
                g_player.posX += stepX;

                // 水平碰撞检测
                for (const auto& tile : solidTiles) {
                    if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                        tile.posX, tile.posY, tile.width, tile.height)) {
                        // 回退到碰撞前的位置
                        g_player.posX -= stepX;
                        g_player.velocityX = 0.0f;

                        // 计算碰撞法线并反弹
                        if (moveX > 0) {
                            // 向右移动时碰撞
                            g_player.posX = tile.posX - PLAYER_WIDTH;
                        }
                        else if (moveX < 0) {
                            // 向左移动时碰撞
                            g_player.posX = tile.posX + tile.width;
                        }
                        break;
                    }
                }

                g_player.posY += stepY;

                // 垂直碰撞检测
                for (const auto& tile : solidTiles) {
                    if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                        tile.posX, tile.posY, tile.width, tile.height)) {
                        // 回退到碰撞前的位置
                        g_player.posY -= stepY;

                        if (moveY > 0) {
                            // 向上移动时碰撞
                            g_player.posY = tile.posY - PLAYER_HEIGHT;
                            g_player.velocityY = 0.0f;
                        }
                        else if (moveY < 0) {
                            // 向下移动时碰撞
                            g_player.posY = tile.posY + tile.height;
                            g_player.velocityY = 0.0f;
                            g_player.isOnGround = true;
                        }
                        break;
                    }
                }
            }
        }
        else {
            // 正常移动使用分离轴碰撞处理
            g_player.posX += moveX;
            g_player.posY += moveY;

            // 重置落地状态
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

                    // 分离轴处理：选择最小重叠方向
                    if (overlapX < overlapY) {
                        // 水平碰撞
                        if (playerCenterX < tileCenterX) {
                            g_player.posX = tile.posX - PLAYER_WIDTH;
                        }
                        else {
                            g_player.posX = tile.posX + tile.width;
                        }
                        g_player.velocityX = 0.0f;
                    }
                    else {
                        // 垂直碰撞
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
        }
    }
    else {
        // 使用空间网格优化的碰撞检测
        std::vector<MapTile*> nearbyTiles;

        // 获取玩家周围的砖块
        float padding = 1.0f;  // 扩展一点范围
        spatialGrid->GetTilesInArea(
            g_player.posX - padding,
            g_player.posY - padding,
            PLAYER_WIDTH + padding * 2,
            PLAYER_HEIGHT + padding * 2,
            nearbyTiles
        );

        // === 修复：无论是否冲刺，只要速度超过阈值就使用连续碰撞检测 ===
        float speedSquared = g_player.velocityX * g_player.velocityX + g_player.velocityY * g_player.velocityY;
        float speedThreshold = 0.1f; // 速度阈值，超过这个值就使用连续碰撞检测

        if (g_player.isDashing || speedSquared > speedThreshold * speedThreshold) {
            int steps = 4;
            float stepX = moveX / steps;
            float stepY = moveY / steps;

            for (int i = 0; i < steps; i++) {
                g_player.posX += stepX;

                // 水平碰撞检测
                for (const auto& tile : nearbyTiles) {
                    if (tile->tileInfo.isSolid &&
                        CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                            tile->posX, tile->posY, tile->width, tile->height)) {
                        // 回退到碰撞前的位置
                        g_player.posX -= stepX;
                        g_player.velocityX = 0.0f;

                        // 计算碰撞法线
                        if (moveX > 0) {
                            // 向右移动时碰撞
                            g_player.posX = tile->posX - PLAYER_WIDTH;
                        }
                        else if (moveX < 0) {
                            // 向左移动时碰撞
                            g_player.posX = tile->posX + tile->width;
                        }
                        break;
                    }
                }

                g_player.posY += stepY;

                // 垂直碰撞检测
                for (const auto& tile : nearbyTiles) {
                    if (tile->tileInfo.isSolid &&
                        CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                            tile->posX, tile->posY, tile->width, tile->height)) {
                        // 回退到碰撞前的位置
                        g_player.posY -= stepY;

                        if (moveY > 0) {
                            // 向上移动时碰撞
                            g_player.posY = tile->posY - PLAYER_HEIGHT;
                            g_player.velocityY = 0.0f;
                        }
                        else if (moveY < 0) {
                            // 向下移动时碰撞
                            g_player.posY = tile->posY + tile->height;
                            g_player.velocityY = 0.0f;
                            g_player.isOnGround = true;
                        }
                        break;
                    }
                }
            }
        }
        else {
            // 正常移动使用分离轴碰撞处理
            g_player.posX += moveX;
            g_player.posY += moveY;

            g_player.isOnGround = false;

            for (const auto& tile : nearbyTiles) {
                if (tile->tileInfo.isSolid &&
                    CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                        tile->posX, tile->posY, tile->width, tile->height)) {

                    float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
                    float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;
                    float tileCenterX = tile->posX + tile->width / 2;
                    float tileCenterY = tile->posY + tile->height / 2;

                    float overlapX = (PLAYER_WIDTH / 2 + tile->width / 2) - fabs(playerCenterX - tileCenterX);
                    float overlapY = (PLAYER_HEIGHT / 2 + tile->height / 2) - fabs(playerCenterY - tileCenterY);

                    // 分离轴处理：选择最小重叠方向
                    if (overlapX < overlapY) {
                        // 水平碰撞
                        if (playerCenterX < tileCenterX) {
                            g_player.posX = tile->posX - PLAYER_WIDTH;
                        }
                        else {
                            g_player.posX = tile->posX + tile->width;
                        }
                        g_player.velocityX = 0.0f;
                    }
                    else {
                        // 垂直碰撞
                        if (playerCenterY < tileCenterY) {
                            g_player.posY = tile->posY - PLAYER_HEIGHT;
                            g_player.velocityY = 0.0f;
                        }
                        else {
                            g_player.posY = tile->posY + tile->height;
                            g_player.velocityY = 0.0f;
                            g_player.isOnGround = true;
                        }
                    }
                }
            }
        }
    }

    // 传送门紒E丒
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
	//如果velocityY绝对值大于0.05f,则认为玩家不在地面上
    if (fabs(g_player.velocityY) > 0.05f) {
        g_player.isOnGround = false;
    }

    // 边界紒E丒
    if (g_player.posY < -2.0f) {
        ResetGame();
    }

    CheckDashAttack();
}

// 修改UpdateDash函数，添加蓄力衰减更新
void UpdateDash(float deltaTime) {
    // 优先竵E鲁宕套刺�
    if (g_player.isDashing) {
        g_player.dashTimer -= deltaTime;

        if (g_player.dashTimer <= 0.0f) {
            g_player.isDashing = false;
            g_player.hasMouseTarget = false;
            EnterDashAftermath(); // 冲刺结束进葋E仓�
        }
    }

    // 然后竵E掠仓弊刺�
    UpdateDashAftermath(deltaTime);
    // 煮竵E碌闶指聪低�
    UpdateDashPoints(deltaTime);

    // 竵E滦λゼ跫剖逼丒
    g_player.UpdateChargeDecay(deltaTime);

    // 宣荭逻辑应该独立于硬直状态
    if (g_player.isCharging) {
        g_player.chargeTime += deltaTime;

        // 硬直状态下允喧禧荭，但宣荭蛠E墒奔丒樘跫�
        if (g_player.chargeTime >= g_player.MAX_CHARGE_TIME) {
            // 宣荭蛠E墒保绻τ谟仓弊刺惹宄仓�
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
    // 紒E槭欠翊τ谛ψ刺也辉市贫�
    if (g_player.isCharging && !g_player.allowMoveWhileCharging) {
        return;  // 宣荭中不允喧钇动，直接返回
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
    // 紒E槭欠翊τ谛ψ刺也辉市贫�
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

// 方法3: 鼠眮E较虺宕�
void DashToMouse() {
    // 紒E榈闶欠褡愎�
    if (g_player.dashPoints <= 0) {
        return;
    }

    // 消耗冲刺点数
    if (!ConsumeDashPoint()) {
        return;
    }

    // 获取鼠眮E澜缱丒
    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY);

    // 计算从玩家指向鼠眮E姆较蛳蛄�
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

    // 存储鼠眮E勘丒恢�
    g_player.mouseTargetX = mouseX;
    g_player.mouseTargetY = mouseY;
    g_player.hasMouseTarget = true;
}

// 修改StartMouseChargeDash函数，铁赜宣荭继承逻辑
void StartMouseChargeDash() {
    // 紒E樘跫菏欠裾诔宕獭⑹欠裾谛Α⒌闶欠褡愎弧⑹欠翊τ诳尚卸刺�
    if (g_player.isDashing || g_player.isCharging || g_player.dashPoints <= 0) {
        return;
    }

    g_player.isCharging = true;

    // 紒E槭欠裼斜４娴男Γ绻性蚣坛�
    if (g_player.hasSavedCharge) {
        g_player.LoadSavedCharge(); // 加载保存的宣荭时紒E
        // 不清除保存的宣荭，允喧莠续继承（直到衰减时间结束）
    }
    else {
        g_player.chargeTime = 0.0f; // 没有保存的宣荭，从头开始
    }

    // 记录初始鼠眮E恢�
    g_inputSystem.GetMousePosition(g_player.mouseTargetX, g_player.mouseTargetY);
    g_player.hasMouseTarget = true;
}

// 修改ExecuteMouseChargeDash函数，在冲刺结束时保存宣荭
void ExecuteMouseChargeDash() {
    if (!g_player.isCharging) return;

    // 允喧疒硬直状态下进行宣荭冲刺
    if (g_player.dashPoints <= 0) return;

    // 清除硬直状态，允喧炻的冲刺
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }

    g_player.hitEnemies.clear();
    if (!ConsumeDashPoint()) return;

    // 获取当前鼠眮E恢�
    float currentMouseX, currentMouseY;
    g_inputSystem.GetMousePosition(currentMouseX, currentMouseY);

    // 计算从玩家指向鼠眮E姆较丒
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

    // 获取当前宣荭等级
    int chargeLevel = g_player.GetChargeLevel();

    // 三段宣荭判定
    float speedMultiplier = 1.0f;
    float durationMultiplier = 1.0f;
    float cooldownMultiplier = 1.0f;

    // 根据宣荭等级设置属性倍率
    switch (chargeLevel) {
    case 1:
        speedMultiplier = 1.3f;
        durationMultiplier = 1.0f;
        cooldownMultiplier = 0.8f;
        break;
    case 2:
        speedMultiplier = 1.6f;
        durationMultiplier = 1.0f;
        cooldownMultiplier = 0.6f;
        break;
    case 3:
        speedMultiplier = 2.0f;
        durationMultiplier = 1.0f;
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

    // 存储蛘鼠眮E恢�
    g_player.mouseTargetX = currentMouseX;
    g_player.mouseTargetY = currentMouseY;

    // === 关紒E薷模涸诔宕探崾北４娴鼻靶Σ闶� ===
    // 只有当宣荭时间磥E阶°兄凳辈疟４妫ū苊獗４嫖扌У亩贪矗�
    if (g_player.chargeTime >= g_player.MIN_CHARGE_TIME) {
        g_player.SaveCharge(); // 保存当前宣荭时紒E
    }

    // 结束宣荭状态
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
}


// 进葋E宕毯笥仓弊刺�
void EnterDashAftermath() {
    // 清除所有速度，使玩家蛠EＶ�
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;

    // 如果没有点数则不进葋E仓弊刺�
    if (g_player.dashPoints <= 0) {
        g_player.ClearSavedCharge();
        return;
    }

    g_player.isInDashAftermath = true;
    g_player.dashAftermathTimer = g_player.DASH_AFTERMATH_DURATION;
}

// 竵E掠仓弊刺�
void UpdateDashAftermath(float deltaTime) {
    if (!g_player.isInDashAftermath) return;

    g_player.dashAftermathTimer -= deltaTime;

    // 紒E橐贫淙丒蚨�
    if (g_inputSystem.IsMovingLeft() || g_inputSystem.IsMovingRight()) {
        g_player.isInDashAftermath = false;
        g_player.velocityY = 0.0f;
        return;
    }

    // 硬直状态结蕘E
    if (g_player.dashAftermathTimer <= 0.0f) {
        g_player.isInDashAftermath = false;
        g_player.velocityY = 0.0f;
    }
}

// 竵E鲁宕痰闶指�
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
    if (!g_player.isDashing) {
        g_player.hitEnemies.clear();
        return;
    }

    // 计算玩家冲刺角度
    float dashAngle = atan2(g_player.dashDirectionY, g_player.dashDirectionX);

    for (auto& enemy : g_enemies) {
        if (!enemy->IsAlive()) continue;

        // 紒E槭欠褚丫髦泄飧龅腥�
        if (std::find(g_player.hitEnemies.begin(), g_player.hitEnemies.end(), enemy) != g_player.hitEnemies.end()) {
            continue;
        }

        // 紒E馀鲎�
        if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight())) {

            // 直接传葋E婕页宕探嵌龋腥俗约杭扑阆喽苑较丒
            int actualDamage = enemy->CalculateDamageFromPlayer((int)g_player.attackDamage, dashAngle);

            // 对敌人詠E缮撕�
            enemy->TakeDamage(actualDamage, dashAngle);

            // 眮E俏鸦髦�
            g_player.hitEnemies.push_back(enemy);
        }
    }
}