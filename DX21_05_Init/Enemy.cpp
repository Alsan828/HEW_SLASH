#include "Enemy.h"
#include "Map.h"

// 初始化伤害数字管理器
std::vector<DamageNumber> DamageNumberManager::damageNumbers;

// Enemy类实现
Enemy::Enemy(float x, float y, float hp)
    : posX(x), posY(y), health(hp), maxHealth(hp), isAlive(true),
    currentState(PATROL), patrolMinX(-1.0f), patrolMaxX(1.0f), attackRange(0.08f) {

    width = PLAYER_WIDTH * 1.2f;
    height = PLAYER_HEIGHT * 1.2f;
    moveSpeed = MOVE_SPEED * 0.65f;

    // 初始化伤害系数
    for (int i = 0; i < 8; i++) {
        damageMultipliers[i] = 1.0f;
    }

    facingRight = true;  // 默认朝右
    velocityX = 0.0f;
    velocityY = 0.0f;

    // 设置基础的8方向伤害系数
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

// 计算相对角度（基于敌人面向方向）
float Enemy::GetRelativeAngle(float attackAngle) const {
    // 敌人朝右时，0度是正前方；朝左时，180度是正前方
    float enemyFrontAngle = facingRight ? 0.0f : 3.14159f;
    float relativeAngle = attackAngle - enemyFrontAngle;

    // 标准化到 [-π, π]
    while (relativeAngle > 3.14159f) relativeAngle -= 2 * 3.14159f;
    while (relativeAngle < -3.14159f) relativeAngle += 2 * 3.14159f;

    return relativeAngle;
}

// 角度转方向索引（8方向）
int Enemy::AngleToDirectionIndex(float relativeAngle) {
    // 将相对角度标准化到 [0, 2π]
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
float Enemy::CalculateDamageFromPlayer(float baseDamage, float playerDashAngle) {
    float multiplier = GetDamageMultiplier(playerDashAngle);
    return baseDamage * multiplier;
}

// 在TakeDamage方法中改用DamageNumberManager
void Enemy::TakeDamage(float damage, float attackAngle) {
    if (!isAlive) return;

    float multiplier = GetDamageMultiplier(attackAngle);
    float actualDamage = damage * multiplier;

    // 使用独立的伤害数字管理器
    bool isCritical = (multiplier > 1.5f);
    DamageNumberManager::AddDamageNumber(
        posX + width * 0.5f,  // 敌人中心X
        posY + height,        // 敌人头顶
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


void Enemy::OnHit(float damage) {
    // 基础敌人被击中时没有特殊行为
}

void Enemy::OnDeath() {
    // 基础敌人死亡处理
    isAlive = false;
}

void Enemy::Update(float deltaTime, MapManager* mapManager) {
    if (!isAlive) return;

    // 更新受击状态
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

    // 移动
    posX += velocityX * deltaTime * 60.0f;

    // 水平碰撞检测 - 使用地图管理器获取固体瓦片
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

    // 只在水平碰撞时重置水平速度
    if (horizontalCollision) {
        velocityX = 0;
    }

    // 边界检查 - 使用地图边界
    if (mapManager && mapManager->GetCurrentMap()) {
        // 这里可以添加基于地图的边界检查
        if (posY < -5.0f) { // 掉落死亡高度
            isAlive = false;
            return;
        }
    }
    else {
        // 后备边界检查
        if (posX < -1.0f) posX = -1.0f;
        if (posX > 1.0f - width) posX = 1.0f - width;
        if (posY < -2.0f) {
            isAlive = false;
            return;
        }
    }

    UpdateAI(deltaTime);
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

    // 每2秒检查一次是否需要转向
    if (patrolTimer >= 2.0f) {
        if (posX <= patrolMinX || posX >= patrolMaxX) {
            patrolDirection *= -1.0f;
        }
        patrolTimer = 0.0f;
    }

    velocityX = patrolDirection * moveSpeed * 0.5f;

    // 添加小的垂直速度变化，避免完全静止
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

    // 简单的跳跃尝试
    if (abs(g_player.posX - posX) < 0.3f && g_player.posY > posY + 0.2f) {
        velocityY = JUMP_FORCE * 0.8f;
    }
}

void Enemy::AttackBehavior(float deltaTime) {
    // 停止移动进行攻击
    velocityX = 0;
    velocityY = 0;
}

void Enemy::FleeBehavior(float deltaTime) {
    // 远离玩家
    if (g_player.posX > posX) {
        velocityX = -moveSpeed;
    }
    else {
        velocityX = moveSpeed;
    }
}

 
void Enemy::WorldToScreenPosition(float worldX, float worldY, float& screenX, float& screenY, const Camera& camera) {
    // 获取相机位置（相机中心坐标）
    float cameraX = camera.GetX();
    float cameraY = camera.GetY();

    // 将世界坐标转换为屏幕坐标（相对坐标）
    // 假设渲染系统以屏幕中心为原点(0,0)
    screenX = worldX - cameraX;
    screenY = worldY - cameraY;
}

void Enemy::Render(ID3D11ShaderResourceView* texture, const Camera& camera) {
    if (!isAlive) return;

    // 将世界坐标转换为屏幕坐标
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    // 根据血量状态选择不同的帧
    int frameIndex = 0;
    if (health < maxHealth * 0.3f) {
        frameIndex = 1;
    }
    if (currentState == ATTACK) {
        frameIndex = 2;
    }
    // 受击状态：闪烁或变色效果
    if (isHit) {
        SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    }

    // 使用屏幕坐标渲染敌人
    RenderImage(screenX, screenY, width, height, texture, frameIndex, 1, 3);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    // 渲染血条（也需要使用屏幕坐标）
    RenderHealthBar(camera);
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

    // 血量条（绿色）
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

// ShieldEnemy 实现 - 修改伤害系数设置
ShieldEnemy::ShieldEnemy(float x, float y) : Enemy(x, y, 150.0f) {
    // 盾牌敌人：正面减伤，背面增伤
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

    // 盾牌敌人的特殊AI逻辑
    if (currentState == PATROL) {
        // 盾牌敌人巡逻更慢但更稳定
        velocityX *= 0.8f;
    }
}

void ShieldEnemy::OnHit(float damage) {
    // 盾牌敌人被击中时可能会格挡
    if (damage < 5.0f) {
        health += damage; // 回滚伤害
    }
}

void ShieldEnemy::OnDeath() {
    Enemy::OnDeath();
}
// MageEnemy 实现
MageEnemy::MageEnemy(float x, float y) : Enemy(x, y, 80.0f) {
    // 法师敌人：上下方向脆弱
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

    // 在攻击状态下施法
    if (currentState == ATTACK && currentSpellCooldown <= 0) {
        CastSpell();
        currentSpellCooldown = spellCooldown;
    }
}

void MageEnemy::CastSpell() {
    // 这里可以实现施法逻辑
}

void MageEnemy::Render(ID3D11ShaderResourceView* texture, const Camera& camera) {
    if (!isAlive) return;

    // 将世界坐标转换为屏幕坐标
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    int frameIndex = 0;
    if (health < maxHealth * 0.3f) {
        frameIndex = 1;
    }
    if (currentState == ATTACK) {
        frameIndex = 2;
    }

    // 渲染法师敌人
    RenderImage(screenX, screenY, width, height, texture, frameIndex, 1, 3);

    // 渲染血条
    RenderHealthBar(camera);

    // 法师敌人有魔法特效（也需要屏幕坐标）
    if (currentState == ATTACK) {
        float effectSize = width * 1.3f;
        float effectX = screenX - (effectSize - width) * 0.5f;
        float effectY = screenY - (effectSize - height) * 0.5f;

        RenderImage(effectX, effectY, effectSize, effectSize, g_chargeEffectTexture, 0, 1, 3);
    }
}

// FastEnemy 实现
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

// 敌人管理函数
void InitEnemies() {
    // 加载敌人纹理
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_enemyTexture);
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_shieldEnemyTexture);
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_mageEnemyTexture);
    LoadTexture(g_pDevice, "asset/Enemy_Shield.png", &g_fastEnemyTexture);

    if (!g_enemyTexture) g_enemyTexture = g_playerTexture;
    if (!g_shieldEnemyTexture) g_shieldEnemyTexture = g_enemyTexture;
    if (!g_mageEnemyTexture) g_mageEnemyTexture = g_enemyTexture;
    if (!g_fastEnemyTexture) g_fastEnemyTexture = g_enemyTexture;
}


void UpdateEnemies(float deltaTime, MapManager* mapManager) {    // 更新伤害数字

    DamageNumberManager::Update(deltaTime);
    for (auto& enemy : g_enemies) {
        enemy->Update(deltaTime, mapManager);
    }

    // 移除死亡的敌人
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

// 修改RenderEnemies函数，传入相机参数
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


void DamageNumberManager::AddDamageNumber(float x, float y, float damage, bool isCritical) {
    damageNumbers.emplace_back(x, y, damage, isCritical);
}

void DamageNumberManager::Update(float deltaTime) {
    for (auto it = damageNumbers.begin(); it != damageNumbers.end();) {
        it->timer += deltaTime;
        it->posY += it->velocityY * deltaTime;
        it->velocityY -= 2.0f * deltaTime; // 重力效果

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
        // 使用静态的世界坐标转屏幕坐标函数
        float cameraX = camera.GetX();
        float cameraY = camera.GetY();
        screenX = number.posX - cameraX;
        screenY = number.posY - cameraY;

        // 计算透明度（淡出效果）
        float alpha = 1.0f - (number.timer / number.lifeTime);

        // 根据是否为暴击设置颜色
        if (number.isCritical) {
            SetColor(1.0f, 0.0f, 0.0f, alpha); // 红色暴击数字
        }
        else {
            SetColor(1.0f, 1.0f, 1.0f, alpha); // 白色普通数字
        }

        // 使用你现有的数字渲染功能
        RenderNumber(number.value, screenX, screenY, 0.03f, 0.03f, pTextureNum);
    }

    // 重置颜色
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void DamageNumberManager::Clear() {
    damageNumbers.clear();
}
