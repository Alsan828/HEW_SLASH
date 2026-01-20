// Enemy.cpp
#include "Enemy.h"
#include "Audio.h"
#include "Map.h"
#include "Projectile.h"

// 初始化伤害数字管理器
std::vector<DamageNumber> DamageNumberManager::damageNumbers;

// 定义敌人纹理
ID3D11ShaderResourceView* g_enemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_enemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_blindEyeEnemyIdleTexture = nullptr;

ID3D11ShaderResourceView* g_flyEnemyIdleTexture = nullptr;  // 改为飞行敌人纹理
ID3D11ShaderResourceView* g_flyEnemyDeathTexture = nullptr;  // 改为飞行敌人死亡纹理

ID3D11ShaderResourceView* g_mageEnemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_mageEnemyAttackTexture = nullptr;
ID3D11ShaderResourceView* g_mageEnemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_fastEnemyRunTexture = nullptr;
ID3D11ShaderResourceView* g_fastEnemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_bombEnemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_bombEnemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_squareEnemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_squareEnemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_beamEnemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_beamEnemyPreAttackTexture = nullptr;
ID3D11ShaderResourceView* g_beamEnemyAttackTexture = nullptr;
ID3D11ShaderResourceView* g_beamEnemyPostAttackTexture = nullptr;
ID3D11ShaderResourceView* g_beamEnemyPreDeathTexture = nullptr;
ID3D11ShaderResourceView* g_beamEnemyDeathTexture = nullptr;
ID3D11ShaderResourceView* g_beamEnemyPostDeathTexture = nullptr;

namespace {
    struct ThrownEnemyState {
        Enemy* enemy = nullptr;
        bool active = false;
        float vx = 0.0f;
        float vy = 0.0f;
    };

    static std::unordered_map<Enemy*, ThrownEnemyState> g_thrownEnemies;

    static void UpdateThrownEnemies(float deltaTime, MapManager* mapManager) {
        if (!mapManager || !mapManager->GetCurrentMap()) {
            return;
        }

        auto* grid = mapManager->GetCurrentMap()->GetSpatialGrid();
        auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();

        for (auto it = g_thrownEnemies.begin(); it != g_thrownEnemies.end();) {
            Enemy* e = it->first;
            ThrownEnemyState& s = it->second;

            if (!e || !e->IsAlive()) {
                it = g_thrownEnemies.erase(it);
                continue;
            }

            if (s.active) {
                it = g_thrownEnemies.erase(it);
                continue;
            }

            float posX = e->GetX();
            float posY = e->GetY();
            const float w = e->GetWidth();
            const float h = e->GetHeight();

            float oldX = posX;
            float oldY = posY;

            float gravityPerFrame = GRAVITY;
            s.vy += gravityPerFrame * deltaTime * 60.0f;

            posX += s.vx * deltaTime * 60.0f;
            posY += s.vy * deltaTime * 60.0f;

            bool landed = false;
            float newY = posY;

            if (grid) {
                std::vector<MapTile*> nearbyTiles;
                grid->GetTilesInArea(posX - 0.5f, posY - 0.5f, w + 1.0f, h + 1.0f, nearbyTiles);
                for (auto* tile : nearbyTiles) {
                    if (!tile || !tile->tileInfo.isSolid) continue;
                    if (CheckCollision(posX, posY, w, h, tile->posX, tile->posY, tile->width, tile->height)) {
                        if (s.vy < 0.0f) {
                            newY = tile->posY + tile->height;
                            landed = true;
                        }
                        break;
                    }
                }
            }
            else {
                for (const auto& tile : solidTiles) {
                    if (!tile.tileInfo.isSolid) continue;
                    if (CheckCollision(posX, posY, w, h, tile.posX, tile.posY, tile.width, tile.height)) {
                        if (s.vy < 0.0f) {
                            newY = tile.posY + tile.height;
                            landed = true;
                        }
                        break;
                    }
                }
            }

            e->SetPosition(posX, landed ? newY : posY);

            // Simple facing based on horizontal velocity
            if (fabs(s.vx) > 0.001f) {
                e->SetFacingRight(s.vx > 0.0f);
            }

            if (landed) {
                s.active = true;
                s.vx = 0.0f;
                s.vy = 0.0f;
                e->SetVelocity(0.0f, 0.0f);
                it = g_thrownEnemies.erase(it);
                continue;
            }

            ++it;
        }
    }
}

// 修改InitEnemies函数，加载所有纹理
void InitEnemies() {
    // 加载敌人纹理
    // 普通敌人
    LoadTexture(g_pDevice, "asset/enemy/enemy_001_eye/enemy_001_eye_idle.png", &g_enemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_001_eye/enemy_001_eye_death.png", &g_enemyDeathTexture);

    // 盲眼普通敌人（idle/普通动画共用同一张图；死亡复用普通敌人死亡）
    LoadTexture(g_pDevice, "asset/enemy/enemy_001_eye/blind_eye.png", &g_blindEyeEnemyIdleTexture);

    // 飞行敌人
    LoadTexture(g_pDevice, "asset/enemy/enemy_004_wing/enemy_004_wing_right.png", &g_flyEnemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_004_wing/enemy_004_wing_death.png", &g_flyEnemyDeathTexture);

    // 法师敌人
    LoadTexture(g_pDevice, "asset/enemy/enemy_003_fort/enemy_003_fort_idle.png", &g_mageEnemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_003_fort/enemy_003_fort_attack.png", &g_mageEnemyAttackTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_003_fort/enemy_003_fort_death.png", &g_mageEnemyDeathTexture);

    // 快速敌人
    LoadTexture(g_pDevice, "asset/enemy/enemy_002_ant/enemy_002_ant_right.png", &g_fastEnemyRunTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_002_ant/enemy_002_ant_death.png", &g_fastEnemyDeathTexture);

    // 炸弹敌人
    LoadTexture(g_pDevice, "asset/enemy/enemy_005_thorn/enemy_005_thorn_idle.png", &g_bombEnemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_005_thorn/enemy_005_thorn_death.png", &g_bombEnemyDeathTexture);

    // Square enemy
    LoadTexture(g_pDevice, "asset/enemy/enemy_006_square/enemy_006_square.png", &g_squareEnemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_006_square/enemy_006_square_death.png", &g_squareEnemyDeathTexture);

    // Beam enemy
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_idle.png", &g_beamEnemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_attack_before.png", &g_beamEnemyPreAttackTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_attack.png", &g_beamEnemyAttackTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_attack_after.png", &g_beamEnemyPostAttackTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_death_attack_before.png", &g_beamEnemyPreDeathTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_death_attack.png", &g_beamEnemyDeathTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_death_attack_after.png", &g_beamEnemyPostDeathTexture);
}

// ========== BlindEyeEnemy ==========
BlindEyeEnemy::BlindEyeEnemy(float x, float y)
    : Enemy(x, y, 100.0f) {
    // 盲眼敌人不追人，巡逻逻辑自己处理转向，不需要朝向冷却
    useTurnCooldown = false;
    detectionRange = 0.0f;
    loseSightRange = 0.0f;

    anim.ClearClips();
    // idle/普通：使用同一张图（单帧/单格）
    anim.AddClip("idle", 0, 0, 1, 1, 0.1f, true, g_blindEyeEnemyIdleTexture);
    // 死亡动画：复用普通敌人死亡
    anim.AddClip("death", 0, 4, 1, 5, 0.06f, false, g_enemyDeathTexture);
    anim.SetClip("idle");

    // 轻微慢一点，符合“普通”巡逻敌人
    moveSpeed = MOVE_SPEED * 0.55f;
    patrolDirection = 1.0f;
}

void BlindEyeEnemy::Update(float deltaTime, MapManager* mapManager) {
    Enemy::Update(deltaTime, mapManager);
}

void BlindEyeEnemy::ChaseBehavior(float deltaTime) {
    // 不追逐
    currentState = PATROL;
}

bool BlindEyeEnemy::IsGroundAhead(MapManager* mapManager, float directionSign) const {
    if (!mapManager || !mapManager->GetCurrentMap()) {
        return true;
    }

    // 在脚下前方做一个小探测点，判断是否还有地面
    const float aheadX = posX + (directionSign > 0.0f ? width : 0.0f) + directionSign * 0.02f;
    const float probeY = posY - 0.02f;
    const float probeW = 0.02f;
    const float probeH = 0.02f;

    SpatialGrid* grid = mapManager->GetCurrentMap()->GetSpatialGrid();
    if (!grid) {
        auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (!tile.tileInfo.isSolid) continue;
            if (CheckCollision(aheadX, probeY, probeW, probeH, tile.posX, tile.posY, tile.width, tile.height)) {
                return true;
            }
        }
        return false;
    }

    std::vector<MapTile*> nearbyTiles;
    grid->GetTilesInArea(aheadX - 0.1f, probeY - 0.1f, probeW + 0.2f, probeH + 0.2f, nearbyTiles);
    for (const auto* tile : nearbyTiles) {
        if (!tile || !tile->tileInfo.isSolid) continue;
        if (CheckCollision(aheadX, probeY, probeW, probeH, tile->posX, tile->posY, tile->width, tile->height)) {
            return true;
        }
    }
    return false;
}

void BlindEyeEnemy::PatrolBehavior(float deltaTime) {
    if (!isAlive || isDying) return;

    // 盲眼敌人只巡逻：遇墙或悬崖掉头
    MapManager* mapManager = &g_mapManager;
    const float dir = patrolDirection;

    // 1) 前方是否还有地面（悬崖检测）
    if (!IsGroundAhead(mapManager, dir)) {
        patrolDirection = -patrolDirection;
    }
    else {
        // 2) 墙壁检测：在预测位置放一个小偏移，若会撞墙则掉头
        float nextX = posX + dir * moveSpeed * 0.5f * deltaTime * 60.0f;
        if (CheckCollisionWithTilesAt(nextX, posY, mapManager)) {
            patrolDirection = -patrolDirection;
        }
    }

    velocityX = patrolDirection * moveSpeed * 1.0f;
    facingRight = (velocityX > 0.0f);
}

// ========== ThrowerEnemy ==========
ThrowerEnemy::ThrowerEnemy(float x, float y)
    : Enemy(x, y, 120.0f) {
    // Reuse mage/"projectile" enemy texture as requested.
    anim.ClearClips();
    anim.AddClip("idle", 0, 1, 1, 1, 0.1f, true, g_mageEnemyIdleTexture);
    anim.AddClip("death", 0, 4, 1, 5, 0.06f, false, g_mageEnemyDeathTexture);
    anim.SetClip("idle");

    moveSpeed = MOVE_SPEED * 0.4f;
    detectionRange = 10.0f;
    loseSightRange = 12.0f;
    throwCooldown = 2.5f;
    currentThrowCooldown = 0.75f;
    throwRange = 7.0f;
    // Larger flight time => slower projectile speed while keeping the same ballistic arc formula
    throwFlyTime = 0.9f;
}

bool ThrowerEnemy::CanThrow() const {
    return currentThrowCooldown <= 0.0f;
}

void ThrowerEnemy::TryThrow(MapManager* mapManager) {
    if (!mapManager || !mapManager->GetCurrentMap()) return;
    if (!CanThrow()) return;

    float enemyCenterX = posX + width * 0.5f;
    float enemyCenterY = posY + height * 0.5f;
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dx = playerCenterX - enemyCenterX;
    float dy = playerCenterY - enemyCenterY;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist > throwRange) return;

    // Spawn a base enemy at thrower's position and launch it.
    Enemy* thrown = new Enemy(posX, posY, 100.0f);
    g_enemies.push_back(thrown);

    float T = std::max(0.25f, throwFlyTime);
    float g = GRAVITY;

    // Convert to game units per second such that Update (vel * dt * 60)
    // => one-second displacement is vel * 60.
    float vx = dx / (T * 60.0f);
    float vy = (dy - 0.5f * g * (T * 60.0f) * (T * 60.0f)) / (T * 60.0f);

    g_thrownEnemies[thrown] = ThrownEnemyState{ thrown, false, vx, vy };

    facingRight = (dx >= 0.0f);
    currentThrowCooldown = throwCooldown;
}

void ThrowerEnemy::PatrolBehavior(float deltaTime) {
    patrolTimer += deltaTime;
    if (patrolTimer > 2.0f) {
        patrolTimer = 0.0f;
        patrolDirection = -patrolDirection;
    }
    velocityX = patrolDirection * moveSpeed;
    facingRight = (velocityX > 0.0f);
}

void ThrowerEnemy::ChaseBehavior(float deltaTime) {
    float dx = g_player.posX - posX;
    facingRight = (dx >= 0.0f);

    // Keep some distance; do not chase too aggressively.
    if (fabs(dx) > 1.5f) {
        velocityX = (dx > 0 ? 1.0f : -1.0f) * moveSpeed;
    }
    else {
        velocityX = 0.0f;
    }
}

void ThrowerEnemy::Update(float deltaTime, MapManager* mapManager) {
    if (!isAlive) {
        Enemy::Update(deltaTime, mapManager);
        return;
    }

    currentThrowCooldown -= deltaTime;

    // Update thrown enemies first so they can land this frame.
    UpdateThrownEnemies(deltaTime, mapManager);

    Enemy::Update(deltaTime, mapManager);

    // Throw after movement logic so aiming uses current position.
    if (currentState == CHASE) {
        TryThrow(mapManager);
    }
}

// Enemy类实现
Enemy::Enemy(float x, float y, float hp)
    : posX(x), posY(y), health(hp), maxHealth(hp), isAlive(true),
    currentState(PATROL), patrolMinX(-1.0f), patrolMaxX(1.0f), weakSpotDeath(false), attackRange(0.0f) {

    // 普通敌人的碰撞盒不要比贴图大一圈：缩小并保持中心点不变
    const float oldWidth = PLAYER_WIDTH * 1.2f;
    const float oldHeight = PLAYER_HEIGHT * 1.2f;
    width = PLAYER_WIDTH * 1.0f;
    height = PLAYER_HEIGHT * 1.0f;
    posX += (oldWidth - width) * 0.5f;
    posY += (oldHeight - height) * 0.5f;
    moveSpeed = MOVE_SPEED * 0.65f;

    // 为基类敌人添加默认动画剪辑
    anim.AddClip("idle", 0, 1, 1, 1, 0.1f, true, g_enemyIdleTexture);
    anim.AddClip("death", 0, 4, 1, 5, 0.06f, false, g_enemyDeathTexture);

    anim.SetClip("idle");

    facingRight = true;  // 默认面向右边
    velocityX = 0.0f;
    velocityY = 0.0f;

    // 设置基础的8方向伤害倍率
    SetDamageMultiplier(DIR_FRONT, 1.0f);
    SetDamageMultiplier(DIR_FRONT_UP, 1.0f);
    SetDamageMultiplier(DIR_UP, 1.0f);
    SetDamageMultiplier(DIR_BACK_UP, 1.0f);
    SetDamageMultiplier(DIR_BACK, 1.0f);
    SetDamageMultiplier(DIR_BACK_DOWN, 1.0f);
    SetDamageMultiplier(DIR_DOWN, 1.0f);
    SetDamageMultiplier(DIR_FRONT_DOWN, 1.0f);

    // 初始化AI参数
    patrolDirection = 1.0f;
    patrolTimer = 0.0f;
    detectionRange = 3.0f;  // 检测范围
    loseSightRange = 8.0f;  // 丢失视野范围

    // base/normal enemy uses turn cooldown by default
    useTurnCooldown = true;
}

void Enemy::SetDamageMultiplier(Direction dir, float multiplier) {
    if (dir >= DIR_FRONT && dir <= DIR_FRONT_DOWN) {
        damageMultipliers[static_cast<int>(dir)] = multiplier;
    }
}

float Enemy::GetDamageMultiplier(float attackAngle) {
    float relativeAngle = GetRelativeAngle(attackAngle);
    int directionIndex = AngleToDirectionIndex(relativeAngle);
    return damageMultipliers[directionIndex];
}

// 计算相对角度（基于敌人面向方向）
float Enemy::GetRelativeAngle(float attackAngle) const {
    // 面向右时，0度为正面；面向左时，180度为正面
    float enemyFrontAngle = facingRight ? 0.0f : 3.14159f;
    float relativeAngle = attackAngle - enemyFrontAngle;

    // 标准化到[-π, π]
    while (relativeAngle > 3.14159f) relativeAngle -= 2 * 3.14159f;
    while (relativeAngle < -3.14159f) relativeAngle += 2 * 3.14159f;

    return relativeAngle;
}

// 转换角度到方向索引（8方向）
int Enemy::AngleToDirectionIndex(float relativeAngle) {
    // 标准化相对角度到[0, 2π]
    float angle = relativeAngle;
    if (angle < 0) angle += 2 * 3.14159f;

    // 8个方向，每个45度
    float sector = 3.14159f / 4.0f;

    // 计算方向索引
    int index = static_cast<int>((angle + sector / 2) / sector) % 8;
    return index;
}

float Enemy::NormalizeAngle(float angle) {
    while (angle < 0) angle += 2 * 3.14159f;
    while (angle >= 2 * 3.14159f) angle -= 2 * 3.14159f;
    return angle;
}

// 根据攻击角度计算伤害
int Enemy::CalculateDamageFromPlayer(int baseDamage, float playerDashAngle) {
    float multiplier = GetDamageMultiplier(playerDashAngle);
    return (int)(baseDamage * multiplier);
}

// 在TakeDamage方法中使用DamageNumberManager
void Enemy::TakeDamage(int damage, float attackAngle) {
    if (!isAlive) return;

    weakSpotDeath = false;

    float multiplier = GetDamageMultiplier(attackAngle);
    int actualDamage = (int)(damage * multiplier);

    // 使用独立的伤害数字管理器
    bool isCritical = (multiplier > 1.5f);
    DamageNumberManager::AddDamageNumber(
        posX + width * 0.5f,  // 敌人中心X
        posY + height,        // 敌人顶部
        actualDamage,
        isCritical
    );

    if (isCritical) {
        SpawnWeakPointHitEffect(posX + width * 0.5f, posY + height * 0.5f);
    }

    health -= actualDamage;
	Audio::PlaySE(SoundEffect::ENEMY_HIT);
    isHit = true;
    hitTimer = HIT_DURATION;
    OnHit(actualDamage);

    if (health <= 0) {
        health = 0;

        /*if (multiplier >= 2.0f) {
            weakSpotDeath = true;
        }*/
        if (multiplier > 1.5f) {  // CHANGED FROM >= 2.0f to > 1.5f
            weakSpotDeath = true;
        }
        OnDeath();
    }

}

void Enemy::OnHit(int damage) {
    // 基础敌人被击中时没有特殊行为
}

void Enemy::OnDeath() {
    if (isDying) return;  // 避免重复触发

    isAlive = false;
    isDying = true;
    // 确保切换到死亡动画
    anim.SetClip("death");

    // 重置动画到第一帧
    anim.Reset();
	Audio::PlaySE(SoundEffect::ENEMY_DEATH);

    // increments the player combo when enemy dies
    //g_player.comboCount++;
    //g_player.comboTimer = 0.0f; // it resets the timer

    //OnEnemyDefeated();
    OnEnemyDefeated(weakSpotDeath);
}

void Enemy::Update(float deltaTime, MapManager* mapManager) {
    // 优先处理死亡状态
    if (isDying) {
        anim.Update(deltaTime);  // 确保死亡动画得到更新

        // 检查动画是否播放完毕
        if (anim.IsFinished()) {
            markedForDeletion = true;
        }
        return;  // 死亡动画期间不执行其他逻辑
    }

    if (!isAlive) {
        // 如果已经死亡但还没开始死亡动画，则开始死亡动画
        OnDeath();
        return;
    }

    // 可见性检测和优化逻辑
    bool isCurrentlyVisible = IsVisible(g_camera);

    if (!isCurrentlyVisible && !NeedsMinimalUpdate()) {
        offScreenTimer += deltaTime;
        if (offScreenTimer > MAX_OFFSCREEN_TIME &&
            currentState == PATROL &&
            !isHit &&
            health >= maxHealth) {
            return;
        }
    }

    if (isCurrentlyVisible && !wasVisible) {
        ResetOffScreenTimer();
    }
    wasVisible = isCurrentlyVisible;

    if (!isCurrentlyVisible && NeedsMinimalUpdate()) {
        UpdateMinimal(deltaTime);
        return;
    }

    anim.Update(deltaTime);

    // 受击状态处理
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    // 应用重力
    velocityY += GRAVITY * deltaTime * 60.0f;

    // 保存旧位置用于碰撞检测
    float oldX = posX;
    float oldY = posY;

    // 水平移动
    posX += velocityX * deltaTime * 60.0f;
    if (CheckHorizontalCollision(mapManager, oldX, oldY)) {
        posX = oldX;
        velocityX = 0.0f;
    }

    // 垂直移动
    posY += velocityY * deltaTime * 60.0f;
    if (CheckVerticalCollision(mapManager, oldX, oldY)) {
        posY = oldY;
        velocityY = 0.0f;
    }

    // 边界检查
    if (posY < -5.0f) {
        isAlive = false;
        OnDeath();  // 触发死亡动画
        return;
    }

    // AI更新
    UpdateAI(deltaTime);

    if (!isCurrentlyVisible) {
        offScreenTimer += deltaTime;
    }
    else {
        offScreenTimer = 0.0f;
    }
}

// 简化更新（对需要更新的屏幕外敌人）
void Enemy::UpdateMinimal(float deltaTime) {
    // 简化的AI更新（只处理状态转换，不进行路径计算等）
    UpdateAIMinimal(deltaTime);

    // 更新离屏计时器
    offScreenTimer += deltaTime;
}

// 简化的AI更新
void Enemy::UpdateAIMinimal(float deltaTime) {
    // 只处理基本状态维护，不进行复杂计算
    float dx = g_player.posX - posX;

    // 更新面向方向
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // 简化状态机：只处理超时或关键状态转换
    static float stateTimer = 0.0f;
    stateTimer += deltaTime;

    // 每5秒检查一次状态转换（减少频率）
    if (stateTimer >= 5.0f) {
        float distance = fabs(dx);

        // 简化状态转换逻辑
        switch (currentState) {
        case PATROL:
            if (distance < detectionRange) currentState = CHASE;
            break;
        case CHASE:
            if (distance > loseSightRange) currentState = PATROL;
            break;
        }

        stateTimer = 0.0f;
    }
}

void Enemy::UpdateAI(float deltaTime) {
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    if (useTurnCooldown && turnCooldownTimer > 0.0f) {
        turnCooldownTimer -= deltaTime;
        if (turnCooldownTimer < 0.0f) turnCooldownTimer = 0.0f;
    }

    // 更新面向方向（普通敌人受冷却限制；其他敌人保持原本“立即转向”）
    if (dx != 0) {
        bool desiredFacingRight = (dx > 0);
        if (useTurnCooldown) {
            if (desiredFacingRight != facingRight && turnCooldownTimer <= 0.0f) {
                facingRight = desiredFacingRight;
                turnCooldownTimer = TURN_COOLDOWN_SECONDS;
            }
        }
        else {
            facingRight = desiredFacingRight;
        }
    }

    // 状态机逻辑
    switch (currentState) {
    case PATROL:
        PatrolBehavior(deltaTime);
        if (distance < detectionRange) {
            currentState = CHASE;
        }
        break;
    case CHASE:
        ChaseBehavior(deltaTime);
        if (distance > loseSightRange) {
            currentState = PATROL;
        }
        break;
    }
}

void Enemy::PatrolBehavior(float deltaTime) {
    patrolTimer += deltaTime;

    // 每2秒检查是否需要改变方向
    if (patrolTimer >= 2.0f) {
        if (posX <= patrolMinX) {
            patrolDirection = 1.0f;  // 向右走
        }
        else if (posX >= patrolMaxX) {
            patrolDirection = -1.0f;  // 向左走
        }
        patrolTimer = 0.0f;
    }

    velocityX = patrolDirection * moveSpeed * 0.5f;

    // 添加小的垂直速度变化避免完全静止
    if (velocityY == 0) {
        velocityY = 0.01f;
    }
}

void Enemy::ChaseBehavior(float deltaTime) {
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;  // 添加垂直方向计算
    float distance = sqrt(dx * dx + dy * dy);  // 使用实际距离

    // 只有远程敌人才在攻击范围内停止移动
    if (attackRange > 0 && distance <= attackRange) {
        // 远程敌人在攻击范围内停止移动
        velocityX = 0;
    }
    else {
        // 普通敌人：一直按自己当前面向方向移动（不再根据玩家位置改变移动方向）
        velocityX = (facingRight ? 1.0f : -1.0f) * moveSpeed;
    }
}

void Enemy::WorldToScreenPosition(float worldX, float worldY, float& screenX, float& screenY, const Camera& camera) {
    // 获取相机位置（相机中心坐标）
    float cameraX = camera.GetX();
    float cameraY = camera.GetY();

    // 将世界坐标转换为屏幕坐标（相对坐标）
    // 假设渲染系统使用屏幕中心作为原点(0,0)
    screenX = worldX - cameraX;
    screenY = worldY - cameraY;
}

void Enemy::Render(ID3D11ShaderResourceView* texture, const Camera& camera) {
    if (!isAlive && !isDying) return;

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // 将世界坐标转换为屏幕坐标
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    // 获取UV偏移用于精灵表动画
    DirectX::XMFLOAT2 uvOffset = anim.GetUVOffset();

    // Apply scale to sprite size
    float renderWidth = width * scale;
    float renderHeight = height * scale;
    // Center the bigger sprite on collision box
    float offsetX = (renderWidth - width) * 0.5f;
    float offsetY = (renderHeight - height) * 0.5f;

    // 渲染敌人精灵
    RenderImage(
        screenX - offsetX,
        screenY - offsetY,
        renderWidth,
        renderHeight,
        anim.GetCurrentClipTexture(),
        anim.GetCurrentFrame(),
        anim.GetSplitX(),  // 替换为动画的X分割数
        anim.GetSplitY(),  // 替换为动画的Y分割数
        false,             // enableCulling
        0.0f,              // rotation
        !facingRight       // flipHorizontal: 注意这里可能应该是!facingRight，根据您的坐标系决定
    );

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // 如果不是死亡状态，渲染血条
    if (!isDying) {
        RenderHealthBar(camera);
    }
}

void Enemy::RenderHealthBar(const Camera& camera) {
    // 将世界坐标转换为屏幕坐标
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    float barWidth = width;
    float barHeight = 0.02f;
    float barX = screenX;
    float barY = screenY + height + 0.02f;

    // 背景条（红色）
    RenderImage(barX, barY, barWidth, barHeight, g_groundTexture, 0, 1, 1);

    // 血条（绿色）
    float healthRatio = health / maxHealth;
    RenderImage(barX, barY, barWidth * healthRatio, barHeight, g_groundTexture, 1, 1, 1);
}

bool Enemy::CheckPlayerCollision() {
    return CheckCollision(posX, posY, width, height,
        g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT);
}

// 检查与特定区域的碰撞
bool Enemy::CheckCollisionWithTilesAt(float checkX, float checkY, MapManager* mapManager) {
    if (!mapManager || !mapManager->GetCurrentMap()) {
        return false;
    }

    SpatialGrid* grid = mapManager->GetCurrentMap()->GetSpatialGrid();
    if (!grid) {
        // 回退到原始方法
        auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (CheckCollision(checkX, checkY, GetWidth(), GetHeight(),
                tile.posX, tile.posY, tile.width, tile.height)) {
                return true;
            }
        }
        return false;
    }

    // 使用空间网格优化
    std::vector<MapTile*> nearbyTiles;
    grid->GetTilesInArea(
        checkX - 0.5f,
        checkY - 0.5f,
        GetWidth() + 1.0f,
        GetHeight() + 1.0f,
        nearbyTiles
    );

    for (const auto& tile : nearbyTiles) {
        if (tile->tileInfo.isSolid &&
            CheckCollision(checkX, checkY, GetWidth(), GetHeight(),
                tile->posX, tile->posY, tile->width, tile->height)) {
            return true;
        }
    }

    return false;
}

// 更新碰撞检测，使用空间网格优化
bool Enemy::CheckCollisionWithTiles(MapManager* mapManager) {
    if (!mapManager || !mapManager->GetCurrentMap()) {
        return false;
    }

    // 缓存空间网格指针
    SpatialGrid* grid = mapManager->GetCurrentMap()->GetSpatialGrid();
    // 使用空间网格优化
    std::vector<MapTile*> nearbyTiles;
    float padding = 0.5f;  // 稍微扩展检测范围
    grid->GetTilesInArea(
        posX - padding,
        posY - padding,
        width + padding * 2,
        height + padding * 2,
        nearbyTiles
    );

    for (const auto& tile : nearbyTiles) {
        if (tile->tileInfo.isSolid && CheckCollisionWithTile(*tile)) {
            return true;
        }
    }

    return false;
}

bool Enemy::CheckCollisionWithTile(const MapTile& tile) {
    return CheckCollision(posX, posY, width, height,
        tile.posX, tile.posY, tile.width, tile.height);
}

// FlyEnemy实现 - 飞行敌人，不受重力影响
FlyEnemy::FlyEnemy(float x, float y) : Enemy(x, y, 150.0f) {
    useTurnCooldown = false;
    // 飞行敌人：空中单位
	targetAltitude = y;
    attackRange = 0.0f;  // 近战敌人
    SetDamageMultiplier(DIR_FRONT, 0.8f);
    SetDamageMultiplier(DIR_FRONT_UP, 0.8f);
    SetDamageMultiplier(DIR_FRONT_DOWN, 0.8f);
    SetDamageMultiplier(DIR_BACK, 1.5f);
    SetDamageMultiplier(DIR_BACK_UP, 1.5f);
    SetDamageMultiplier(DIR_BACK_DOWN, 1.5f);
    SetDamageMultiplier(DIR_UP, 1.2f);
    SetDamageMultiplier(DIR_DOWN, 1.2f);

    // 添加动画剪辑
    anim.AddClip("idle", 0, 3, 1, 4, 0.15f, true, g_flyEnemyIdleTexture);
    anim.AddClip("death", 0, 3, 1, 4, 0.06f, false, g_flyEnemyDeathTexture);

    anim.SetClip("idle");
    width = PLAYER_WIDTH * 1.5f;
    moveSpeed = MOVE_SPEED * 0.8f;

    // 飞行敌人特定参数
    patrolMinX = x - 2.0f;  // 扩大巡逻范围
    patrolMaxX = x + 2.0f;
    detectionRange = 5.0f;  // 更远的探测距离
    patrolAltitude = y;  // 巡逻高度
    altitudeChangeTimer = 0.0f;
    altitudeChangeRate = 0.05f;  // 高度变化速度

    scale = 3.0f;
}


void FlyEnemy::PatrolBehavior(float deltaTime) {
    patrolTimer += deltaTime;
    altitudeChangeTimer += deltaTime;

    // 每2秒检查是否需要改变水平方向
    if (patrolTimer >= 2.0f) {
        if (posX <= patrolMinX) {
            patrolDirection = 1.0f;  // 向右走
        }
        else if (posX >= patrolMaxX) {
            patrolDirection = -1.0f;  // 向左走
        }
        patrolTimer = 0.0f;
    }

    // 垂直漂浮效果
    float altitudeVariation = sin(altitudeChangeTimer * 2.0f) * 0.1f;
    targetAltitude = patrolAltitude + altitudeVariation;

    // 平滑移动到目标高度
    if (fabs(posY - targetAltitude) > 0.01f) {
        if (posY < targetAltitude) {
            velocityY = altitudeChangeRate;
        }
        else {
            velocityY = -altitudeChangeRate;
        }
    }
    else {
        velocityY = 0.0f;
    }

    velocityX = patrolDirection * moveSpeed * 0.3f;  // 巡逻时较慢
}


void FlyEnemy::ChaseBehavior(float deltaTime) {
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    // 如果玩家在检测范围内，直接向玩家移动
    if (distance > 0.1f) {
        // 归一化方向向量
        dx /= distance;
        dy /= distance;

        // 飞行敌人直接向玩家移动，无视地形
        velocityX = dx * moveSpeed;
        velocityY = dy * moveSpeed;
    }
    else {
        // 接近时稍微减速
        velocityX *= 0.5f;
        velocityY *= 0.5f;
    }
}

void FlyEnemy::OnHit(int damage) {
    // 飞行敌人被击中时会有短暂硬直
    velocityX *= 0.5f;
    velocityY = 0.0f;
}

void FlyEnemy::OnDeath() {
    Enemy::OnDeath();
    // 飞行敌人死亡时可能会有特殊效果
}

// MageEnemy实现
MageEnemy::MageEnemy(float x, float y) : Enemy(x, y, 80.0f) {
    useTurnCooldown = false;
    // 法师敌人：从顶部和底部易受伤害
    SetDamageMultiplier(DIR_UP, 2.0f);
    SetDamageMultiplier(DIR_DOWN, 2.0f);
    SetDamageMultiplier(DIR_FRONT, 0.7f);
    SetDamageMultiplier(DIR_BACK, 0.7f);

    spellCooldown = 3.0f;
    currentSpellCooldown = 0.0f;
    detectionRange = 4.0f;  // 更远的探测距离
    attackRange = 2.5f;  // 射弹攻击范围
    moveSpeed = MOVE_SPEED * 0.4f;

    // 添加动画剪辑
    anim.AddClip("idle", 0, 1, 1, 2, 0.2f, true, g_mageEnemyIdleTexture);
    anim.AddClip("death", 0, 3, 1, 4, 0.06f, false, g_mageEnemyDeathTexture); // for when I kill the enemy
    anim.SetClip("idle");


    // 射弹相关参数
    projectileSpeed = 2.0f;
    lastAttackTime = 0.0f;
    attackCooldown = 1.5f;  // 攻击冷却时间
    projectileDamage = 20.0f;

    scale = 3.0f;
}

void MageEnemy::Update(float deltaTime, MapManager* mapManager) {
    // 死亡动画期间不应再产生新射弹
    if (isDying) {
        anim.Update(deltaTime);
        if (anim.IsFinished()) {
            markedForDeletion = true;
        }
        return;
    }

    Enemy::Update(deltaTime, mapManager);

    // Enemy::Update 可能在死亡/离屏最小更新等情况下提前返回，
    // 这里再兜底一次，确保不会在这些状态下继续发射射弹。
    if (!isAlive) {
        return;
    }

    lastAttackTime += deltaTime;

    // 在追逐状态下发射射弹
    if (currentState == CHASE && lastAttackTime >= attackCooldown) {
        CastProjectile();
        lastAttackTime = 0.0f;
    }
}

void MageEnemy::ChaseBehavior(float deltaTime) {
    float dx = g_player.posX - posX;
    float distance = fabs(dx);

    // 法师在追逐时保持距离
    if (distance > attackRange) {
        if (dx > 0) {
            velocityX = moveSpeed * 0.5f;  // 移动较慢
        }
        else {
            velocityX = -moveSpeed * 0.5f;
        }
    }
    else {
        velocityX = 0;  // 在攻击距离内停止移动
    }
}

void MageEnemy::CastProjectile() {
    //如果死亡
    if (!isAlive) return;

    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance > 0.1f) {
        // 归一化方向向量
        dx /= distance;
        dy /= distance;

        // 获取ProjectileManager实例
        ProjectileManager& projectileManager = ProjectileManager::GetInstance();

        // 创建魔法射弹配置
        ProjectileEffect magicEffect;
        magicEffect.damage = projectileDamage;
        magicEffect.areaRadius = 0.2f;
        magicEffect.pierce = false;

        // 计算射弹目标位置
        float targetX = g_player.posX;
        float targetY = g_player.posY;

        // 发射魔法射弹
        projectileManager.CreateFireball(
            posX + width * 0.5f,  // 从中心发射
            posY + height * 0.4f,  // 从敌人高度40%处发射（原来是0.7f，降低了30%）
            targetX,
            targetY,
            false
        );

        // 播放攻击动画
        //PlayAnimation("attack");
    }
}

// FastEnemy实现
FastEnemy::FastEnemy(float x, float y) : Enemy(x, y, 60.0f) {
    useTurnCooldown = false;
    moveSpeed = MOVE_SPEED * 1.5f;
    dashCooldown = 2.0f;
    currentDashCooldown = 0.0f;
    detectionRange = 4.0f;
    attackRange = 0.5f;

    attackRange = 0.0f;  // 近战敌人
    anim.AddClip("run", 0, 3, 1, 4, 0.05f, true, g_fastEnemyRunTexture);
    anim.AddClip("death", 0, 3, 1, 4, 0.06f, false, g_fastEnemyDeathTexture);
    anim.SetClip("run");

    scale = 3.0f;
}


// 添加这个函数实现
void FastEnemy::Update(float deltaTime, MapManager* mapManager) {
    Enemy::Update(deltaTime, mapManager);

    if (currentDashCooldown > 0) {
        currentDashCooldown -= deltaTime;
    }

    if (currentState == CHASE && currentDashCooldown <= 0) {
        DashAttack();
        currentDashCooldown = dashCooldown;
    }
}

void FlyEnemy::Update(float deltaTime, MapManager* mapManager) {  
    if (isDying) {
        anim.Update(deltaTime);  // 确保死亡动画得到更新

        // 检查动画是否播放完毕
        if (anim.IsFinished()) {
            markedForDeletion = true;
        }
        return;  // 死亡动画期间不执行其他逻辑
    }

    if (!isAlive) {
        // 如果已经死亡但还没开始死亡动画，则开始死亡动画
        OnDeath();
        return;
    }

    // 可见性检测和优化逻辑
    bool isCurrentlyVisible = IsVisible(g_camera);

    if (!isCurrentlyVisible && !NeedsMinimalUpdate()) {
        offScreenTimer += deltaTime;
        if (offScreenTimer > MAX_OFFSCREEN_TIME &&
            currentState == PATROL &&
            !isHit &&
            health >= maxHealth) {
            return;
        }
    }

    if (isCurrentlyVisible && !wasVisible) {
        ResetOffScreenTimer();
    }
    wasVisible = isCurrentlyVisible;

    if (!isCurrentlyVisible && NeedsMinimalUpdate()) {
        UpdateMinimal(deltaTime);
        return;
    }

    anim.Update(deltaTime);

    // 受击状态处理
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    // AI更新
    UpdateAI(deltaTime);

    // 应用水平移动
    posX += velocityX * deltaTime * 60.0f;

    // 飞行敌人的垂直移动（漂浮效果）
    posY += velocityY * deltaTime * 60.0f;

    // 边界检查（防止飞出世界）
    if (posY < -50.0f) {
        isAlive = false;
        return;
    }

    if (!isCurrentlyVisible) {
        offScreenTimer += deltaTime;
    }
    else {
        offScreenTimer = 0.0f;
    }
}

void FastEnemy::ChaseBehavior(float deltaTime) {
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);  // 使用实际距离

    // 快速敌人一直向玩家移动
    if (dx > 0) {
        velocityX = moveSpeed;
    }
    else {
        velocityX = -moveSpeed;
    }

    // 冲刺攻击
    if (currentDashCooldown <= 0 && distance < 2.0f) {  // 冲刺距离阈值
        DashAttack();
        currentDashCooldown = dashCooldown;
    }
}

void FastEnemy::DashAttack() {
    // 快速敌人向前冲刺
    velocityX = (g_player.posX > posX ? 1.0f : -1.0f) * moveSpeed * 2.5f;
}

// BombEnemy实现
BombEnemy::BombEnemy(float x, float y) : Enemy(x, y, 120.0f) {
    useTurnCooldown = false;
    // 炸弹敌人：顶部和底部10倍伤害，其他方向减少伤害
    SetDamageMultiplier(DIR_UP, 10.0f);
    SetDamageMultiplier(DIR_DOWN, 10.0f);
    SetDamageMultiplier(DIR_FRONT, 0.7f);
    SetDamageMultiplier(DIR_BACK, 0.7f);
    SetDamageMultiplier(DIR_FRONT_UP, 1.2f);
    SetDamageMultiplier(DIR_FRONT_DOWN, 1.2f);
    SetDamageMultiplier(DIR_BACK_UP, 1.2f);
    SetDamageMultiplier(DIR_BACK_DOWN, 1.2f);

    width = PLAYER_WIDTH * 1.3f;
    height = PLAYER_HEIGHT * 1.3f;
    moveSpeed = 0.0f;  // 不移动
    detectionRange = 2.0f;

    anim.AddClip("idle", 0, 0, 1, 1, 0.3f, true, g_bombEnemyIdleTexture);
    anim.AddClip("death", 0, 3, 1, 4, 0.06f, false, g_bombEnemyDeathTexture);

    anim.SetClip("idle");

    pulseTimer = 0.0f;
    baseSize = 1.0f;
    explosionRadius = 1.5f;
    explosionDamage = 50.0f;

    scale = 3.0f;
}

// 覆盖TakeDamage函数，添加爆炸检测
void BombEnemy::TakeDamage(int damage, float attackAngle) {
    if (!isAlive) return;

    // 获取伤害倍率
    float multiplier = GetDamageMultiplier(attackAngle);
    int actualDamage = (int)(damage * multiplier);

    // 使用独立的伤害数字管理器
    bool isCritical = (multiplier > 1.5f);
    DamageNumberManager::AddDamageNumber(
        posX + width * 0.5f,  // 敌人中心X
        posY + height,        // 敌人顶部
        actualDamage,
        multiplier >= 10.0f  // 如果从顶部/底部方向，显示为暴击
    );

    // 检查是否从顶部或底部攻击
    float relativeAngle = GetRelativeAngle(attackAngle);
    int directionIndex = AngleToDirectionIndex(relativeAngle);
    bool isVerticalAttack = (directionIndex == DIR_UP || directionIndex == DIR_DOWN);

    // 如果从顶部或底部攻击，立即死亡并触发爆炸
    if (multiplier >= 10.0f) {
        health = 0;  // 立即死亡
        isAlive = false;
        OnDeath();  // 触发爆炸
        return;     // 直接返回，跳过后续逻辑
    }

    // 非垂直攻击，正常处理伤害
    health -= actualDamage;
    isHit = true;
    hitTimer = HIT_DURATION;
    OnHit(actualDamage);

    if (health <= 0) {
        health = 0;
        isAlive = false;
        OnDeath();
    }
}

void BombEnemy::Update(float deltaTime, MapManager* mapManager) {    // 优先处理死亡状态
    if (isDying) {
        anim.Update(deltaTime);  // 确保死亡动画得到更新

        // 检查动画是否播放完毕
        if (anim.IsFinished()) {
            markedForDeletion = true;
        }
        return;  // 死亡动画期间不执行其他逻辑
    }

    if (!isAlive) {
        // 如果已经死亡但还没开始死亡动画，则开始死亡动画
        OnDeath();
        return;
    }
    // 调用基类的受击状态更新
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    // 脉动效果
    pulseTimer += deltaTime;
    float pulseEffect = sin(pulseTimer * 3.0f) * 0.1f;
    baseSize = 1.0f + pulseEffect;

    // 简单AI：只检测玩家距离
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    // 更新面向方向
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // 简单状态机
    if (distance < detectionRange) {
        currentState = CHASE;  // 当玩家接近时进入追逐状态
    }
    else {
        currentState = PATROL;  // 否则保持巡逻状态（静止）
    }

    // 死亡时触发爆炸
    if (health <= 0 && isAlive) {
        isAlive = false;
        OnDeath();
    }
}

void BombEnemy::ChaseBehavior(float deltaTime) {
    // 在追逐状态下，如果玩家在爆炸范围内，就自爆
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < explosionRadius) {
        OnDeath();  // 触发爆炸
    }
}

void BombEnemy::OnDeath() {
    // 先调用基类的OnDeath
    Explode();
    Enemy::OnDeath();
}

void BombEnemy::Explode() {
    // 播放爆炸音效
    // PlaySound("explosion.wav");

    // 创建爆炸效果
    // CreateExplosionEffect(posX, posY);

    // 创建射弹向左右发射
    CreateProjectiles();
}

//void BombEnemy::CreateProjectiles() {
//    // 获取ProjectileManager实例
//    ProjectileManager& projectileManager = ProjectileManager::GetInstance();
//
//    // 创建火球效果配置
//    ProjectileEffect fireballEffect;
//    fireballEffect.damage = 30.0f;  // 基础伤害
//    fireballEffect.burnDamage = 5.0f;  // 燃烧伤害
//    fireballEffect.areaRadius = 0.3f;  // 爆炸半径
//    fireballEffect.pierce = false;  // 不穿透
//
//    float projectileSpeed = 3.0f;  // 射弹速度
//
//    // 向左发射火球
//    projectileManager.CreateFireball(
//        posX,  // 起始X
//        posY + height * 0.5f,  // 从敌人中心高度发射
//        posX - 10.0f,  // 左侧远处位置
//        posY + height * 0.5f,  // 水平方向
//        true  // 来自玩家
//    );
//
//    // 向右发射火球
//    projectileManager.CreateFireball(
//        posX,  // 起始X
//        posY + height * 0.5f,  // 从敌人中心高度发射
//        posX + 10.0f,  // 右侧远处位置
//        posY + height * 0.5f,  // 水平方向
//        true  // 来自玩家
//    );
//
//    // 可以在这里添加粒子效果
//    // CreateParticleEffect(posX, posY, "explosion");
//}
// so the enemy throw projectiles in 8 directions 
void BombEnemy::CreateProjectiles() {
    // Get ProjectileManager instance
    ProjectileManager& projectileManager = ProjectileManager::GetInstance();

    float distance = 10.0f;  // How far to target

    // Calculate center position of the enemy
    float centerX = posX + width * 0.5f;
    float centerY = posY + height * 0.3f;  // 从敌人高度30%处发射（原来是0.5f，降低了20%）

    // for the 8 directions pattern
    for (int i = 0; i < 8; i++) {
        float targetX = centerX + EIGHT_DIRECTIONS[i].x * distance;
        float targetY = centerY + EIGHT_DIRECTIONS[i].y * distance;

        projectileManager.CreateFireball(
            centerX,
            centerY,
            targetX,
            targetY,
            true  // will not hurt the player
        );
    }
}


BossEnemy::BossEnemy(float x, float y) : Enemy(x, y, 500000.0f)
{
    useTurnCooldown = false;
    // change these variables as you want. this is just for the test of the testboss
    SetMaxHealth(500000.0f); 
    SetHealth(500000.0f);

    width = PLAYER_WIDTH * 3.0f;
    height = PLAYER_HEIGHT * 3.0f;
    scale = 5.0f;  // Even bigger sprite
    moveSpeed = MOVE_SPEED * 0.3f;  // Slower movement


    // Boss has different damage multipliers (harder to damage from front)


    // also add animations, etc....
}

void BossEnemy::Update(float deltaTime, MapManager* mapManager)
{
    // write here the update code regardsing the boss 

    float healthPercent = health / maxHealth;
    if (healthPercent < 0.3f && phase == 1) {
        phase = 2;
        moveSpeed *= 1.5f;  // Faster in phase 2
    }
}
void BossEnemy::ChaseBehavior(float deltaTime) {
    float dx = g_player.posX - posX;
    float distance = fabs(dx);

    // Boss moves toward player
    if (distance > attackRange) {
        if (dx > 0) {
            velocityX = moveSpeed;
        }
        else {
            velocityX = -moveSpeed;
        }
    }
    else {
        velocityX = 0;  // Stop at attack range
    }
}

void BossEnemy::SpecialAttack() {
    // Boss special attack - shoot multiple projectiles, etc.
    // Use ProjectileManager to create attacks
}

void BossEnemy::OnHit(int damage) {
    Enemy::OnHit(damage);
    // Boss hit reaction
}

void BossEnemy::OnDeath() {
    Enemy::OnDeath();
    // Boss death - maybe trigger cutscene or level completion
}


// SquareEnemy implementation - stationary enemy
SquareEnemy::SquareEnemy(float x, float y) : Enemy(x, y, 100.0f) {
    useTurnCooldown = false;
    // Square enemy: takes normal damage from all directions
    SetDamageMultiplier(DIR_FRONT, 1.0f);
    SetDamageMultiplier(DIR_BACK, 1.0f);
    SetDamageMultiplier(DIR_UP, 1.0f);
    SetDamageMultiplier(DIR_DOWN, 1.0f);
    SetDamageMultiplier(DIR_FRONT_UP, 1.0f);
    SetDamageMultiplier(DIR_FRONT_DOWN, 1.0f);
    SetDamageMultiplier(DIR_BACK_UP, 1.0f);
    SetDamageMultiplier(DIR_BACK_DOWN, 1.0f);

    width = PLAYER_WIDTH * 1.0f;
    height = PLAYER_HEIGHT * 1.0f;
    moveSpeed = 0.0f;  // Doesn't move
    detectionRange = 0.0f;  // Doesn't chase
    attackRange = 0.0f;  // Contact damage only

    // Add animations (adjust frame counts based on your sprites)
    anim.AddClip("idle", 0, 7, 1, 8, 0.15f, true, g_squareEnemyIdleTexture);
    anim.AddClip("death", 0, 3, 1, 4, 0.06f, false, g_squareEnemyDeathTexture); // Adjust frames as needed

    anim.SetClip("idle");

    pulseTimer = 0.0f;
    scale = 3.0f;
}

void SquareEnemy::Update(float deltaTime, MapManager* mapManager) {
    // Handle death state first
    if (isDying) {
        anim.Update(deltaTime);

        if (anim.IsFinished()) {
            markedForDeletion = true;
        }
        return;
    }

    if (!isAlive) {
        OnDeath();
        return;
    }

    // Visibility detection
    bool isCurrentlyVisible = IsVisible(g_camera);

    if (!isCurrentlyVisible && !NeedsMinimalUpdate()) {
        offScreenTimer += deltaTime;
        if (offScreenTimer > MAX_OFFSCREEN_TIME &&
            currentState == PATROL &&
            !isHit &&
            health >= maxHealth) {
            return;
        }
    }

    if (isCurrentlyVisible && !wasVisible) {
        ResetOffScreenTimer();
    }
    wasVisible = isCurrentlyVisible;

    if (!isCurrentlyVisible && NeedsMinimalUpdate()) {
        UpdateMinimal(deltaTime);
        return;
    }

    anim.Update(deltaTime);

    // Handle hit state
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    // Square enemy doesnt move
    velocityX = 0.0f;
    velocityY = 0.0f;

    // Pulse effect for visual feedback
    pulseTimer += deltaTime;

    // just stay in place
    currentState = PATROL;

    if (!isCurrentlyVisible) {
        offScreenTimer += deltaTime;
    }
    else {
        offScreenTimer = 0.0f;
    }
}

void SquareEnemy::OnHit(int damage) {
    // Square enemy has no special behavior when hit
    Enemy::OnHit(damage);
}

void SquareEnemy::OnDeath() {
    // Call base death logic
    Enemy::OnDeath();
}


BeamEnemy::BeamEnemy(float x, float y) : Enemy(x, y, 150.0f) {
    // Weak points: Vertical and Horizontal lines (like a cross)
    SetDamageMultiplier(DIR_UP, 8.0f);
    SetDamageMultiplier(DIR_DOWN, 8.0f);
    SetDamageMultiplier(DIR_FRONT, 8.0f);
    SetDamageMultiplier(DIR_BACK, 8.0f);

    width = PLAYER_WIDTH * 1.3f;
    height = PLAYER_HEIGHT * 1.3f;
    moveSpeed = 0.0f;
    detectionRange = 0.6f;  // change this depending on the range you want

    // for the animation
    anim.AddClip("idle", 0, 2, 1, 3, 0.25f, true, g_beamEnemyIdleTexture);
    anim.AddClip("pre_attack", 0, 3, 1, 4, 0.8f, false, g_beamEnemyPreAttackTexture);
    anim.AddClip("attack", 0, 3, 1, 4, 0.1f, false, g_beamEnemyAttackTexture);
    anim.AddClip("post_attack", 0, 2, 1, 3, 0.15f, false, g_beamEnemyPostAttackTexture);
    anim.AddClip("pre_death", 0, 5, 1, 6, 0.15f, false, g_beamEnemyPreDeathTexture);
    anim.AddClip("death", 0, 2, 1, 3, 0.1f, false, g_beamEnemyDeathTexture);
    anim.AddClip("post_death", 0, 2, 1, 3, 0.15f, false, g_beamEnemyPostDeathTexture);

    anim.SetClip("idle");

    scale = 13.2f;
    beamState = BEAM_IDLE;
    currentCooldown = 0.0f;
    stateTimer = 0.0f;
    deathAnimationPhase = 0;
    hasExploded = false;
    hasKilledPlayerThisAttack = false;
    pulseTimer = 0.0f;
}

void BeamEnemy::TakeDamage(int damage, float attackAngle) {
    if (!isAlive) return;

    float multiplier = GetDamageMultiplier(attackAngle);
    int actualDamage = (int)(damage * multiplier);

    // Show damage number
    bool isCritical = (multiplier >= 8.0f);
    DamageNumberManager::AddDamageNumber(
        posX + width * 0.5f,
        posY + height,
        actualDamage,
        isCritical
    );

    // If hit on weak points, instant death
    if (multiplier >= 8.0f) {
        health = 0;
        isAlive = false;
        OnDeath();
        return;
    }

    // Normal damage
    health -= actualDamage;
    isHit = true;
    hitTimer = HIT_DURATION;
    OnHit(actualDamage);

    if (health <= 0) {
        health = 0;
        isAlive = false;
        OnDeath();
    }
}

void BeamEnemy::Update(float deltaTime, MapManager* mapManager) {
    // Handle death animation sequence
    if (isDying) {
        anim.Update(deltaTime);

        // Phase 0: pre_death animation
        if (deathAnimationPhase == 0) {
            if (anim.GetCurrentClipName() != "pre_death") {
                anim.SetClip("pre_death");
            }
            if (anim.IsFinished()) {
                deathAnimationPhase = 1;
                anim.SetClip("death");
            }
        }
        // Phase 1: main death with explosion
        else if (deathAnimationPhase == 1) {
            // Trigger the explosion
            if (!hasExploded && anim.GetCurrentFrame() >= 1) {
                CreateDeathExplosion();
                hasExploded = true;
            }
            if (anim.IsFinished()) {
                deathAnimationPhase = 2;
                anim.SetClip("post_death");
            }
        }
        // Phase 2: post_death animation (beam fading)
        else if (deathAnimationPhase == 2) {
            if (anim.IsFinished()) {
                markedForDeletion = true;
            }
        }

        return;
    }

    if (!isAlive) {
        OnDeath();
        return;
    }

    // Handle hit state
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    velocityX = 0.0f;
    velocityY = 0.0f;
    pulseTimer += deltaTime;

    // Calculate distance to player
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    // Update facing direction
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // Get current frame (declare before switch to avoid scoping issues)
    int currentFrame = anim.GetCurrentFrame();

    // State machine for beam attack cycle
    switch (beamState) {
    case BEAM_IDLE:
        if (anim.GetCurrentClipName() != "idle") {
            anim.SetClip("idle");
        }

        // Cooldown timer
        if (currentCooldown > 0.0f) {
            currentCooldown -= deltaTime;
        }

        // Check if player is in range and cooldown is done
        if (distance < detectionRange && currentCooldown <= 0.0f) {
            beamState = BEAM_PRE_ATTACK;
            stateTimer = 0.0f;
            anim.SetClip("pre_attack");
        }
        break;

    case BEAM_PRE_ATTACK:
        stateTimer += deltaTime;

        hasKilledPlayerThisAttack = false; // Reset kill flag when starting new attack

        // After charging animation completes, start attack
        if (stateTimer >= preAttackDuration || anim.IsFinished()) {
            beamState = BEAM_ATTACKING;
            stateTimer = 0.0f;
            anim.SetClip("attack");
        }
        break;

    case BEAM_ATTACKING:
        stateTimer += deltaTime;

        CheckBeamDamage(); // Check beam damage EVERY frame during attack, not just specific frames

        // After attack duration, start post-attack
        if (stateTimer >= attackDuration) {
            beamState = BEAM_POST_ATTACK;
            stateTimer = 0.0f;
            anim.SetClip("post_attack");
        }
        break;

    case BEAM_POST_ATTACK:
        stateTimer += deltaTime;

        // After post-attack animation, return to idle
        if (stateTimer >= postAttackDuration || anim.IsFinished()) {
            beamState = BEAM_IDLE;
            currentCooldown = attackCooldown;  // Reset cooldown
            anim.SetClip("idle");
        }
        break;
    }

    anim.Update(deltaTime);  // Update animation
}

void BeamEnemy::CheckBeamDamage() {
    // so the enemy kills one time per attack to the player kill once per attack
    if (hasKilledPlayerThisAttack) return;

    // Get centers
    float centerX = posX + width * 0.5f;
    float centerY = posY + height * 0.5f;

    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float distanceX = fabs(playerCenterX - centerX);
    float distanceY = fabs(playerCenterY - centerY);

    bool hitHorizontal = false;
    bool hitVertical = false;

    // Check horizontal beam (left-right line)
    if (distanceY < beamHitboxWidth && distanceX < beamHorizontalLength) {
        hitHorizontal = true;
    }

    // Check vertical beam (up-down line)
    if (distanceX < beamHitboxWidth && distanceY < beamVerticalLength) {
        hitVertical = true;
    }

    // Create a small safe zone in the very center
    float centerSafeZone = 0.15f;
    bool inCenterSafeZone = (distanceX < centerSafeZone && distanceY < centerSafeZone);

    // Hit if touching either line, but NOT in the center safe zone
    bool hit = (hitHorizontal || hitVertical) && !inCenterSafeZone;

    // the player dies instantly
    if (hit && !g_player.isDead && !g_player.isInvincible && !g_player.isDashing) {
        g_player.health = 0.0f;
        OnPlayerDeath();
        hasKilledPlayerThisAttack = true;
    }
}

void BeamEnemy::OnDeath() {
    if (isDying) return;

    // Start death animation sequence
    Enemy::OnDeath();
    deathAnimationPhase = 0;  // Start with pre_death animation
    hasExploded = false;
}

void BeamEnemy::CreateDeathExplosion() {
    // Damages other enemies but not the player
    float centerX = posX + width * 0.5f;
    float centerY = posY + height * 0.5f;

    // Damage nearby enemies
    for (auto& enemy : g_enemies) {
        if (!enemy->IsAlive() || enemy == this) continue;

        float dx = enemy->GetX() + enemy->GetWidth() * 0.5f - centerX;
        float dy = enemy->GetY() + enemy->GetHeight() * 0.5f - centerY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance <= deathExplosionRadius) {
            float angle = atan2(dy, dx);
            enemy->TakeDamage((int)deathExplosionDamage, angle);
        }
    }
}



// 敌人更新函数
void UpdateEnemies(float deltaTime, MapManager* mapManager) {
    DamageNumberManager::Update(deltaTime);

    int visibleEnemyCount = 0;
    int totalEnemyCount = (int)g_enemies.size();

    // NOTE:
    // Some enemies (e.g., ThrowerEnemy) can spawn new enemies during Update().
    // Using a range-for over std::vector while it is modified can invalidate
    // references/iterators and crash. Iterate by index over the initial count.
    const size_t initialCount = g_enemies.size();
    for (size_t i = 0; i < initialCount; ++i) {
        Enemy* enemy = g_enemies[i];
        if (!enemy) continue;

        // 调试信息：计数可见敌人
        if (enemy->IsVisible(g_camera)) {
            visibleEnemyCount++;
        }

        enemy->Update(deltaTime, mapManager);
    }

    // 调试输出（可选）
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    if (debugTimer > 2.0f) {
        char debugMsg[256];
        sprintf_s(debugMsg, "Enemy optimization: Total=%d, Visible=%d, Optimization rate=%.1f%%\n",
            totalEnemyCount, visibleEnemyCount,
            (1.0f - (float)visibleEnemyCount / totalEnemyCount) * 100.0f);
        OutputDebugStringA(debugMsg);
        debugTimer = 0.0f;
    }

    // 移除死亡的敌人
    g_enemies.erase(
        std::remove_if(g_enemies.begin(), g_enemies.end(),
            [](Enemy* e) {
                if (!e) {
                    return true;
                }
                if (/*!e->IsAlive()*/e->IsMarkedForDeletion()) {
                    delete e;
                    return true;
                }
                return false;
            }),
        g_enemies.end()
    );
}

// 修改RenderEnemies函数
void RenderEnemies(const Camera& camera) {
    for (auto& enemy : g_enemies) {
        ID3D11ShaderResourceView* texture = g_enemyIdleTexture;  // 默认纹理

        if (dynamic_cast<FlyEnemy*>(enemy)) {  // 改为FlyEnemy
            texture = g_flyEnemyIdleTexture;
        }
        else if (dynamic_cast<MageEnemy*>(enemy)) {
            texture = g_mageEnemyIdleTexture;
        }
        else if (dynamic_cast<FastEnemy*>(enemy)) {
            texture = g_fastEnemyRunTexture;
        }
        else if (dynamic_cast<BombEnemy*>(enemy)) {
            texture = g_bombEnemyIdleTexture;
        }
        else if (dynamic_cast<SquareEnemy*>(enemy)) {
            texture = g_squareEnemyIdleTexture;
        }

        enemy->Render(texture, camera); // 传递相机参数
    }
    DamageNumberManager::Render(camera);
}

void CleanupEnemies() {
    DamageNumberManager::Clear();
    for (auto& enemy : g_enemies) {
        delete enemy;
    }
    g_enemies.clear();
}
