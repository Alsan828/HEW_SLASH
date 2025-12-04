// Projectile.cpp
#include "Projectile.h"

// 全局射弹管理器实例
ProjectileManager& ProjectileManager::GetInstance() {
    static ProjectileManager instance;
    return instance;
}

// Projectile 类实现
Projectile::Projectile(ProjectileType type, float startX, float startY,
    float targetX, float targetY, float speed,
    const ProjectileEffect& effect, bool fromPlayer)
    : type(type), posX(startX), posY(startY), speed(speed), effect(effect),
    fromPlayer(fromPlayer), isActive(true), homingTarget(nullptr),
    currentPierceCount(0), rotation(0.0f), scaleEffect(1.0f) {

    // 计算方向向量
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

    // 根据类型设置初始属性
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
        maxLifeTime = 0.5f; // 闪电持续时间很短
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

    // 检查生命周期
    if (lifeTime >= maxLifeTime) {
        isActive = false;
        CreateImpactEffect();
        return;
    }

    // 类型特定的更新逻辑
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

    // 移动和碰撞检测
    Move(deltaTime);

    // 检查地图碰撞
    if (CheckMapCollision(mapManager)) {
        isActive = false;
        CreateImpactEffect();
        return;
    }

    // 检查敌人碰撞
    CheckEnemyCollision(enemies);
}

void Projectile::UpdateFireball(float deltaTime) {
    // 火球：逐渐变大并加速
    scaleEffect = 1.0f + lifeTime * 0.5f;
    speed += deltaTime * 2.0f;
    rotation += deltaTime * 10.0f;
}

void Projectile::UpdateIceShard(float deltaTime) {
    // 冰箭：旋转效果
    rotation += deltaTime * 15.0f;

    // 冰晶拖尾效果
    if (fmod(lifeTime, 0.1f) < 0.05f) {
        // 可以在这里添加冰晶粒子效果
    }
}


void Projectile::UpdateMagicMissile(float deltaTime, std::vector<Enemy*>& enemies) {
    // 魔法飞弹：跟踪最近的敌人
    if (homingTarget && !homingTarget->IsAlive()) {
        homingTarget = nullptr;
    }

    if (!homingTarget) {
        // 寻找最近的敌人
        float closestDistance = 1000.0f;
        for (auto& enemy : enemies) {
            if (enemy->IsAlive()) {
                // 使用Get函数获取敌人位置
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
    // 跟踪目标
    if (homingTarget && homingStrength > 0) {
        float dx = homingTarget->GetX() - posX;
        float dy = homingTarget->GetY() - posY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance > 0) {
            // 逐步调整方向
            float targetVX = (dx / distance) * speed;
            float targetVY = (dy / distance) * speed;

            velocityX += (targetVX - velocityX) * homingStrength * deltaTime;
            velocityY += (targetVY - velocityY) * homingStrength * deltaTime;

            // 标准化速度
            float currentSpeed = sqrt(velocityX * velocityX + velocityY * velocityY);
            velocityX = (velocityX / currentSpeed) * speed;
            velocityY = (velocityY / currentSpeed) * speed;
        }
    }

    rotation += deltaTime * 8.0f;
}

void Projectile::UpdateLightning(float deltaTime) {
    // 闪电：快速闪烁效果
    scaleEffect = 0.8f + 0.4f * sin(lifeTime * 30.0f);
}

void Projectile::UpdatePoisonDart(float deltaTime) {
    // 毒镖：轻微正弦波移动
    float waveOffset = sin(lifeTime * 10.0f) * 0.02f;
    posX += waveOffset * deltaTime * 10.0f;
    rotation += deltaTime * 20.0f;
}

void Projectile::UpdateHolyBolt(float deltaTime) {
    // 圣光箭：脉冲光效
    scaleEffect = 1.0f + 0.2f * sin(lifeTime * 8.0f);
    rotation += deltaTime * 5.0f;
}

void Projectile::Move(float deltaTime) {
    posX += velocityX * deltaTime;
    posY += velocityY * deltaTime;
}

bool Projectile::CheckMapCollision(MapManager* mapManager) {
    if (!mapManager || !mapManager->GetCurrentMap()) return false;

    auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
    for (const auto& tile : solidTiles) {
        // 简单矩形碰撞检测
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
    for (auto& enemy : enemies) {
        if (!enemy->IsAlive()) continue;

        // 使用Get函数获取敌人属性
        float enemyX = enemy->GetX();
        float enemyY = enemy->GetY();
        float enemyWidth = enemy->GetWidth();
        float enemyHeight = enemy->GetHeight();

        // 碰撞检测（使用Get函数获取的值）
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

    // 计算攻击角度（从射弹到敌人的方向）
    float dx = enemy->GetX() - posX;
    float dy = enemy->GetY() - posY;
    float attackAngle = atan2(dy, dx);

    // 造成伤害
    enemy->TakeDamage((int)effect.damage, attackAngle);

    // 应用特殊效果
    // TODO: 这里可以添加燃烧、减速、眩晕等状态效果
    // 需要为Enemy类添加状态效果系统
}

void Projectile::CreateImpactEffect() {
    // TODO: 创建碰撞特效
    // 可以在这里添加粒子效果、声音效果等
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

    // 获取对应纹理
    ID3D11ShaderResourceView* texture = ProjectileManager::GetInstance().GetTextureForType(type);
    if (!texture) return;

    // 转换为屏幕坐标
    float screenX, screenY;
    float cameraX = camera.GetX();
    float cameraY = camera.GetY();
    screenX = posX - cameraX;
    screenY = posY - cameraY;

    // 根据类型设置颜色
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

    // 渲染射弹（带旋转和缩放）
    float renderSize = size * scaleEffect;
    RenderImage(screenX, screenY, renderSize, renderSize, texture, 0, 1, 1);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

// ProjectileManager 类实现
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
    // 加载各种射弹纹理
    LoadTexture(device, "asset/Projectile_Fireball.png", &fireballTexture);
    LoadTexture(device, "asset/Projectile_IceShard.png", &iceShardTexture);
    LoadTexture(device, "asset/Projectile_MagicMissile.png", &magicMissileTexture);
    LoadTexture(device, "asset/Projectile_Lightning.png", &lightningTexture);
    LoadTexture(device, "asset/Projectile_PoisonDart.png", &poisonDartTexture);
    LoadTexture(device, "asset/Projectile_HolyBolt.png", &holyBoltTexture);

    // 设置默认纹理（如果加载失败）
    if (!fireballTexture) fireballTexture = g_enemyTexture;
    if (!iceShardTexture) iceShardTexture = g_enemyTexture;
    if (!magicMissileTexture) magicMissileTexture = fireballTexture;
    if (!lightningTexture) lightningTexture = fireballTexture;
    if (!poisonDartTexture) poisonDartTexture = fireballTexture;
    if (!holyBoltTexture) holyBoltTexture = fireballTexture;
}

// 预定义射弹创建函数
void ProjectileManager::CreateFireball(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 25.0f;
    effect.burnDamage = 5.0f;
    effect.areaRadius = 0.3f;

    AddProjectile(ProjectileType::FIREBALL, startX, startY, targetX, targetY, 2.0f, effect, fromPlayer);
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
    effect.burnDamage = 3.0f; // 这里作为中毒持续伤害

    AddProjectile(ProjectileType::POISON_DART, startX, startY, targetX, targetY, 12.0f, effect, fromPlayer);
}

void ProjectileManager::CreateHolyBolt(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 30.0f;
    effect.pierce = true;
    effect.maxPierceCount = 3;

    AddProjectile(ProjectileType::HOLY_BOLT, startX, startY, targetX, targetY, 9.0f, effect, fromPlayer);
}