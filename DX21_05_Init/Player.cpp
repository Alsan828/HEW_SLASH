#include "Game.h"
#include "Enemy.h"
#include "Audio.h"

static void PerformDashEndCircleHitTest();

static float ComputeDashHitStopTime()
{
    // Base feel (old value)
    constexpr float BASE_HITSTOP = 0.075f;
    // Extra hitstop per charge level (0..3)
    constexpr float EXTRA_PER_LEVEL = 0.025f;
    constexpr float MAX_HITSTOP = 0.18f;

    int chargeLevel = 0;
    if (g_player.hasSavedCharge) {
        chargeLevel = g_player.GetChargeLevelFromTime(g_player.savedChargeTime);
    }
    else {
        chargeLevel = g_player.GetChargeLevelFromTime(g_player.chargeTime);
    }

    float hitStop = BASE_HITSTOP + static_cast<float>(chargeLevel) * EXTRA_PER_LEVEL;
    // Requested: increase hit-stop time after hitting an enemy to 1.5x.
    hitStop *= 1.5f;
    return std::min(hitStop, MAX_HITSTOP);
}

// 冲刺命中检测辅助：给定测试位置，使用缩小并居中的碰撞盒检测敌人
static void PerformDashHitTest(float testX, float testY) {
    if (!g_player.isDashing) {
        return;
    }

    // 冲刺斩击的“攻击判定”不应该等同于冲刺时的“受击判定”(当前物理碰撞盒被缩小到 1/4)。
    // 这里单独放大命中检测盒，避免高速擦肩而过。
    // Requested: slightly increase slash width.
    constexpr float DASH_ATTACK_HITBOX_SCALE = 0.75f;

    float playerWidth = PLAYER_WIDTH;
    float playerHeight = PLAYER_HEIGHT;
    float offsetX = 0.0f;
    float offsetY = 0.0f;

    if (g_player.isDashing) {
        playerWidth = PLAYER_WIDTH * DASH_ATTACK_HITBOX_SCALE;
        playerHeight = PLAYER_HEIGHT * DASH_ATTACK_HITBOX_SCALE;
        offsetX = (PLAYER_WIDTH - playerWidth) * 0.5f;
        offsetY = (PLAYER_HEIGHT - playerHeight) * 0.5f;
    }

    float dashAngle = atan2(g_player.dashDirectionY, g_player.dashDirectionX);

    for (auto& enemy : g_enemies) {
        if (!enemy->IsAlive()) {
            continue;
        }

        // 已命中过的敌人跳过
        if (std::find(g_player.hitEnemies.begin(), g_player.hitEnemies.end(), enemy) != g_player.hitEnemies.end()) {
            continue;
        }

        if (CheckCollision(testX + offsetX, testY + offsetY, playerWidth, playerHeight,
            enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight())) {

            g_player.comboCount++;
            g_player.comboTimer = 5.0f;

            g_gameStats.UpdateMaxCombo(g_player.comboCount);// added january 22nd

            float multiplier = enemy->GetDamageMultiplier(dashAngle);
            // Hit-stop should depend on charge strength, not on invincibility state.
            if (g_player.hitStopTriggered < 3) {
                g_player.hitStopTimer = ComputeDashHitStopTime();
                g_player.hitStopTriggered++;
                if (!g_player.isInvincible) {
                    g_camera.Shake(0.02f, 0.05f);
                    // 可选全局慢动作
                    // TriggerSlowMotion(0.05f, 0.3f);
                }
            }

            if (!g_player.isInvincible)// if player is not invincible
            {
                if (multiplier > 1.5f && g_player.gaugePoints < g_player.MAX_GAUGE_POINTS) {
                    g_player.gaugePoints += 2;
                }
                else if (g_player.gaugePoints < g_player.MAX_GAUGE_POINTS) {
                    g_player.gaugePoints += 1;
                }
                if (g_player.gaugePoints > g_player.MAX_GAUGE_POINTS) {
                    g_player.gaugePoints = g_player.MAX_GAUGE_POINTS;
                }
            }
 
            int actualDamage = enemy->CalculateDamageFromPlayer((int)g_player.attackDamage, dashAngle);
            enemy->TakeDamage(actualDamage, dashAngle);

            g_player.hitEnemies.push_back(enemy);
        }
    }
}

// 冲刺结束点追加一个“小圆形”命中判定：只在到达终点的那一刻触发一次。
// 该判定复用 hitEnemies 去重，因此不会与冲刺过程中的伤害重合。
static void PerformDashEndCircleHitTest() {
    if (!g_player.isDashing || !g_player.hasMouseTarget) {
        return;
    }

    const float forwardDistance = PLAYER_WIDTH * 0.5f;
    const float radius = PLAYER_WIDTH * 0.35f;
    const float radiusSq = radius * radius;

    float endCenterX = g_player.mouseTargetX + g_player.dashDirectionX * forwardDistance;
    float endCenterY = g_player.mouseTargetY + g_player.dashDirectionY * forwardDistance;

    float dashAngle = atan2(g_player.dashDirectionY, g_player.dashDirectionX);

    for (auto& enemy : g_enemies) {
        if (!enemy || !enemy->IsAlive()) {
            continue;
        }

        if (std::find(g_player.hitEnemies.begin(), g_player.hitEnemies.end(), enemy) != g_player.hitEnemies.end()) {
            continue;
        }

        float enemyCenterX = enemy->GetX() + enemy->GetWidth() * 0.5f;
        float enemyCenterY = enemy->GetY() + enemy->GetHeight() * 0.5f;

        float dx = enemyCenterX - endCenterX;
        float dy = enemyCenterY - endCenterY;
        if (dx * dx + dy * dy <= radiusSq) {
            g_player.comboCount++;
            g_player.comboTimer = 5.0f;

            g_gameStats.UpdateMaxCombo(g_player.comboCount); // added january 22nd

            float multiplier = enemy->GetDamageMultiplier(dashAngle);
            // Hit-stop should depend on charge strength, not on invincibility state.
            if (g_player.hitStopTriggered < 3) {
                g_player.hitStopTimer = ComputeDashHitStopTime();
                g_player.hitStopTriggered++;
                if (!g_player.isInvincible) {
                    g_camera.Shake(0.02f, 0.05f);
                }
            }

            if (!g_player.isInvincible) {
                if (multiplier > 1.5f && g_player.gaugePoints < g_player.MAX_GAUGE_POINTS) {
                    g_player.gaugePoints += 2;
                }
                else if (g_player.gaugePoints < g_player.MAX_GAUGE_POINTS) {
                    g_player.gaugePoints += 1;
                }
                if (g_player.gaugePoints > g_player.MAX_GAUGE_POINTS) {
                    g_player.gaugePoints = g_player.MAX_GAUGE_POINTS;
                }
            }

            int actualDamage = enemy->CalculateDamageFromPlayer((int)g_player.attackDamage, dashAngle);
            enemy->TakeDamage(actualDamage, dashAngle);
            g_player.hitEnemies.push_back(enemy);
        }
    }
}

void UpdatePlayerPhysics(float deltaTime) {
    if (g_player.isDead)
        return;

    g_mapManager.GetCurrentMap()->BuildSpatialGrid();//TODO
    // 计算玩家当前的碰撞体大小
    float currentWidth = PLAYER_WIDTH;
    float currentHeight = PLAYER_HEIGHT;

    // 计算碰撞体偏移量，确保中心位置不变
    float offsetX = 0.0f;
    float offsetY = 0.0f;

    // 冲刺时碰撞体缩小为1/4，计算偏移量以保持中心不变
    if (g_player.isDashing) {
        float widthReduction = (PLAYER_WIDTH - PLAYER_WIDTH * 0.25f) / 2.0f;
        float heightReduction = (PLAYER_HEIGHT - PLAYER_HEIGHT * 0.25f) / 2.0f;
        currentWidth = PLAYER_WIDTH * 0.25f;
        currentHeight = PLAYER_HEIGHT * 0.25f;
        offsetX = widthReduction;
        offsetY = heightReduction;
    }

    // 应用重力
    // When no-gravity aftermath mode is enabled, aftermath behaves like a gravity-free state.
    bool ignoreGravity = g_player.isDashing || (g_noGravityAftermathMode && g_player.isInDashAftermath);
    if (!ignoreGravity) {
        float fixedDeltaTime = std::min(deltaTime, 0.033f);

        // 如果在墙壁滑行状态，应用较小的重力
        if (g_player.isWallSliding) {
            // 墙壁滑行时，垂直速度限制在滑行速度
            if (g_player.velocityY < g_player.WALL_SLIDE_SPEED) {
                g_player.velocityY = g_player.WALL_SLIDE_SPEED;
            }
            else if (g_player.velocityY > 0) {
                // 如果向上移动，仍然应用正常重力
                g_player.velocityY += GRAVITY * fixedDeltaTime * 60.0f;
            }
            else {
                // 向下移动时，应用较小的重力
                g_player.velocityY += GRAVITY * 0.3f * fixedDeltaTime * 60.0f;
            }

            // 墙壁滑行时水平速度逐渐减小
            if (g_player.velocityX > 0) {
                g_player.velocityX = std::max(0.0f, g_player.velocityX - 0.1f);
            }
            else if (g_player.velocityX < 0) {
                g_player.velocityX = std::min(0.0f, g_player.velocityX + 0.1f);
            }
        }
        else {
            // 正常重力
            g_player.velocityY += GRAVITY * fixedDeltaTime * 60.0f;
        }

        if (g_player.velocityY < -0.15f) {
            g_player.velocityY = -0.15f;
        }
    }

    // 重置墙壁滑行状态
    g_player.isWallSliding = false;
    g_player.wallSlideDirection = 0;

    // 如果不在冲刺状态且不在地面上，检测墙壁滑行
    // 注意：死亡状态下不应再根据环境改写朝向/状态
    if (!g_player.isDead && !g_player.isDashing && !g_player.isOnGround && g_player.velocityY < 0) {
        // 获取当前地图的空间网格
        SpatialGrid* spatialGrid = g_mapManager.GetCurrentMap()->GetSpatialGrid();
        if (spatialGrid) {
            std::vector<MapTile*> nearbyTiles;

            // 获取玩家周围的瓦片
            float padding = 1.0f;
            spatialGrid->GetTilesInArea(
                g_player.posX + offsetX - padding,
                g_player.posY + offsetY - padding,
                currentWidth + padding * 2,
                currentHeight + padding * 2,
                nearbyTiles
            );

            // 使用玩家的实际碰撞体进行接触检测
            float playerLeft = g_player.posX + offsetX;
            float playerRight = playerLeft + currentWidth;
            float playerTop = g_player.posY + offsetY;
            float playerBottom = playerTop + currentHeight;

            // 定义很小的接触阈值
            const float CONTACT_EPSILON = 0.002f;

            // 检测左右墙壁接触
            for (const auto& tile : nearbyTiles) {
                if (tile->tileInfo.isSolid) {
                    float tileLeft = tile->posX;
                    float tileRight = tile->posX + tile->width;
                    float tileTop = tile->posY;
                    float tileBottom = tile->posY + tile->height;

                    // 检查垂直方向重叠（Y轴有重叠）
                    bool verticalOverlap = (playerTop < tileBottom && playerBottom > tileTop);

                    if (verticalOverlap) {
                        // 检测左侧墙壁接触
                        float leftDistance = tileRight - playerLeft;
                        if (leftDistance >= 0 && leftDistance <= CONTACT_EPSILON) {
                            g_player.isWallSliding = true;
                            g_player.wallSlideDirection = -1; // 左侧墙
                            g_player.facingRight = true;
                            break; // 找到一面墙就足够
                        }

                        // 检测右侧墙壁接触
                        float rightDistance = playerRight - tileLeft;
                        if (rightDistance >= 0 && rightDistance <= CONTACT_EPSILON) {
                            g_player.isWallSliding = true;
                            g_player.wallSlideDirection = 1;  // 右侧墙
                            g_player.facingRight = false;
                            break; // 找到一面墙就足够
                        }
                    }
                }
            }
        }
    }

    // 保存原始位置用于碰撞检测
    float oldX = g_player.posX;
    float oldY = g_player.posY;

    // 计算移动距离
    float moveX = g_player.velocityX * deltaTime * 60.0f;
    float moveY = g_player.velocityY * deltaTime * 60.0f;

    // 获取当前地图的空间网格
    SpatialGrid* spatialGrid = g_mapManager.GetCurrentMap()->GetSpatialGrid();

    if (!spatialGrid) {
        // 如果没有空间网格，使用原始方法
        g_player.posX += moveX;
        g_player.posY += moveY;
    }
    else {
        // === 根据速度动态调整检测次数 ===
        float speed = sqrt(g_player.velocityX * g_player.velocityX + g_player.velocityY * g_player.velocityY);
        int steps = 1;

        if (g_player.isDashing) {
            // 冲刺：基础8步，按速度提升，封顶16步
            steps = std::min(std::max(16, static_cast<int>(ceilf(speed * 20.0f))), 32);
        }
        else if (speed > 0.2f) {
            // 高速移动时增加检测次数
            steps = std::min(static_cast<int>(speed * 10.0f), 16);
        }
        else {
            // 正常移动
            steps = 4; // 保持原来的4次
        }

        // === 预收集所有可能碰撞的瓦片 ===
        std::vector<MapTile*> nearbyTiles;

        // 计算移动范围，扩大检测区域
        float paddingX = 3.0f;
        float paddingY = 3.0f;

        // 预测移动后的位置范围
        float minX = std::min(g_player.posX + offsetX, g_player.posX + offsetX + moveX) - paddingX;
        float minY = std::min(g_player.posY + offsetY, g_player.posY + offsetY + moveY) - paddingY;
        float maxX = std::max(g_player.posX + offsetX + currentWidth,
            g_player.posX + offsetX + moveX + currentWidth) + paddingX;
        float maxY = std::max(g_player.posY + offsetY + currentHeight,
            g_player.posY + offsetY + moveY + currentHeight) + paddingY;

        // 获取移动范围内的所有瓦片
        spatialGrid->GetTilesInArea(minX, minY, maxX - minX, maxY - minY, nearbyTiles);

        // 分离普通固体瓦片和单向平台
        std::vector<MapTile*> regularSolidTiles;
        std::vector<MapTile*> oneWayPlatformTiles;

        for (const auto& tile : nearbyTiles) {
            if (tile->tileInfo.isSolid) {
                if (tile->tileInfo.type == "platform" && tile->tileInfo.subtype == "one_way") {
                    oneWayPlatformTiles.push_back(tile);
                }
                else {
                    regularSolidTiles.push_back(tile);
                }
            }
        }

        // 使用连续碰撞检测
        float stepX = moveX / steps;
        float stepY = moveY / steps;

        for (int i = 0; i < steps; i++) {
            g_player.posX += stepX;

            // 水平碰撞检测（只检测普通固体，单向平台不影响水平移动）
            bool xCollision = false;
            for (const auto& tile : regularSolidTiles) {
                if (CheckCollision(g_player.posX + offsetX, g_player.posY + offsetY,
                    currentWidth, currentHeight,
                    tile->posX, tile->posY, tile->width, tile->height)) {

                    // 计算碰撞方向和回退
                    float playerCenterX = g_player.posX + offsetX + currentWidth / 2;
                    float playerCenterY = g_player.posY + offsetY + currentHeight / 2;
                    float tileCenterX = tile->posX + tile->width / 2;
                    float tileCenterY = tile->posY + tile->height / 2;

                    // 计算重叠
                    float overlapX = (currentWidth / 2 + tile->width / 2) - fabs(playerCenterX - tileCenterX);
                    float overlapY = (currentHeight / 2 + tile->height / 2) - fabs(playerCenterY - tileCenterY);

                    // 选择最小重叠方向
                    if (overlapX < overlapY) {
                        // 水平碰撞
                        if (playerCenterX < tileCenterX) {
                            g_player.posX = tile->posX - currentWidth - offsetX;
                        }
                        else {
                            g_player.posX = tile->posX + tile->width - offsetX;
                        }
                        g_player.velocityX = 0.0f;
                    }
                    else {
                        // 垂直碰撞
                        if (playerCenterY < tileCenterY) {
                            g_player.posY = tile->posY - currentHeight - offsetY;
                        }
                        else {
                            g_player.posY = tile->posY + tile->height - offsetY;
                        }
                        g_player.velocityY = 0.0f;
                        if (stepY < 0) g_player.isOnGround = true;
                    }
                    xCollision = true;
                    break;
                }
            }

            g_player.posY += stepY;

            // 垂直碰撞检测（先检测普通固体）
            bool yCollision = false;
            float preCollisionY = g_player.posY - stepY; // 记录碰撞前的Y位置

            for (const auto& tile : regularSolidTiles) {
                if (CheckCollision(g_player.posX + offsetX, g_player.posY + offsetY,
                    currentWidth, currentHeight,
                    tile->posX, tile->posY, tile->width, tile->height)) {

                    // 计算碰撞方向和回退
                    float playerCenterX = g_player.posX + offsetX + currentWidth / 2;
                    float playerCenterY = g_player.posY + offsetY + currentHeight / 2;
                    float tileCenterX = tile->posX + tile->width / 2;
                    float tileCenterY = tile->posY + tile->height / 2;

                    // 计算重叠
                    float overlapX = (currentWidth / 2 + tile->width / 2) - fabs(playerCenterX - tileCenterX);
                    float overlapY = (currentHeight / 2 + tile->height / 2) - fabs(playerCenterY - tileCenterY);

                    // 选择最小重叠方向
                    if (overlapX < overlapY) {
                        // 水平碰撞
                        if (playerCenterX < tileCenterX) {
                            g_player.posX = tile->posX - currentWidth - offsetX;
                        }
                        else {
                            g_player.posX = tile->posX + tile->width - offsetX;
                        }
                        g_player.velocityX = 0.0f;
                    }
                    else {
                        // 垂直碰撞
                        if (playerCenterY < tileCenterY) {
                            g_player.posY = tile->posY - currentHeight - offsetY;
                        }
                        else {
                            g_player.posY = tile->posY + tile->height - offsetY;
                        }
                        g_player.velocityY = 0.0f;
                        if (stepY < 0) g_player.isOnGround = true;
                    }
                    yCollision = true;
                    break;
                }
            }

            // 如果没有与普通固体碰撞，检测单向平台
            if (!yCollision) {
                for (const auto& tile : oneWayPlatformTiles) {
                    // 计算玩家的边界
                    float playerLeft = g_player.posX + offsetX;
                    float playerRight = playerLeft + currentWidth;
                    float playerTop = g_player.posY + offsetY;
                    float playerBottom = playerTop + currentHeight;

                    // 计算平台的边界
                    float platformLeft = tile->posX;
                    float platformRight = platformLeft + tile->width;
                    float platformTop = tile->posY;
                    float platformBottom = platformTop + tile->height;

                    // 1. 检测水平方向的重叠
                    bool horizontalOverlap = (playerRight > platformLeft && playerLeft < platformRight);

                    if (!horizontalOverlap)
                        continue;

                    if (g_player.velocityY >= 0.0f) 
                        continue; 

                    float currentBottom = playerBottom;

                    if (currentBottom > platformTop + currentHeight + offsetY) {
                        if (g_inputSystem.IsKeyDown(VK_S)) {
                            g_player.isOnGround = false;
                            continue;
                        }
                        if (g_player.posY < platformTop + currentHeight + offsetY) {
                            g_player.isOnGround = true;
                            g_player.posY = platformTop + currentHeight + offsetY;
                            g_player.velocityY = 0.0f;
                        }
                        yCollision = true;
                        break;
                    }
                }
            }

            // 如果两个方向都发生碰撞
            if (xCollision && yCollision) {
                // 可以在这里添加对角线碰撞的特殊处理
                g_player.posY -= stepY; // 回退垂直移动
                g_player.posX -= stepX; // 回退水平移动
            }

            // 冲刺时在每个子步位置做敌人命中检测，避免高速穿过
            if (g_player.isDashing) {
                PerformDashHitTest(g_player.posX, g_player.posY);
            }
        }
    }

    // 传送门处理
    static float portalCooldown = 0.0f;
    if (portalCooldown > 0.0f) {
        portalCooldown -= deltaTime;
    }

    if (portalCooldown <= 0.0f) {
        std::string targetMap;
        int portalId, linkedSpawnId;

        // 使用正确的碰撞体大小检测传送门
        float portalWidth = PLAYER_WIDTH;
        float portalHeight = PLAYER_HEIGHT;
        float portalOffsetX = 0.0f;
        float portalOffsetY = 0.0f;

        if (g_player.isDashing) {
            portalWidth = PLAYER_WIDTH * 0.25f;
            portalHeight = PLAYER_HEIGHT * 0.25f;
            portalOffsetX = (PLAYER_WIDTH - portalWidth) / 2.0f;
            portalOffsetY = (PLAYER_HEIGHT - portalHeight) / 2.0f;
        }

        if (g_mapManager.GetCurrentMap()->CheckPortalCollision(
            g_player.posX + portalOffsetX, g_player.posY + portalOffsetY,
            portalWidth, portalHeight,
            targetMap, portalId, linkedSpawnId)) {

            if (targetMap == "boss") {
                // it goes to boss of world1
                //sceneManager.SwitchScene(BOSS);
                
                // it goes to boss stage (World 1, Stage 8)
                sceneManager.SwitchToStage(1, 8);
                portalCooldown = 1.0f;
            }

            else {
                g_mapManager.SwitchMap(targetMap, portalId, linkedSpawnId);
                portalCooldown = 1.0f;
            }
        }
    }

    // 如果垂直速度绝对值 > 0.05f，认为玩家不在地面上
    if (fabs(g_player.velocityY) > 0.05f) {
        g_player.isOnGround = false;
    }

    // 边界检查
    if (g_player.posY < -4.0f) {
        g_gameStats.IncrementDeaths();

        // Reset combo on death
        g_player.comboCount = 0;
        g_player.comboTimer = 0.0f;

        // only counts deaths if the player has points
        int killPoints = (g_gameStats.GetEnemiesKilled() * 10) + (g_gameStats.GetWeakPointKills() * 30);
        if (killPoints > 0) {
            g_gameStats.IncrementPenalizableDeaths();
        }

        ResetGame();
    }

    CheckDashAttack();
}

void CheckDashAttack() {
    if (!g_player.isDashing) {
        g_player.hitEnemies.clear();
        g_player.hitStopTriggered = 0; // 重置顿刀触发计数
        g_player.hitStopTimer = 0.0f;  // 重置顿刀计时器
        return;
    }

    // 使用当前最终位置检测（作为兜底），子步检测已在UpdatePlayerPhysics中进行
    PerformDashHitTest(g_player.posX, g_player.posY);
}

// New: Update player death state
void UpdatePlayerDeath(float deltaTime) {
    if (!g_player.isDead) {
        return;
    }

    // Reset combo when player dies
    g_player.comboCount = 0;
    g_player.comboTimer = 0.0f;

    g_player.deathTimer -= deltaTime;

    if (g_player.deathTimer <= 0.0f) {
        ResetGame();
    }
}

// New: Player death handler
void OnPlayerDeath() {
    g_player.isDead = true;
    g_player.deathTimer = g_player.DEATH_RESPAWN_TIME;
    g_player.deathCount++;

    // Small camera shake on player getting hit/death.
    // Keep it subtle to avoid nausea.
    g_camera.Shake(0.015f, 0.08f);

    // Track death
    g_gameStats.IncrementDeaths();

    g_player.comboCount = 0;
    g_player.comboTimer = 0.0f;

    // only count deaths if the player has points
    int killPoints = (g_gameStats.GetEnemiesKilled() * 10) + (g_gameStats.GetWeakPointKills() * 30);
    if (killPoints > 0) {
        g_gameStats.IncrementPenalizableDeaths();
    }

    // Stop all player actions
    g_player.isMoving = false;
    g_player.isDashing = false;
    g_player.isCharging = false;
    g_player.isInDashAftermath = false;
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;

    // Death sound can be added here
    // g_audioManager.PlaySFX("death_sound.wav");
}

// New: Check if player should die
void CheckPlayerDeath() {
    if (g_player.isDead || g_player.isInvincible) {
        return;
    }

    // Check collision with all alive enemies
    for (auto& enemy : g_enemies) {
        if (!enemy->IsAlive()) continue;

        if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight())) {

            // If player is dashing, they won't die but will attack the enemy instead
            if (g_player.isDashing) {
                // Attack logic handled in CheckDashAttack
                continue;
            }
            else {
                // Otherwise, player dies
                OnPlayerDeath();
            }
            return;
        }
    }

}

// Modified UpdateDash function: add charge decay update
void UpdateDash(float deltaTime) {

    // Prioritize dashing state update
    if (g_player.isDashing) {
        g_player.dashTimer -= deltaTime;

        if (g_player.dashTimer <= 0.0f) {
            g_player.isDashing = false;
            g_player.hasMouseTarget = false;

            // Brief invincibility window after dash ends
            g_player.isInvincible = true;
            g_player.invincibleTimer = std::max(g_player.invincibleTimer, 0.2f);

            // Requested: after dash ends, enter a real-time 0.75s slow-motion at 75% speed.
            // Player is invincible during this time.
            g_player.isInDashEndSlowMo = true;
            g_player.dashEndSlowMoTimer = g_player.DASH_END_SLOWMO_REALTIME;
            g_player.invincibleTimer = std::max(g_player.invincibleTimer, g_player.DASH_END_SLOWMO_REALTIME);

            EnterDashAftermath(); // Enter aftermath when dash ends
        }
    }

    // Update dash-end slow motion timer in real time (unscaled).
    if (g_player.isInDashEndSlowMo) {
        g_player.dashEndSlowMoTimer -= deltaTime;
        if (g_player.dashEndSlowMoTimer <= 0.0f) {
            g_player.isInDashEndSlowMo = false;
            g_player.dashEndSlowMoTimer = 0.0f;
        }
    }

    // Then update aftermath state
    UpdateDashAftermath(deltaTime);
    // Update dash point recovery system
    UpdateDashPoints(deltaTime);

    // Update charge decay timer
    g_player.UpdateChargeDecay(deltaTime);

    // Charge logic should be independent of aftermath state
    if (g_player.isCharging) {
        g_player.chargeTime += deltaTime * g_player.GetChargeSpeedMultiplier();

        // Allow charging during stun, but charge time cannot be too long
        if (g_player.chargeTime >= g_player.MAX_CHARGE_TIME) {
            // When charge time is max, clear aftermath first if in aftermath
            /*if (g_player.isInDashAftermath) {
                g_player.isInDashAftermath = false;
            }
            ExecuteMouseChargeDash();*/
            g_player.chargeTime = g_player.MAX_CHARGE_TIME; // it caps to max charge time and it doesnt release it unless you stop clicking
        }
    }


    CheckPlayerDeath();
}

void CancelChargeDash() {
    if (g_player.isCharging) {
        g_player.isCharging = false;
        g_player.chargeTime = 0.0f;
    }
}
void MovePlayerLeft() {
    // 死亡中禁止任何移动输入
    if (g_player.isDead) {
        return;
    }
    // 检查是否正在蓄力且不允许移动
    if (g_player.isCharging && !g_player.allowMoveWhileCharging) {
        return;
    }

    // 如果在硬直中，移动会中断它
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }
    // Also break dash-end slow motion on movement.
    if (g_player.isInDashEndSlowMo) {
        g_player.isInDashEndSlowMo = false;
        g_player.dashEndSlowMoTimer = 0.0f;
    }

    g_player.velocityX = -MOVE_SPEED * g_player.GetMoveSpeedMultiplier();
    g_player.isMoving = true;
    g_player.facingRight = false;
}

void MovePlayerRight() {
    // 死亡中禁止任何移动输入
    if (g_player.isDead) {
        return;
    }
    // 检查是否正在蓄力且不允许移动
    if (g_player.isCharging && !g_player.allowMoveWhileCharging) {
        return;
    }

    // 如果在硬直中，移动会中断它
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }
    // Also break dash-end slow motion on movement.
    if (g_player.isInDashEndSlowMo) {
        g_player.isInDashEndSlowMo = false;
        g_player.dashEndSlowMoTimer = 0.0f;
    }

    g_player.velocityX = MOVE_SPEED * g_player.GetMoveSpeedMultiplier();
    g_player.isMoving = true;
    g_player.facingRight = true;
}

void StopPlayer() {
    if (g_player.isDead) {
        g_player.velocityX = 0.0f;
        g_player.isMoving = false;
        return;
    }
    if (!g_player.isDashing) {
        g_player.velocityX = 0.0f;
    }
    g_player.isMoving = false;
}

// Improved jump function
void Jump() {
    if (g_player.isDead) {
        return;
    }
    if (g_player.isDashing || g_player.isCharging) {
        return;
    }

    // Wall jump: when sliding on a wall, jump to the opposite direction.
    if (g_player.isWallSliding && g_player.wallSlideDirection != 0) {
        g_player.isWallSliding = false;

        g_player.velocityY = JUMP_FORCE;
        // Push away from wall: wallSlideDirection is -1 for left wall, +1 for right wall.
        // We want to jump opposite, so use negative direction.
        g_player.velocityX = (-static_cast<float>(g_player.wallSlideDirection)) * MOVE_SPEED * 1.5f;
        g_player.facingRight = (g_player.wallSlideDirection == -1);
        g_player.isOnGround = false;
        return;
    }

    // Normal jump
    if (g_player.isOnGround) {
        g_player.velocityY = JUMP_FORCE;
        g_player.isOnGround = false;
    }
}

// Method 3: Mouse direction dash
void DashToMouse() {
    if (g_player.isDead) {
        return;
    }
    // Check if points are sufficient
    if (g_player.dashPoints <= 0) {
        return;
    }

    // Consume dash point
    if (!ConsumeDashPoint()) {
        return;
    }

	Audio::PlaySE(SoundEffect::DASH);

    g_mouseIndicator.showArrow(false);
    // Get mouse world coordinates
    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY);

    // Calculate direction vector from player to mouse
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dirX = mouseX - playerCenterX;
    float dirY = mouseY - playerCenterY;

    // Normalize direction vector
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }
    else {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
        dirY = 0.0f;
    }

    // Set dash state
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION;
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // Face towards dash direction during slash.
    if (fabsf(dirX) > 1e-4f) {
        g_player.facingRight = (dirX >= 0.0f);
    }

    // Set dash speed
    g_player.velocityX = dirX * DASH_SPEED;
    g_player.velocityY = dirY * DASH_SPEED;

    // Store mouse target position
    g_player.mouseTargetX = mouseX;
    g_player.mouseTargetY = mouseY;
    g_player.hasMouseTarget = true;
}
// 修改后的 StartMouseChargeDash 函数
void StartMouseChargeDash() {
    if (g_player.isDead) {
        return;
    }
    // 条件：不在冲刺、不在蓄力、有点数、在可行动状态
    if (g_player.isDashing || g_player.isCharging || g_player.dashPoints <= 0) {
        return;
    }

    g_mouseIndicator.showArrow(true);
    g_player.isCharging = true;
	Audio::PlaySE(SoundEffect::CHARGE_START);
    g_inputSystem.GetMousePosition(g_player.mouseTargetX, g_player.mouseTargetY);
    g_player.hasMouseTarget = true;

    // 重置蓄力时间，从0开始计算本次蓄力
    g_player.chargeTime = 0.0f;
    // 不清除保存的蓄力，但本次重新开始
}

// 修改后的 ExecuteMouseChargeDash 函数
void ExecuteMouseChargeDash() {
    if (g_player.isDead) {
        return;
    }
    if (!g_player.isCharging) return;

    if (g_player.dashPoints <= 0) return;

    g_mouseIndicator.showArrow(false);
	Audio::PlaySE(SoundEffect::CHARGE_RELEASE);
    // 清除硬直状态以允许新的冲刺
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }

    // Starting a dash should interrupt dash-end slow motion.
    if (g_player.isInDashEndSlowMo) {
        g_player.isInDashEndSlowMo = false;
        g_player.dashEndSlowMoTimer = 0.0f;
    }

    g_player.hitEnemies.clear();
    if (!ConsumeDashPoint()) return;

    // 获取当前鼠标位置
    float currentMouseX, currentMouseY;
    g_inputSystem.GetMousePosition(currentMouseX, currentMouseY);

    // 计算从玩家到鼠标的方向
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dirX = currentMouseX - playerCenterX;
    float dirY = currentMouseY - playerCenterY;

    // 标准化方向
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }
    else {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
        dirY = 0.0f;
    }

    // === 关键修改：判断是短按还是长按 ===
    int chargeLevel = 0;
    float speedMultiplier = 1.0f;
    float durationMultiplier = 1.0f;
    float cooldownMultiplier = 1.0f;

    if (g_player.chargeTime < g_player.CHARGE_THRESHOLD_LOW) {
        // 短按：使用保存的蓄力（如果有的话）
        if (g_player.hasSavedCharge) {
            // 使用保存的蓄力等级
            int savedChargeLevel = g_player.GetChargeLevelFromTime(g_player.savedChargeTime);
            chargeLevel = savedChargeLevel;

            // 根据保存的蓄力等级设置属性倍数
            switch (savedChargeLevel) {
            case 1:
                speedMultiplier = 1.3f;
                cooldownMultiplier = 0.8f;
                break;
            case 2:
                speedMultiplier = 1.6f;
                cooldownMultiplier = 0.6f;
                break;
            case 3:
                speedMultiplier = 2.0f;
                cooldownMultiplier = 0.5f;
                break;
            default:
                speedMultiplier = 1.0f;
                cooldownMultiplier = 1.0f;
                break;
            }

            // 短按不保存新的蓄力，保留原来的蓄力
            // 但重置衰减计时器，让保存的蓄力保持更久
            g_player.chargeDecayTimer = g_player.CHARGE_DECAY_TIME;

            // 关键：本次冲刺实际“使用”的是保存蓄力。
            // 将其写回 chargeTime，保证命中顿刀等效果读取到正确的蓄力强度。
            g_player.chargeTime = g_player.savedChargeTime;
        }
        else {
            // 没有保存的蓄力，则使用当前短暂的蓄力时间
            chargeLevel = 0; // 相当于无蓄力
            speedMultiplier = 1.0f;
            durationMultiplier = 1.0f;
            cooldownMultiplier = 1.0f;
        }
    }
    else {
        // 长按：使用本次蓄力，并保存
        chargeLevel = g_player.GetChargeLevel();

        // 根据当前蓄力等级设置属性倍数
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

        // 长按蓄力：保存当前蓄力时间
        if (g_player.chargeTime >= g_player.MIN_CHARGE_TIME) {
            g_player.SaveCharge(); // 保存当前蓄力时间
        }
        else {
            g_player.ClearSavedCharge(); // 时间太短，清除保存的蓄力
        }
    }
    
    // 设置冲刺状态
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION * durationMultiplier;
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // Face towards dash direction during slash.
    if (fabsf(dirX) > 1e-4f) {
        g_player.facingRight = (dirX >= 0.0f);
    }

    // 应用冲刺速度
    g_player.velocityX = dirX * DASH_SPEED * speedMultiplier;
    g_player.velocityY = dirY * DASH_SPEED * speedMultiplier;

    // 存储新的鼠标目标位置
    g_player.mouseTargetX = currentMouseX;
    g_player.mouseTargetY = currentMouseY;

    // 结束蓄力状态
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
}
// Enter dash aftermath state
void EnterDashAftermath() {
    // 在进入硬直前，补一次“终点小圆形”攻击判定（只触发一次）。
    // 注意：这里仍处于 isDashing=true，因此会复用 hitEnemies 去重，不与冲刺过程伤害重合。
    PerformDashEndCircleHitTest();

    // Clear all velocity to keep player stationary
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;

    // Do not enter aftermath if no points left
    if (g_player.dashPoints <= 0) {
        g_player.ClearSavedCharge();
        return;
    }

    g_player.isInDashAftermath = true;
    g_player.dashAftermathTimer = g_player.DASH_AFTERMATH_DURATION;
}

// Update aftermath state
void UpdateDashAftermath(float deltaTime) {
    if (!g_player.isInDashAftermath) return;

    g_player.dashAftermathTimer -= deltaTime;

    // Check for movement input to interrupt
    if (g_inputSystem.IsMovingLeft() || g_inputSystem.IsMovingRight()) {
        g_player.isInDashAftermath = false;
        g_player.velocityY = 0.0f;
        return;
    }

    // Aftermath state ends
    if (g_player.dashAftermathTimer <= 0.0f) {
        g_player.isInDashAftermath = false;
        g_player.velocityY = 0.0f;
    }
}

// Update dash point recovery
void UpdateDashPoints(float deltaTime) {
    // Ground recovery of points:
    // - isOnGround 连续保持 > 1.0s 之后才开始回复
    // - 开始回复后每 0.25s 回复 1 点直到回满
    constexpr float DASH_POINT_RECOVER_DELAY = 0.3f;
    constexpr float DASH_POINT_RECOVER_INTERVAL = 0.2f;

    if (!g_player.isOnGround || g_player.dashPoints >= g_player.MAX_DASH_POINTS) {
        g_player.dashPointRecoverTimer = 0.0f;
        return;
    }

    g_player.dashPointRecoverTimer += deltaTime;

    if (g_player.dashPointRecoverTimer <= DASH_POINT_RECOVER_DELAY) {
        return;
    }

    float timeAfterDelay = g_player.dashPointRecoverTimer - DASH_POINT_RECOVER_DELAY;
    int pointsToRecover = static_cast<int>(floorf(timeAfterDelay / DASH_POINT_RECOVER_INTERVAL));
    if (pointsToRecover <= 0) {
        return;
    }

    int missing = g_player.MAX_DASH_POINTS - g_player.dashPoints;
    int actualRecover = std::min(pointsToRecover, missing);
    g_player.dashPoints += actualRecover;

    // 保留余量时间，实现“每0.25秒一次”的稳定节拍
    float leftover = fmodf(timeAfterDelay, DASH_POINT_RECOVER_INTERVAL);
    g_player.dashPointRecoverTimer = DASH_POINT_RECOVER_DELAY + leftover;
}

// Consume dash point
bool ConsumeDashPoint() {
    if (g_player.dashPoints > 0) {
        g_player.dashPoints--;
        return true;
    }
    return false;
}

// Restore point on enemy defeat (reserved interface)
void OnEnemyDefeated() {
    // Track kill for statistics
    g_gameStats.IncrementKills();

   /* if (g_player.dashPoints < g_player.MAX_DASH_POINTS) {
        g_player.dashPoints++;
    }*/
}

void OnEnemyDefeated(bool wasWeakPointKill) {
    // for 30 points kill
    if (wasWeakPointKill) {
        g_gameStats.IncrementWeakPointKills();
    }
    else { // for 10 points kill
        g_gameStats.IncrementKills();
    }

    if (g_player.dashPoints < g_player.MAX_DASH_POINTS) {
        g_player.dashPoints++;
    }
}
