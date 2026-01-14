// Projectile.cpp
#include "Projectile.h"

// Global projectile manager instance
ProjectileManager& ProjectileManager::GetInstance() {
    static ProjectileManager instance;
    return instance;
}
Projectile::Projectile(ProjectileType type, float startX, float startY,
    float targetX, float targetY, float speed,
    const ProjectileEffect& effect, bool fromPlayer)
    : type(type), posX(startX), posY(startY), speed(speed), effect(effect),
    fromPlayer(fromPlayer), isActive(true), homingTarget(nullptr),
    currentPierceCount(0), rotation(0.0f), scaleEffect(1.0f) {

    // Calculate direction vector
    float dx = targetX - startX;
    float dy = targetY - startY;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance > 0) {
        velocityX = (dx / distance) * speed;
        velocityY = (dy / distance) * speed;
    }
    else {
        velocityX = speed;
        velocityY = 0;
    }

    // 根据速度方向计算初始旋转角度
    rotation = CalculateDirectionAngle();

    // Set initial properties based on type
    switch (type) {
    case ProjectileType::FIREBALL:
        size = 0.08f;
        maxLifeTime = 3.0f;
        homingStrength = 0.0f;
        break;
    case ProjectileType::ICE_SHARD:
        size = 0.05f;
        maxLifeTime = 4.0f;
        homingStrength = 0.0f;
        break;
    case ProjectileType::MAGIC_MISSILE:
        size = 0.04f;
        maxLifeTime = 5.0f;
        homingStrength = 5.0f;
        break;
    case ProjectileType::LIGHTNING:
        size = 0.02f;
        maxLifeTime = 0.5f; // Lightning has very short duration
        homingStrength = 0.0f;
        break;
    case ProjectileType::POISON_DART:
        size = 0.03f;
        maxLifeTime = 3.0f;
        homingStrength = 0.0f;
        break;
    case ProjectileType::HOLY_BOLT:
        size = 0.06f;
        maxLifeTime = 4.0f;
        homingStrength = 2.0f;
        break;
    }

    lifeTime = 0.0f;
}

void Projectile::Update(float deltaTime, MapManager* mapManager, std::vector<Enemy*>& enemies) {
    if (!isActive) return;

    lifeTime += deltaTime;

    // Check lifetime
    if (lifeTime >= maxLifeTime) {
        isActive = false;
        CreateImpactEffect();
        return;
    }

    // 保存当前速度方向
    float oldVelocityX = velocityX;
    float oldVelocityY = velocityY;

    // Type-specific update logic
    switch (type) {
    case ProjectileType::FIREBALL:
        UpdateFireball(deltaTime);
        break;
    case ProjectileType::ICE_SHARD:
        UpdateIceShard(deltaTime);
        break;
    case ProjectileType::MAGIC_MISSILE:
        UpdateMagicMissile(deltaTime, enemies);
        break;
    case ProjectileType::LIGHTNING:
        UpdateLightning(deltaTime);
        break;
    case ProjectileType::POISON_DART:
        UpdatePoisonDart(deltaTime);
        break;
    case ProjectileType::HOLY_BOLT:
        UpdateHolyBolt(deltaTime);
        break;
    }

    // 如果速度方向发生变化，更新旋转角度
    if (fabs(velocityX - oldVelocityX) > 0.001f || fabs(velocityY - oldVelocityY) > 0.001f) {
        // 计算新的方向角度
        float newDirectionAngle = CalculateDirectionAngle();
        float oldDirectionAngle = atan2(oldVelocityY, oldVelocityX);

        // 只对非自转子弹类型更新基础方向
        switch (type) {
        case ProjectileType::LIGHTNING:
        case ProjectileType::HOLY_BOLT:
            // 这些类型没有自转，直接更新旋转角度
            rotation = newDirectionAngle;
            break;
        default:
            // 其他类型保持原有的自转逻辑
            break;
        }
    }

    // Movement and collision detection
    Move(deltaTime);

    // Check map collision
    if (CheckMapCollision(mapManager)) {
        isActive = false;
        CreateImpactEffect();
        return;
    }
    CheckPlayerCollision();
    CheckEnemyCollision(enemies);
}
// Check collision with player
void  Projectile::CheckPlayerCollision() {
    // Only check player collision if projectile is not from player
    if (!isActive || fromPlayer) {
        return;
    }

    // Get player position and size
    float playerX = g_player.posX;
    float playerY = g_player.posY;
    float playerWidth = PLAYER_WIDTH;
    float playerHeight = PLAYER_HEIGHT;

    // 考虑玩家冲刺时的碰撞体变化
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    if (g_player.isDashing) {
        playerWidth = PLAYER_WIDTH * 0.25f;
        playerHeight = PLAYER_HEIGHT * 0.25f;
        offsetX = (PLAYER_WIDTH - playerWidth) * 0.5f;
        offsetY = (PLAYER_HEIGHT - playerHeight) * 0.5f;
    }

    // 计算射弹的实际碰撞体大小（考虑缩放效果）
    float actualSize = size * scaleEffect * 0.5f;

    // 射弹中心点
    float projectileCenterX = posX + actualSize * 0.5f;
    float projectileCenterY = posY + actualSize * 0.5f;

    // 玩家碰撞体中心点
    float playerCenterX = playerX + offsetX + playerWidth * 0.5f;
    float playerCenterY = playerY + offsetY + playerHeight * 0.5f;

    // 使用中心点距离检测碰撞（更准确）
    float dx = projectileCenterX - playerCenterX;
    float dy = projectileCenterY - playerCenterY;
    float distance = sqrt(dx * dx + dy * dy);
    
    // 碰撞半径
    float collisionRadius = (actualSize + std::min(playerWidth, playerHeight)) * 0.5f;

    if (distance < collisionRadius)
    {
        if (!g_player.isDashing && !g_player.isDead) {
            // Apply effect to player
            OnPlayerDeath();
            isActive = false; // 射弹命中后应该消失
        }
    }
}
void Projectile::UpdateFireball(float deltaTime) {
    // Fireball: gradually grows larger and accelerates
    scaleEffect = 1.0f + lifeTime * 0.5f;
    speed += deltaTime * 2.0f;
    // 火球的自转效果保留
    rotation += deltaTime * 10.0f;
}

void Projectile::UpdateIceShard(float deltaTime) {
    // Ice Shard: 基础方向 + 自转效果
    rotation += deltaTime * 15.0f;

    // Ice trail effect
    if (fmod(lifeTime, 0.1f) < 0.05f) {
        // Could add ice particle effects here
    }
}

void Projectile::UpdateLightning(float deltaTime) {
    // Lightning: 基础方向 + 闪烁效果，不自转
    scaleEffect = 0.8f + 0.4f * sin(lifeTime * 30.0f);
    // 闪电不自转，旋转角度来自基础方向
}

void Projectile::UpdatePoisonDart(float deltaTime) {
    // Poison Dart: 基础方向 + 自转
    // 正弦波运动
    float waveOffset = sin(lifeTime * 10.0f) * 0.02f;
    posX += waveOffset * deltaTime * 10.0f;
    rotation += deltaTime * 20.0f;
}

void Projectile::UpdateHolyBolt(float deltaTime) {
    // Holy Bolt: 基础方向 + 脉冲效果，不自转
    scaleEffect = 1.0f + 0.2f * sin(lifeTime * 8.0f);
    // 圣光箭不自转，旋转角度来自基础方向
}
void Projectile::UpdateMagicMissile(float deltaTime, std::vector<Enemy*>& enemies) {
    // Magic Missile: homes to the nearest enemy
    if (homingTarget && !homingTarget->IsAlive()) {
        homingTarget = nullptr;
    }

    if (!homingTarget) {
        // Find the nearest enemy
        float closestDistance = 1000.0f;
        for (auto& enemy : enemies) {
            if (enemy->IsAlive()) {
                // Use Get methods to get enemy position
                float enemyX = enemy->GetX();
                float enemyY = enemy->GetY();

                float dx = enemyX - posX;
                float dy = enemyY - posY;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance < closestDistance) {
                    closestDistance = distance;
                    homingTarget = enemy;
                }
            }
        }
    }

    // 保存旧的旋转角度
    float oldDirectionAngle = CalculateDirectionAngle();

    // Home towards target
    if (homingTarget && homingStrength > 0) {
        float dx = homingTarget->GetX() - posX;
        float dy = homingTarget->GetY() - posY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance > 0) {
            // Gradually adjust direction
            float targetVX = (dx / distance) * speed;
            float targetVY = (dy / distance) * speed;

            velocityX += (targetVX - velocityX) * homingStrength * deltaTime;
            velocityY += (targetVY - velocityY) * homingStrength * deltaTime;

            // Normalize velocity
            float currentSpeed = sqrt(velocityX * velocityX + velocityY * velocityY);
            velocityX = (velocityX / currentSpeed) * speed;
            velocityY = (velocityY / currentSpeed) * speed;

            // 计算新的方向角度
            float newDirectionAngle = CalculateDirectionAngle();

            // 平滑过渡旋转角度
            float angleDiff = newDirectionAngle - oldDirectionAngle;

            // 将角度差标准化到[-π, π]范围内
            while (angleDiff > 3.14159f) angleDiff -= 2 * 3.14159f;
            while (angleDiff < -3.14159f) angleDiff += 2 * 3.14159f;

            // 使用插值平滑旋转过渡
            rotation += angleDiff * 0.5f;
        }
    }
    else {
        // 如果没有跟踪目标，保持原有的自转
        rotation += deltaTime * 8.0f;
    }
}
void Projectile::Move(float deltaTime) {
    posX += velocityX * deltaTime;
    posY += velocityY * deltaTime;
}

bool Projectile::CheckMapCollision(MapManager* mapManager) {
    if (!mapManager || !mapManager->GetCurrentMap()) return false;

    auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
    for (const auto& tile : solidTiles) {
        // Simple rectangle collision detection
        if (posX < tile.posX + tile.width &&
            posX + size > tile.posX &&
            posY < tile.posY + tile.height &&
            posY + size > tile.posY) {
            return true;
        }
    }
    return false;
}

void Projectile::CheckEnemyCollision(std::vector<Enemy*>& enemies) {
    // Check enemy collision if projectile is from player
    if (!isActive || !fromPlayer) {
        return;
    }
    for (auto& enemy : enemies) {
        if (!enemy->IsAlive()) continue;

        // Use Get methods to get enemy attributes
        float enemyX = enemy->GetX();
        float enemyY = enemy->GetY();
        float enemyWidth = enemy->GetWidth();
        float enemyHeight = enemy->GetHeight();

        // Collision detection (using values from Get methods)
        if (posX < enemyX + enemyWidth &&
            posX + size > enemyX &&
            posY < enemyY + enemyHeight &&
            posY + size > enemyY) {

            ApplyEffectToEnemy(enemy);

            if (!effect.pierce || currentPierceCount >= effect.maxPierceCount) {
                isActive = false;
                CreateImpactEffect();
                return;
            }
            else {
                currentPierceCount++;
            }
        }
    }
}


void Projectile::ApplyEffectToEnemy(Enemy* enemy) {
    if (!enemy || !enemy->IsAlive()) return;

    // Calculate attack angle (direction from projectile to enemy)
    float dx = enemy->GetX() - posX;
    float dy = enemy->GetY() - posY;
    float attackAngle = atan2(dy, dx);

    // Apply damage
    enemy->TakeDamage((int)effect.damage, attackAngle);

    // Apply special effects
    // TODO: Add status effects like burning, slowing, stunning, etc.
    // Need to add a status effect system to the Enemy class
}

void Projectile::CreateImpactEffect() {
    // TODO: Create impact effects
    // Can add particle effects, sound effects, etc. here
}


ID3D11ShaderResourceView* ProjectileManager::GetTextureForType(ProjectileType type) {
    switch (type) {
    case ProjectileType::FIREBALL: return fireballTexture;
    case ProjectileType::ICE_SHARD: return iceShardTexture;
    case ProjectileType::MAGIC_MISSILE: return magicMissileTexture;
    case ProjectileType::LIGHTNING: return lightningTexture;
    case ProjectileType::POISON_DART: return poisonDartTexture;
    case ProjectileType::HOLY_BOLT: return holyBoltTexture;
    default: return fireballTexture;
    }
}

void Projectile::Render(const Camera& camera) {
    if (!isActive) return;

    // Get corresponding texture
    ID3D11ShaderResourceView* texture = ProjectileManager::GetInstance().GetTextureForType(type);
    if (!texture) return;

    // Convert to screen coordinates
    float screenX, screenY;
    float cameraX = camera.GetX();
    float cameraY = camera.GetY();
    screenX = posX - cameraX;
    screenY = posY - cameraY;

    // Set color based on type
    switch (type) {
    case ProjectileType::FIREBALL:
        SetColor(1.0f, 0.5f, 0.2f, 1.0f);
        break;
    case ProjectileType::ICE_SHARD:
        SetColor(0.6f, 0.8f, 1.0f, 1.0f);
        break;
    case ProjectileType::MAGIC_MISSILE:
        SetColor(0.8f, 0.3f, 0.9f, 1.0f);
        break;
    case ProjectileType::LIGHTNING:
        SetColor(0.9f, 0.9f, 0.2f, 1.0f);
        break;
    case ProjectileType::POISON_DART:
        SetColor(0.4f, 0.8f, 0.3f, 1.0f);
        break;
    case ProjectileType::HOLY_BOLT:
        SetColor(1.0f, 1.0f, 0.8f, 1.0f);
        break;
    }

    // 获取总旋转角度
    float totalRotation = GetRotationAngle();

    // 渲染射弹（带旋转和缩放）
    float renderSize = size * scaleEffect;
    RenderImage(screenX, screenY, renderSize, renderSize, texture, 0, 1, 1, false, totalRotation, false);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}
// ProjectileManager class implementation
void ProjectileManager::AddProjectile(ProjectileType type, float startX, float startY,
    float targetX, float targetY, float speed,
    const ProjectileEffect& effect, bool fromPlayer) {
    projectiles.emplace_back(type, startX, startY, targetX, targetY, speed, effect, fromPlayer);
}

void ProjectileManager::Update(float deltaTime, MapManager* mapManager, std::vector<Enemy*>& enemies) {
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        it->Update(deltaTime, mapManager, enemies);

        if (!it->IsActive()) {
            it = projectiles.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ProjectileManager::Render(const Camera& camera) {
    for (auto& projectile : projectiles) {
        projectile.Render(camera);
    }
}

void ProjectileManager::ClearAll() {
    projectiles.clear();
}

void ProjectileManager::LoadTextures(ID3D11Device* device) {
    // Load various projectile textures
    LoadTexture(device, "asset/enemy/enemy_005_thorn/enemy_005_thorn_Pbullet_right.png", &fireballTexture);
    LoadTexture(device, "asset/Projectile_IceShard.png", &iceShardTexture);

}

// Predefined projectile creation functions
void ProjectileManager::CreateFireball(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 100000.0f;
    effect.burnDamage = 5.0f;
    effect.areaRadius = 0.3f;

    AddProjectile(ProjectileType::FIREBALL, startX, startY, targetX, targetY, 0.2f, effect, fromPlayer);
}

void ProjectileManager::CreateIceShard(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 15.0f;
    effect.slowEffect = 0.5f;
    effect.stunDuration = 0.5f;

    AddProjectile(ProjectileType::ICE_SHARD, startX, startY, targetX, targetY, 10.0f, effect, fromPlayer);
}

void ProjectileManager::CreateMagicMissile(float startX, float startY, Enemy* target, bool fromPlayer) {
    if (!target) return;

    ProjectileEffect effect;
    effect.damage = 20.0f;
    effect.pierce = true;
    effect.maxPierceCount = 2;

    AddProjectile(ProjectileType::MAGIC_MISSILE, startX, startY,
        target->GetX(), target->GetY(), 6.0f, effect, fromPlayer);
}

void ProjectileManager::CreateLightningStrike(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 40.0f;
    effect.stunDuration = 1.0f;

    AddProjectile(ProjectileType::LIGHTNING, startX, startY, targetX, targetY, 20.0f, effect, fromPlayer);
}

void ProjectileManager::CreatePoisonDart(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 8.0f;
    effect.burnDamage = 3.0f; // Using burnDamage as poison over time damage

    AddProjectile(ProjectileType::POISON_DART, startX, startY, targetX, targetY, 12.0f, effect, fromPlayer);
}

void ProjectileManager::CreateHolyBolt(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 30.0f;
    effect.pierce = true;
    effect.maxPierceCount = 3;

    AddProjectile(ProjectileType::HOLY_BOLT, startX, startY, targetX, targetY, 9.0f, effect, fromPlayer);
}

// 在Projectile.cpp中添加以下方法实现

// 计算速度方向角度
float Projectile::CalculateDirectionAngle() const {
    // 计算速度方向的角度（弧度）
    return atan2(velocityY, velocityX);
}

// 获取旋转角度
float Projectile::GetRotationAngle() const {
    // 基础的方向角度
    float baseAngle = CalculateDirectionAngle();

    // 根据子弹类型调整旋转
    switch (type) {
    case ProjectileType::FIREBALL:
        // 火球：基础方向角度 + 自转角度
        return baseAngle;
    case ProjectileType::ICE_SHARD:
        // 冰箭：基础方向角度 + 自转角度
        return baseAngle ;
    case ProjectileType::MAGIC_MISSILE:
        // 魔法飞弹：基础方向角度 + 自转角度
        return baseAngle + rotation;
    case ProjectileType::LIGHTNING:
        // 闪电：基础方向角度，加上随机的闪烁效果
        return baseAngle;
    case ProjectileType::POISON_DART:
        // 毒箭：基础方向角度 + 自转角度
        return baseAngle + rotation;
    case ProjectileType::HOLY_BOLT:
        // 圣光箭：基础方向角度
        return baseAngle;
    default:
        return baseAngle;
    }
}