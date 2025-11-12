#include "Enemy.h"

// Enemy类实现
Enemy::Enemy(float x, float y, float hp)
    : posX(x), posY(y), health(hp), maxHealth(hp), isAlive(true),
    currentState(PATROL), patrolMinX(-1.0f), patrolMaxX(1.0f), attackRange(0.8f) {

    width = PLAYER_WIDTH * 1.2f;
    height = PLAYER_HEIGHT * 1.2f;
    moveSpeed = MOVE_SPEED * 0.7f;

    // 初始化伤害系数
    for (int i = 0; i < 8; i++) {
        damageMultipliers[i] = 1.0f;
    }

    facingAngle = 0.0f;
    velocityX = 0.0f;
    velocityY = 0.0f;
}

void Enemy::SetDamageMultiplier(Direction dir, float multiplier) {
    if (dir >= DIR_RIGHT && dir <= DIR_DOWN_RIGHT) {
        damageMultipliers[static_cast<int>(dir)] = multiplier;
    }
}

float Enemy::GetDamageMultiplier(float attackAngle) {
    float relativeAngle = NormalizeAngle(attackAngle - facingAngle);
    int directionIndex = AngleToDirectionIndex(relativeAngle);
    return damageMultipliers[directionIndex];
}

void Enemy::TakeDamage(float damage, float attackAngle) {
    if (!isAlive) return;

    float multiplier = GetDamageMultiplier(attackAngle);
    float actualDamage = damage * multiplier;

    health -= actualDamage;
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

void Enemy::Update(float deltaTime) {
    if (!isAlive) return;

    // 应用重力（简单版本）
    velocityY += GRAVITY * deltaTime * 60.0f;

    // 保存旧位置用于碰撞检测
    float oldX = posX;
    float oldY = posY;

    // 移动
    posX += velocityX * deltaTime * 60.0f;
    posY += velocityY * deltaTime * 60.0f;

    // 碰撞检测与地图块
    for (const auto& block : g_mapBlocks) {
        if (block.isSolid && CheckCollisionWithBlock(block)) {
            // 简单碰撞响应：回退到之前的位置
            posX = oldX;
            posY = oldY;
            velocityX = 0;
            velocityY = 0;
            break;
        }
    }

    // 边界检查
    if (posX < -1.0f) posX = -1.0f;
    if (posX > 1.0f - width) posX = 1.0f - width;
    if (posY < -2.0f) {
        isAlive = false; // 掉落死亡
        return;
    }

    UpdateAI(deltaTime);
}

void Enemy::UpdateAI(float deltaTime) {
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    // 更新朝向
    if (dx != 0 || dy != 0) {
        facingAngle = atan2(dy, dx);
    }

    // 简单的状态机
    switch (currentState) {
    case PATROL:
        PatrolBehavior(deltaTime);
        if (distance < 10.0f) currentState = CHASE;
        break;
    case CHASE:
        ChaseBehavior(deltaTime);
        if (distance > 10.0f) currentState = PATROL;
        //if (distance < attackRange) currentState = ATTACK;
        break;
    case ATTACK:
        AttackBehavior(deltaTime);
        if (distance > attackRange + 0.2f) currentState = CHASE;
        if (health < maxHealth * 0.3f) currentState = FLEE;
        break;
    case FLEE:
        FleeBehavior(deltaTime);
        if (health > maxHealth * 0.3f) currentState = CHASE;
        break;
    }
}

void Enemy::PatrolBehavior(float deltaTime) {
    static float patrolDirection = 1.0f;

    // 简单的巡逻逻辑
    if (posX <= patrolMinX) patrolDirection = 1.0f;
    if (posX >= patrolMaxX) patrolDirection = -1.0f;

    velocityX = patrolDirection * moveSpeed * 0.5f;
    velocityY = 0;
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

    // 这里可以添加攻击逻辑
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

void Enemy::Render(ID3D11ShaderResourceView* texture) {
    if (!isAlive) return;

    // 根据血量状态选择不同的帧
    int frameIndex = 0; // 默认帧

    // 如果血量低于30%，使用受伤帧
    if (health < maxHealth * 0.3f) {
        frameIndex = 1;
    }

    // 如果正在攻击，使用攻击帧
    if (currentState == ATTACK) {
        frameIndex = 2;
    }

    // 渲染敌人
    RenderImage(posX, posY, width, height, texture, frameIndex, 1, 3);

    // 渲染血条
    RenderHealthBar();
}

void Enemy::RenderHealthBar() {
    float barWidth = width;
    float barHeight = 0.02f;
    float barX = posX;
    float barY = posY + height + 0.02f;

    // 背景条（红色）- 使用地面纹理，帧索引0
    RenderImage(barX, barY, barWidth, barHeight, g_groundTexture, 0, 1, 1);

    // 血量条（绿色）- 使用地面纹理，帧索引1
    float healthRatio = health / maxHealth;
    RenderImage(barX, barY, barWidth * healthRatio, barHeight, g_groundTexture, 1, 1, 1);
}

bool Enemy::CheckPlayerCollision() {
    return CheckCollision(posX, posY, width, height,
        g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT);
}

bool Enemy::CheckCollisionWithBlock(const MapBlock& block) {
    return CheckCollision(posX, posY, width, height,
        block.posX, block.posY, block.width, block.height);
}

float Enemy::NormalizeAngle(float angle) {
    while (angle < 0) angle += 2 * 3.14159f;
    while (angle >= 2 * 3.14159f) angle -= 2 * 3.14159f;
    return angle;
}

int Enemy::AngleToDirectionIndex(float angle) {
    float sector = 3.14159f / 4.0f;
    int index = static_cast<int>((angle + sector / 2) / sector) % 8;
    return index;
}

// ShieldEnemy 实现
ShieldEnemy::ShieldEnemy(float x, float y) : Enemy(x, y, 150.0f) {
    // 盾牌敌人：正面减伤，背面增伤
    SetDamageMultiplier(DIR_RIGHT, 0.1f);
    SetDamageMultiplier(DIR_UP_RIGHT, 0.3f);
    SetDamageMultiplier(DIR_UP, 0.5f);
    SetDamageMultiplier(DIR_UP_LEFT, 0.7f);
    SetDamageMultiplier(DIR_LEFT, 2.0f);
    SetDamageMultiplier(DIR_DOWN_LEFT, 1.8f);
    SetDamageMultiplier(DIR_DOWN, 1.5f);
    SetDamageMultiplier(DIR_DOWN_RIGHT, 0.3f);

    width = PLAYER_WIDTH * 1.5f;
    moveSpeed = MOVE_SPEED * 0.5f;
}

void ShieldEnemy::OnHit(float damage) {
    // 盾牌敌人被击中时可能会格挡
    if (damage < 5.0f) { // 小伤害完全格挡
        health += damage; // 回滚伤害
    }
}

void ShieldEnemy::OnDeath() {
    Enemy::OnDeath();
    // 盾牌敌人死亡时可能掉落盾牌
}

// MageEnemy 实现
MageEnemy::MageEnemy(float x, float y) : Enemy(x, y, 80.0f) {
    // 法师敌人：脆弱但远程
    SetDamageMultiplier(DIR_UP, 2.0f);
    SetDamageMultiplier(DIR_DOWN, 2.0f);

    spellCooldown = 3.0f;
    currentSpellCooldown = 0.0f;
    attackRange = 1.2f;
    moveSpeed = MOVE_SPEED * 0.4f;
}

void MageEnemy::Update(float deltaTime) {
    Enemy::Update(deltaTime);

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
    // 比如创建火球、闪电等
}

void MageEnemy::Render(ID3D11ShaderResourceView* texture) {
    if (!isAlive) return;

    // 法师敌人有特殊的帧索引逻辑
    int frameIndex = 0; // 默认帧

    if (health < maxHealth * 0.3f) {
        frameIndex = 1; // 受伤帧
    }

    if (currentState == ATTACK) {
        frameIndex = 2; // 施法帧
    }

    // 渲染法师敌人
    RenderImage(posX, posY, width, height, texture, frameIndex, 1, 3);

    // 渲染血条
    RenderHealthBar();

    // 法师敌人有魔法特效（如果正在攻击）
    if (currentState == ATTACK) {
        float effectSize = width * 1.3f;
        // 使用特效纹理，假设有3帧动画
        RenderImage(posX - (effectSize - width) * 0.5f,
            posY - (effectSize - height) * 0.5f,
            effectSize, effectSize, g_chargeEffectTexture, 0, 1, 3);
    }
}

// FastEnemy 实现
FastEnemy::FastEnemy(float x, float y) : Enemy(x, y, 60.0f) {
    moveSpeed = MOVE_SPEED * 1.5f;
    dashCooldown = 2.0f;
    currentDashCooldown = 0.0f;
}

void FastEnemy::Update(float deltaTime) {
    Enemy::Update(deltaTime);

    if (currentDashCooldown > 0) {
        currentDashCooldown -= deltaTime;
    }

    if (currentState == ATTACK && currentDashCooldown <= 0) {
        DashAttack();
        currentDashCooldown = dashCooldown;
    }
}

void FastEnemy::DashAttack() {
    // 快速敌人进行冲刺攻击
    velocityX = (g_player.posX > posX ? 1.0f : -1.0f) * moveSpeed * 3.0f;
}

// 敌人管理函数
void InitEnemies() {
    // 加载敌人纹理
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_enemyTexture);
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_shieldEnemyTexture);
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_mageEnemyTexture);
    LoadTexture(g_pDevice, "asset/Enemy_Shield.png", &g_fastEnemyTexture);

    // 如果某些纹理加载失败，使用默认纹理
    if (!g_enemyTexture) g_enemyTexture = g_playerTexture;
    if (!g_shieldEnemyTexture) g_shieldEnemyTexture = g_enemyTexture;
    if (!g_mageEnemyTexture) g_mageEnemyTexture = g_enemyTexture;
    if (!g_fastEnemyTexture) g_fastEnemyTexture = g_enemyTexture;
}

void CreateTestEnemies() {
    CleanupEnemies();

    // 创建各种类型的敌人
    g_enemies.push_back(new Enemy(-0.3f, -0.5f, 80.0f));
    g_enemies.push_back(new ShieldEnemy(0.4f, -0.3f));
    g_enemies.push_back(new MageEnemy(0.0f, 0.0f));
    g_enemies.push_back(new FastEnemy(0.6f, -0.7f));
}

void UpdateEnemies(float deltaTime) {
    for (auto& enemy : g_enemies) {
        enemy->Update(deltaTime);
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

void RenderEnemies() {
    for (auto& enemy : g_enemies) {
        // 根据敌人类型选择不同的纹理
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

        enemy->Render(texture);
    }
}

void CleanupEnemies() {
    for (auto& enemy : g_enemies) {
        delete enemy;
    }
    g_enemies.clear();
}