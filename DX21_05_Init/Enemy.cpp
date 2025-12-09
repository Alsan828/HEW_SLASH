#include "Enemy.h"
#include "Map.h"
#include "Projectile.h"

// Initialize damage number manager
std::vector<DamageNumber> DamageNumberManager::damageNumbers;

// Enemy class implementation
Enemy::Enemy(float x, float y, float hp)
    : posX(x), posY(y), health(hp), maxHealth(hp), isAlive(true),
    currentState(PATROL), patrolMinX(-1.0f), patrolMaxX(1.0f), attackRange(0.08f) {

    width = PLAYER_WIDTH * 1.2f;
    height = PLAYER_HEIGHT * 1.2f;
    moveSpeed = MOVE_SPEED * 0.65f;

    // Initialize damage multipliers
    for (int i = 0; i < 8; i++) {
        damageMultipliers[i] = 1.0f;
    }

    facingRight = true;  // Default facing right
    velocityX = 0.0f;
    velocityY = 0.0f;

    // Set base 8-direction damage multipliers
    SetDamageMultiplier(DIR_FRONT, 1.0f);
    SetDamageMultiplier(DIR_FRONT_UP, 1.0f);
    SetDamageMultiplier(DIR_UP, 1.0f);
    SetDamageMultiplier(DIR_BACK_UP, 1.0f);
    SetDamageMultiplier(DIR_BACK, 1.0f);
    SetDamageMultiplier(DIR_BACK_DOWN, 1.0f);
    SetDamageMultiplier(DIR_DOWN, 1.0f);
    SetDamageMultiplier(DIR_FRONT_DOWN, 1.0f);
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

// Calculate relative angle (based on enemy facing direction)
float Enemy::GetRelativeAngle(float attackAngle) const {
    // When facing right, 0 degrees is front; when facing left, 180 degrees is front
    float enemyFrontAngle = facingRight ? 0.0f : 3.14159f;
    float relativeAngle = attackAngle - enemyFrontAngle;

    // Normalize to [-π, π]
    while (relativeAngle > 3.14159f) relativeAngle -= 2 * 3.14159f;
    while (relativeAngle < -3.14159f) relativeAngle += 2 * 3.14159f;

    return relativeAngle;
}

// Convert angle to direction index (8 directions)
int Enemy::AngleToDirectionIndex(float relativeAngle) {
    // Normalize relative angle to [0, 2π]
    float angle = relativeAngle;
    if (angle < 0) angle += 2 * 3.14159f;

    // 8 directions, each 45 degrees
    float sector = 3.14159f / 4.0f;

    // Calculate direction index
    int index = static_cast<int>((angle + sector / 2) / sector) % 8;
    return index;
}

float Enemy::NormalizeAngle(float angle) {
    while (angle < 0) angle += 2 * 3.14159f;
    while (angle >= 2 * 3.14159f) angle -= 2 * 3.14159f;
    return angle;
}

// Calculate damage based on attack angle
int Enemy::CalculateDamageFromPlayer(int baseDamage, float playerDashAngle) {
    float multiplier = GetDamageMultiplier(playerDashAngle);
    return (int)(baseDamage * multiplier);
}

// Use DamageNumberManager in TakeDamage method
void Enemy::TakeDamage(int damage, float attackAngle) {
    if (!isAlive) return;

    float multiplier = GetDamageMultiplier(attackAngle);
    int actualDamage = (int)(damage * multiplier);

    // Use independent damage number manager
    bool isCritical = (multiplier > 1.5f);
    DamageNumberManager::AddDamageNumber(
        posX + width * 0.5f,  // Enemy center X
        posY + height,        // Top of enemy
        actualDamage,
        isCritical
    );

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


void Enemy::OnHit(int damage) {
    // Base enemy has no special behavior when hit
}

void Enemy::OnDeath() {
    // Base enemy death handling
    isAlive = false;
    OnEnemyDefeated();
}
void Enemy::Update(float deltaTime, MapManager* mapManager) {
    if (!isAlive) return;

    // 可见性检测和优化逻辑
    bool isCurrentlyVisible = IsVisible(g_camera); // 假设有全局相机对象

    // 如果不可见且不需要最小更新，则跳过完整更新
    if (!isCurrentlyVisible && !NeedsMinimalUpdate()) {
        // 只更新离开屏幕计时器
        offScreenTimer += deltaTime;

        // 如果离开屏幕时间过长且状态简单，完全跳过更新
        if (offScreenTimer > MAX_OFFSCREEN_TIME &&
            currentState == PATROL &&
            !isHit &&
            health >= maxHealth) {
            return;
        }
    }

    // 如果从不可见变为可见，重置计时器
    if (isCurrentlyVisible && !wasVisible) {
        ResetOffScreenTimer();
    }
    wasVisible = isCurrentlyVisible;

    // 如果不可见但需要最小更新，执行简化版更新
    if (!isCurrentlyVisible && NeedsMinimalUpdate()) {
        UpdateMinimal(deltaTime);
        return;
    }

    // 完整更新逻辑（原有代码）
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

    // 移动和碰撞检测（原有完整逻辑）
    posX += velocityX * deltaTime * 60.0f;

    // 水平碰撞检测
    bool horizontalCollision = false;
    if (mapManager && mapManager->GetCurrentMap()) {
        auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (CheckCollisionWithTile(tile)) {
                posX = oldX;
                horizontalCollision = true;
                break;
            }
        }
    }

    posY += velocityY * deltaTime * 60.0f;

    // 垂直碰撞检测
    bool verticalCollision = false;
    if (mapManager && mapManager->GetCurrentMap()) {
        auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (CheckCollisionWithTile(tile)) {
                posY = oldY;
                verticalCollision = true;
                velocityY = 0;
                break;
            }
        }
    }

    if (horizontalCollision) {
        velocityX = 0;
    }

    // 边界检查
    if (mapManager && mapManager->GetCurrentMap()) {
        if (posY < -5.0f) {
            isAlive = false;
            return;
        }
    }
    else {
        if (posX < -1.0f) posX = -1.0f;
        if (posX > 1.0f - width) posX = 1.0f - width;
        if (posY < -2.0f) {
            isAlive = false;
            return;
        }
    }

    // 只在可见时更新AI（避免屏幕外敌人消耗计算资源）
    if (isCurrentlyVisible) {
        UpdateAI(deltaTime);
    }
    else {
        // 屏幕外敌人简化AI更新
        UpdateAIMinimal(deltaTime);
    }

    // 更新离开屏幕计时器
    if (!isCurrentlyVisible) {
        offScreenTimer += deltaTime;
    }
    else {
        offScreenTimer = 0.0f;
    }
}

// 简化版更新（用于屏幕外但需要更新的敌人）
void Enemy::UpdateMinimal(float deltaTime) {
    // 简化版AI更新（只处理状态转换，不计算路径等）
    UpdateAIMinimal(deltaTime);

    // 更新离开屏幕计时器
    offScreenTimer += deltaTime;
}

// 简化版AI更新
void Enemy::UpdateAIMinimal(float deltaTime) {
    // 只处理最基本的状态维护，不进行复杂计算
    float dx = g_player.posX - posX;

    // 更新面向方向
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // 简化状态机：只处理超时或关键状态转换
    static float stateTimer = 0.0f;
    stateTimer += deltaTime;

    // 每5秒检查一次状态转换（降低频率）
    if (stateTimer >= 5.0f) {
        float distance = fabs(dx);

        // 简化版状态转换逻辑
        switch (currentState) {
        case PATROL:
            if (distance < 3.0f) currentState = CHASE;
            break;
        case CHASE:
            if (distance > 8.0f) currentState = PATROL;
            break;
        case ATTACK:
            // 攻击状态保持，直到条件改变
            break;
        case FLEE:
            if (health > maxHealth * 0.5f) currentState = CHASE;
            break;
        }

        stateTimer = 0.0f;
    }
}

void Enemy::UpdateAI(float deltaTime) {
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    // Update facing direction
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // State machine logic
    switch (currentState) {
    case PATROL:
        PatrolBehavior(deltaTime);
        if (distance < 3.0f) currentState = CHASE;
        break;
    case CHASE:
        ChaseBehavior(deltaTime);
        if (distance > 8.0f) currentState = PATROL;
        if (distance < attackRange) currentState = ATTACK;
        break;
    case ATTACK:
        AttackBehavior(deltaTime);
        if (distance > attackRange + 0.2f) currentState = CHASE;
        if (health < maxHealth * 0.3f) currentState = FLEE;
        break;
    case FLEE:
        FleeBehavior(deltaTime);
        if (health > maxHealth * 0.5f) currentState = CHASE;
        break;
    }
}

void Enemy::PatrolBehavior(float deltaTime) {
    static float patrolDirection = 1.0f;
    static float patrolTimer = 0.0f;

    patrolTimer += deltaTime;

    // Check if need to change direction every 2 seconds
    if (patrolTimer >= 2.0f) {
        if (posX <= patrolMinX || posX >= patrolMaxX) {
            patrolDirection *= -1.0f;
        }
        patrolTimer = 0.0f;
    }

    velocityX = patrolDirection * moveSpeed * 0.5f;

    // Add small vertical velocity variation to avoid complete stillness
    if (velocityY == 0) {
        velocityY = 0.01f;
    }
}

void Enemy::ChaseBehavior(float deltaTime) {
    if (g_player.posX > posX) {
        velocityX = moveSpeed;
    }
    else {
        velocityX = -moveSpeed;
    }

    // Simple jump attempt
    if (abs(g_player.posX - posX) < 0.3f && g_player.posY > posY + 0.2f) {
        velocityY = JUMP_FORCE * 0.8f;
    }
}

void Enemy::AttackBehavior(float deltaTime) {
    // Stop moving to attack
    velocityX = 0;
    velocityY = 0;
}

void Enemy::FleeBehavior(float deltaTime) {
    // Move away from player
    if (g_player.posX > posX) {
        velocityX = -moveSpeed;
    }
    else {
        velocityX = moveSpeed;
    }
}


void Enemy::WorldToScreenPosition(float worldX, float worldY, float& screenX, float& screenY, const Camera& camera) {
    // Get camera position (camera center coordinates)
    float cameraX = camera.GetX();
    float cameraY = camera.GetY();

    // Convert world coordinates to screen coordinates (relative coordinates)
    // Assume rendering system uses screen center as origin (0,0)
    screenX = worldX - cameraX;
    screenY = worldY - cameraY;
}

void Enemy::Render(ID3D11ShaderResourceView* texture, const Camera& camera) {
    if (!isAlive) return;

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    // Convert world coordinates to screen coordinates
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    // Select different frames based on health status
    int frameIndex = 0;
    if (health < maxHealth * 0.3f) {
        frameIndex = 1;
    }
    if (currentState == ATTACK) {
        frameIndex = 2;
    }
    // Hit state: flicker or color change effect
    if (isHit) {
        SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    }

    // Render enemy using screen coordinates
    RenderImage(screenX, screenY, width, height, texture, frameIndex, 1, 3);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    // Render health bar (also needs screen coordinates)
    RenderHealthBar(camera);
}

void Enemy::RenderHealthBar(const Camera& camera) {
    // Convert world coordinates to screen coordinates
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    float barWidth = width;
    float barHeight = 0.02f;
    float barX = screenX;
    float barY = screenY + height + 0.02f;

    // Background bar (red)
    RenderImage(barX, barY, barWidth, barHeight, g_groundTexture, 0, 1, 1);

    // Health bar (green)
    float healthRatio = health / maxHealth;
    RenderImage(barX, barY, barWidth * healthRatio, barHeight, g_groundTexture, 1, 1, 1);
}

bool Enemy::CheckPlayerCollision() {
    return CheckCollision(posX, posY, width, height,
        g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT);
}

bool Enemy::CheckCollisionWithTiles(const std::vector<MapTile>& solidTiles) {
    for (const auto& tile : solidTiles) {
        if (CheckCollisionWithTile(tile)) {
            return true;
        }
    }
    return false;
}

bool Enemy::CheckCollisionWithTile(const MapTile& tile) {
    return CheckCollision(posX, posY, width, height,
        tile.posX, tile.posY, tile.width, tile.height);
}

// ShieldEnemy implementation - modify damage multiplier settings
ShieldEnemy::ShieldEnemy(float x, float y) : Enemy(x, y, 150.0f) {
    // Shield enemy: reduced damage from front, increased damage from back
    SetDamageMultiplier(DIR_FRONT, 0.1f);
    SetDamageMultiplier(DIR_FRONT_UP, 0.3f);
    SetDamageMultiplier(DIR_FRONT_DOWN, 0.3f);
    SetDamageMultiplier(DIR_BACK, 2.0f);
    SetDamageMultiplier(DIR_BACK_UP, 1.5f);
    SetDamageMultiplier(DIR_BACK_DOWN, 1.5f);

    width = PLAYER_WIDTH * 1.5f;
    moveSpeed = MOVE_SPEED * 0.5f;
}


void ShieldEnemy::Update(float deltaTime, MapManager* mapManager) {
    Enemy::Update(deltaTime, mapManager);

    // Shield enemy special AI logic
    if (currentState == PATROL) {
        // Shield enemy patrols slower but more stable
        velocityX *= 0.8f;
    }
}

void ShieldEnemy::OnHit(int damage) {
    // Shield enemy may block when hit
    if (damage < 5) {
        health += damage; // Rollback damage
    }
}

void ShieldEnemy::OnDeath() {
    Enemy::OnDeath();
}
// MageEnemy implementation
MageEnemy::MageEnemy(float x, float y) : Enemy(x, y, 80.0f) {
    // Mage enemy: vulnerable from top and bottom
    SetDamageMultiplier(DIR_UP, 2.0f);
    SetDamageMultiplier(DIR_DOWN, 2.0f);

    spellCooldown = 3.0f;
    currentSpellCooldown = 0.0f;
    attackRange = 1.2f;
    moveSpeed = MOVE_SPEED * 0.4f;
}

void MageEnemy::Update(float deltaTime, MapManager* mapManager) {
    Enemy::Update(deltaTime, mapManager);

    if (currentSpellCooldown > 0) {
        currentSpellCooldown -= deltaTime;
    }

    // Cast spell in attack state
    if (currentState == ATTACK && currentSpellCooldown <= 0) {
        CastSpell();
        currentSpellCooldown = spellCooldown;
    }
}

void MageEnemy::CastSpell() {
    // Implement spell casting logic here
}

void MageEnemy::Render(ID3D11ShaderResourceView* texture, const Camera& camera) {
    if (!isAlive) return;

    // Convert world coordinates to screen coordinates
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    int frameIndex = 0;
    if (health < maxHealth * 0.3f) {
        frameIndex = 1;
    }
    if (currentState == ATTACK) {
        frameIndex = 2;
    }

    // Render mage enemy
    RenderImage(screenX, screenY, width, height, texture, frameIndex, 1, 3);

    // Render health bar
    RenderHealthBar(camera);

    // Mage enemy has magic effects (also needs screen coordinates)
    if (currentState == ATTACK) {
        float effectSize = width * 1.3f;
        float effectX = screenX - (effectSize - width) * 0.5f;
        float effectY = screenY - (effectSize - height) * 0.5f;

        RenderImage(effectX, effectY, effectSize, effectSize, g_chargeEffectTexture, 0, 1, 3);
    }
}

// FastEnemy implementation
FastEnemy::FastEnemy(float x, float y) : Enemy(x, y, 60.0f) {
    moveSpeed = MOVE_SPEED * 1.5f;
    dashCooldown = 2.0f;
    currentDashCooldown = 0.0f;
}

void FastEnemy::Update(float deltaTime, MapManager* mapManager) {
    Enemy::Update(deltaTime, mapManager);

    if (currentDashCooldown > 0) {
        currentDashCooldown -= deltaTime;
    }

    if (currentState == ATTACK && currentDashCooldown <= 0) {
        DashAttack();
        currentDashCooldown = dashCooldown;
    }
}

void FastEnemy::DashAttack() {
    velocityX = (g_player.posX > posX ? 1.0f : -1.0f) * moveSpeed * 3.0f;
}

// 在全局纹理定义中添加
ID3D11ShaderResourceView* g_bombEnemyTexture = nullptr;

// BombEnemy实现
// Enemy.cpp
// BombEnemy实现
BombEnemy::BombEnemy(float x, float y) : Enemy(x, y, 120.0f) {
    // 爆炸敌人：上下方向是十倍伤害，其他方向减伤
    SetDamageMultiplier(DIR_UP, 10.0f);        // 上方十倍伤害
    SetDamageMultiplier(DIR_DOWN, 10.0f);      // 下方十倍伤害
    SetDamageMultiplier(DIR_FRONT, 0.7f);     // 正面减伤
    SetDamageMultiplier(DIR_BACK, 0.7f);      // 背面减伤
    SetDamageMultiplier(DIR_FRONT_UP, 1.2f);  // 前上中等
    SetDamageMultiplier(DIR_FRONT_DOWN, 1.2f);// 前下中等
    SetDamageMultiplier(DIR_BACK_UP, 1.2f);   // 后上中等
    SetDamageMultiplier(DIR_BACK_DOWN, 1.2f); // 后下中等

    // 爆炸敌人属性
    width = PLAYER_WIDTH * 1.3f;
    height = PLAYER_HEIGHT * 1.3f;
    moveSpeed = 0.0f;  // 不会移动
    pulseTimer = 0.0f;
    baseSize = 1.0f;

    // 设置巡逻范围为0，因为不会移动
    patrolMinX = posX;
    patrolMaxX = posX;
}

// 重写TakeDamage函数，添加爆炸检测
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
        multiplier >= 10.0f  // 如果是上下方向，显示为暴击
    );

    // 检查是否为上下方向攻击
    float relativeAngle = GetRelativeAngle(attackAngle);
    int directionIndex = AngleToDirectionIndex(relativeAngle);
    bool isVerticalAttack = (directionIndex == DIR_UP || directionIndex == DIR_DOWN);

    // 如果是上下方向攻击，立即死亡并触发爆炸
    if (multiplier >= 10.0f) {
        health = 0;  // 立即死亡
        isAlive = false;
        OnDeath();  // 触发爆炸
        return;     // 直接返回，不执行后续逻辑
    }

    // 非上下方向攻击，正常处理伤害
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

void BombEnemy::Update(float deltaTime, MapManager* mapManager) {
    if (!isAlive) return;

    // 调用基类的受击状态更新
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    // 爆炸敌人不移动，所以不需要处理重力和碰撞
    velocityX = 0.0f;
    velocityY = 0.0f;

    // 脉动效果
    pulseTimer += deltaTime;
    float pulseEffect = sin(pulseTimer * 3.0f) * 0.1f;
    baseSize = 1.0f + pulseEffect;

    // 简化AI：只检测玩家距离
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    // 更新面向方向
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // 简单状态机
    if (distance < 2.0f) {
        currentState = ATTACK;  // 玩家靠近时进入攻击状态
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

// 重写OnDeath函数，处理爆炸效果
void BombEnemy::OnDeath() {
    // 先调用基类的OnDeath
    Enemy::OnDeath();

    // 然后触发爆炸效果
    Explode();
}

void BombEnemy::Render(ID3D11ShaderResourceView* texture, const Camera& camera) {
    if (!isAlive) return;

    // 转换为屏幕坐标
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    // 根据状态和血量选择帧
    int frameIndex = 0;
    if (health < maxHealth * 0.3f) {
        frameIndex = 1;  // 低血量帧
    }
    if (currentState == ATTACK) {
        frameIndex = 2;  // 攻击状态帧
    }

    // 应用颜色效果
    if (isHit) {
        SetColor(1.0f, 0.0f, 0.0f, 1.0f);  // 受击红色
    }
    else if (currentState == ATTACK) {
        // 攻击状态时，脉动颜色变化
        float pulse = 0.5f + 0.5f * sin(pulseTimer * 5.0f);
        SetColor(1.0f, 0.3f + pulse * 0.5f, 0.3f, 1.0f);  // 红-橙脉动
    }
    else {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);  // 正常颜色
    }

    // 应用脉动缩放
    float renderWidth = width * baseSize;
    float renderHeight = height * baseSize;
    float offsetX = (width - renderWidth) * 0.5f;
    float offsetY = (height - renderHeight) * 0.5f;

    // 渲染敌人
    RenderImage(screenX + offsetX, screenY + offsetY, renderWidth, renderHeight,
        texture, frameIndex, 1, 3);

    // 渲染生命条
    RenderHealthBar(camera);

    // 如果处于攻击状态，显示警告效果
    if (currentState == ATTACK) {
        float warningSize = renderWidth * 1.5f;
        float warningX = screenX - (warningSize - renderWidth) * 0.5f;
        float warningY = screenY - (warningSize - renderHeight) * 0.5f;

        float alpha = 0.3f + 0.3f * sin(pulseTimer * 4.0f);
        SetColor(1.0f, 0.3f, 0.1f, alpha);
        RenderImage(warningX, warningY, warningSize, warningSize,
            g_groundTexture, 0, 1, 1);
    }

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void BombEnemy::Explode() {
    // 播放爆炸音效
    // PlaySound("explosion.wav");

    // 创建爆炸特效
    // CreateExplosionEffect(posX, posY);

    // 对周围敌人造成伤害
    float explosionRadius = 1.5f;  // 爆炸半径

    for (Enemy* otherEnemy : g_enemies) {
        if (otherEnemy == this || !otherEnemy->IsAlive()) continue;

        float dx = otherEnemy->GetX() - posX;
        float dy = otherEnemy->GetY() - posY;
        float distance = sqrt(dx * dx + dy * dy);

        // 如果在爆炸半径内
        if (distance <= explosionRadius) {
            // 向左右发射子弹的逻辑
            // 这里可以创建一个Projectile对象或直接造成伤害

            // 简单实现：直接造成伤害
            // 计算攻击角度（从左到右）
            float attackAngle = (dx > 0) ? 0.0f : 3.14159f;  // 左或右

            // 对敌人造成伤害
            int explosionDamage = 30;  // 爆炸基础伤害
            otherEnemy->TakeDamage(explosionDamage, attackAngle);
        }
    }

    // 对玩家造成伤害
    if (g_player.isAlive) {
        float dx = g_player.posX - posX;
        float dy = g_player.posY - posY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance <= explosionRadius) {
            // 对玩家造成伤害
            // 这里需要调用玩家的受伤函数
            // g_player.TakeDamage(20);

            // 显示伤害数字
            DamageNumberManager::AddDamageNumber(
                posX + width * 0.5f,
                posY + height * 0.5f,
                20,
                false
            );
        }
    }

    // 创建向左和向右的射弹
    CreateProjectiles();
}

// Enemy.cpp
void BombEnemy::CreateProjectiles() {
    // 获取 ProjectileManager 实例
    ProjectileManager& projectileManager = ProjectileManager::GetInstance();

    // 创建火球效果配置
    ProjectileEffect fireballEffect;
    fireballEffect.damage = 30.0f;  // 基础伤害
    fireballEffect.burnDamage = 5.0f;  // 燃烧持续伤害
    fireballEffect.areaRadius = 0.3f;  // 范围爆炸半径
    fireballEffect.pierce = false;  // 不穿透

    float projectileSpeed = 3.0f;  // 射弹速度

    // 向左发射火球
    projectileManager.CreateFireball(
        posX,  // 起点X
        posY + height * 0.5f,  // 从敌人中心高度发射
        posX - 10.0f,  // 向左很远的位置
        posY + height * 0.5f,  // 水平方向
        false  // 来自敌人
    );

    // 向右发射火球
    projectileManager.CreateFireball(
        posX,  // 起点X
        posY + height * 0.5f,  // 从敌人中心高度发射
        posX + 10.0f,  // 向右很远的位置
        posY + height * 0.5f,  // 水平方向
        false  // 来自敌人
    );

    // 保留伤害数字显示作为反馈
    DamageNumberManager::AddDamageNumber(
        posX, posY + height + 0.5f,
        0,  // 显示0表示爆炸特效
        true
    );

    // 可以在这里添加粒子效果
    // CreateParticleEffect(posX, posY, "explosion");
}

// Enemy management functions
void InitEnemies() {
    // Load enemy textures
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_enemyTexture);
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_shieldEnemyTexture);
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_mageEnemyTexture);
    LoadTexture(g_pDevice, "asset/Enemy_Shield.png", &g_fastEnemyTexture);

    if (!g_enemyTexture) g_enemyTexture = g_playerTexture;
    if (!g_shieldEnemyTexture) g_shieldEnemyTexture = g_enemyTexture;
    if (!g_mageEnemyTexture) g_mageEnemyTexture = g_enemyTexture;
    if (!g_fastEnemyTexture) g_fastEnemyTexture = g_enemyTexture;
}

void UpdateEnemies(float deltaTime, MapManager* mapManager) {
    DamageNumberManager::Update(deltaTime);

    int visibleEnemyCount = 0;
    int totalEnemyCount = g_enemies.size();

    for (auto& enemy : g_enemies) {
        // 调试信息：统计可见敌人数量
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
        sprintf_s(debugMsg, "敌人优化: 总数=%d, 可见=%d, 优化率=%.1f%%\n",
            totalEnemyCount, visibleEnemyCount,
            (1.0f - (float)visibleEnemyCount / totalEnemyCount) * 100.0f);
        OutputDebugStringA(debugMsg);
        debugTimer = 0.0f;
    }

    // 移除死亡敌人
    g_enemies.erase(
        std::remove_if(g_enemies.begin(), g_enemies.end(),
            [](Enemy* e) {
                if (!e->IsAlive()) {
                    delete e;
                    return true;
                }
                return false;
            }),
        g_enemies.end()
    );
}
// Modify RenderEnemies function to pass camera parameter
void RenderEnemies(const Camera& camera) {
    for (auto& enemy : g_enemies) {
        ID3D11ShaderResourceView* texture = g_enemyTexture;

        if (dynamic_cast<ShieldEnemy*>(enemy)) {
            texture = g_shieldEnemyTexture;
        }
        else if (dynamic_cast<MageEnemy*>(enemy)) {
            texture = g_mageEnemyTexture;
        }
        else if (dynamic_cast<FastEnemy*>(enemy)) {
            texture = g_fastEnemyTexture;
        }

        enemy->Render(texture, camera); // Pass camera parameter
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


void DamageNumberManager::AddDamageNumber(float x, float y, int damage, bool isCritical) {
    damageNumbers.emplace_back(x, y, damage, isCritical);
}

void DamageNumberManager::Update(float deltaTime) {
    for (auto it = damageNumbers.begin(); it != damageNumbers.end();) {
        it->timer += deltaTime;
        it->posY += it->velocityY * deltaTime;
        it->velocityY -= 2.0f * deltaTime; // Gravity effect

        if (it->timer >= it->lifeTime) {
            it = damageNumbers.erase(it);
        }
        else {
            ++it;
        }
    }
}

void DamageNumberManager::Render(const Camera& camera) {
    for (auto& number : damageNumbers) {
        float screenX, screenY;
        // Use static world-to-screen coordinate conversion function
        float cameraX = camera.GetX();
        float cameraY = camera.GetY();
        screenX = number.posX - cameraX;
        screenY = number.posY - cameraY;

        // Calculate alpha (fade-out effect)
        float alpha = 1.0f - (number.timer / number.lifeTime);

        // Set color based on whether it's critical
        if (number.isCritical) {
            //SetColor(1.0f, 0.0f, 0.0f, alpha); // Red critical numbers
            SetColor(1.0f, 1.0f, 1.0f, alpha); // White normal numbers
        }
        else {
            SetColor(1.0f, 1.0f, 1.0f, alpha); // White normal numbers
        }

        // Use existing number rendering functionality
        RenderNumber(number.value, screenX, screenY, 0.07f, 0.1f, pTextureNum);
    }

    // Reset color
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void DamageNumberManager::Clear() {
    damageNumbers.clear();
}