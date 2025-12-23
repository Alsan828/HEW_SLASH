// Enemy.cpp
#include "Enemy.h"
#include "Map.h"
#include "Projectile.h"

// 初始化伤害数字管理器
std::vector<DamageNumber> DamageNumberManager::damageNumbers;

// 定义敌人纹理
ID3D11ShaderResourceView* g_enemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_enemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_flyEnemyIdleTexture = nullptr;  // 改为飞行敌人纹理
ID3D11ShaderResourceView* g_flyEnemyDeathTexture = nullptr;  // 改为飞行敌人死亡纹理

ID3D11ShaderResourceView* g_mageEnemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_mageEnemyAttackTexture = nullptr;
ID3D11ShaderResourceView* g_mageEnemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_fastEnemyRunTexture = nullptr;
ID3D11ShaderResourceView* g_fastEnemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_bombEnemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_bombEnemyDeathTexture = nullptr;

// 修改InitEnemies函数，加载所有纹理
void InitEnemies() {
    // 加载敌人纹理
    // 普通敌人
    LoadTexture(g_pDevice, "asset/enemy/enemy_001_eye/enemy_001_eye_idle.png", &g_enemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_001_eye/enemy_001_eye_death.png", &g_enemyDeathTexture);

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
}

// Enemy类实现
Enemy::Enemy(float x, float y, float hp)
    : posX(x), posY(y), health(hp), maxHealth(hp), isAlive(true),
    currentState(PATROL), patrolMinX(-1.0f), patrolMaxX(1.0f), weakSpotDeath(false), attackRange(0.0f) {

    width = PLAYER_WIDTH * 1.2f;
    height = PLAYER_HEIGHT * 1.2f;
    moveSpeed = MOVE_SPEED * 0.65f;

    // 为基类敌人添加默认动画剪辑
    anim.AddClip("idle", 0, 1, 1, 1, 0.1f, true, g_enemyIdleTexture);
    anim.AddClip("death", 0, 4, 1, 5, 0.2f, false, g_enemyDeathTexture);

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

    health -= actualDamage;
    isHit = true;
    hitTimer = HIT_DURATION;
    OnHit(actualDamage);

    if (health <= 0) {
        health = 0;
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

    OnEnemyDefeated();
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

    // 更新面向方向
    if (dx != 0) {
        facingRight = (dx > 0);
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

    // 更新面向方向
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // 只有远程敌人才在攻击范围内停止移动
    if (attackRange > 0 && distance <= attackRange) {
        // 远程敌人在攻击范围内停止移动
        velocityX = 0;
    }
    else {
        // 近战敌人一直向玩家移动
        if (dx > 0) {
            velocityX = moveSpeed;
        }
        else {
            velocityX = -moveSpeed;
        }
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
    anim.AddClip("death", 0, 3, 1, 4, 0.1f, false, g_flyEnemyDeathTexture);

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
    anim.AddClip("death", 0, 3, 1, 4, 0.2f, false, g_mageEnemyDeathTexture); // for when I kill the enemy
    anim.SetClip("idle");


    // 射弹相关参数
    projectileSpeed = 2.0f;
    lastAttackTime = 0.0f;
    attackCooldown = 1.5f;  // 攻击冷却时间
    projectileDamage = 20.0f;

    scale = 3.0f;
}

void MageEnemy::Update(float deltaTime, MapManager* mapManager) {
    Enemy::Update(deltaTime, mapManager);

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
            posY + height * 0.7f,  // 从敌人高度70%处发射
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
    moveSpeed = MOVE_SPEED * 1.5f;
    dashCooldown = 2.0f;
    currentDashCooldown = 0.0f;
    detectionRange = 4.0f;
    attackRange = 0.5f;

    attackRange = 0.0f;  // 近战敌人
    anim.AddClip("run", 0, 3, 1, 4, 0.05f, true, g_fastEnemyRunTexture);
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
    anim.AddClip("death", 0, 3, 1, 4, 0.3f, false, g_bombEnemyDeathTexture);

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

    // 炸弹敌人不移动，所以不需要处理重力和碰撞
    velocityX = 0.0f;
    velocityY = 0.0f;

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

void BombEnemy::CreateProjectiles() {
    // 获取ProjectileManager实例
    ProjectileManager& projectileManager = ProjectileManager::GetInstance();

    // 创建火球效果配置
    ProjectileEffect fireballEffect;
    fireballEffect.damage = 30.0f;  // 基础伤害
    fireballEffect.burnDamage = 5.0f;  // 燃烧伤害
    fireballEffect.areaRadius = 0.3f;  // 爆炸半径
    fireballEffect.pierce = false;  // 不穿透

    float projectileSpeed = 3.0f;  // 射弹速度

    // 向左发射火球
    projectileManager.CreateFireball(
        posX,  // 起始X
        posY + height * 0.5f,  // 从敌人中心高度发射
        posX - 10.0f,  // 左侧远处位置
        posY + height * 0.5f,  // 水平方向
        true  // 来自玩家
    );

    // 向右发射火球
    projectileManager.CreateFireball(
        posX,  // 起始X
        posY + height * 0.5f,  // 从敌人中心高度发射
        posX + 10.0f,  // 右侧远处位置
        posY + height * 0.5f,  // 水平方向
        true  // 来自玩家
    );

    // 可以在这里添加粒子效果
    // CreateParticleEffect(posX, posY, "explosion");
}


BossEnemy::BossEnemy(float x, float y) : Enemy(x, y, 500000.0f)
{
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



// 敌人更新函数
void UpdateEnemies(float deltaTime, MapManager* mapManager) {
    DamageNumberManager::Update(deltaTime);

    int visibleEnemyCount = 0;
    int totalEnemyCount = (int)g_enemies.size();

    for (auto& enemy : g_enemies) {
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
