// Projectile.cpp
#include "Projectile.h"

// Global projectile manager instance
ProjectileManager& ProjectileManager::GetInstance() {
    static ProjectileManager instance;
    return instance;
}

// Projectile class implementation
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

    // Movement and collision detection
    Move(deltaTime);

    // Check map collision
    if (CheckMapCollision(mapManager)) {
        isActive = false;
        CreateImpactEffect();
        return;
    }
    CheckPlayerCollision();
    // Check enemy collision
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

    // Collision detection
    if (posX < playerX + playerWidth &&
        posX + size > playerX &&
        posY < playerY + playerHeight &&
        posY + size > playerY)
    {
        if (!g_player.isDashing) {
            // Apply effect to player
            OnPlayerDeath();

            CreateImpactEffect();
        }


    }
}

void Projectile::UpdateFireball(float deltaTime) {
    // Fireball: gradually grows larger and accelerates
    scaleEffect = 1.0f + lifeTime * 0.5f;
    speed += deltaTime * 2.0f;
    rotation += deltaTime * 10.0f;
}

void Projectile::UpdateIceShard(float deltaTime) {
    // Ice Shard: rotation effect
    rotation += deltaTime * 15.0f;

    // Ice trail effect
    if (fmod(lifeTime, 0.1f) < 0.05f) {
        // Could add ice particle effects here
    }
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
        }
    }

    rotation += deltaTime * 8.0f;
}

void Projectile::UpdateLightning(float deltaTime) {
    // Lightning: quick flicker effect
    scaleEffect = 0.8f + 0.4f * sin(lifeTime * 30.0f);
}

void Projectile::UpdatePoisonDart(float deltaTime) {
    // Poison Dart: slight sine wave movement
    float waveOffset = sin(lifeTime * 10.0f) * 0.02f;
    posX += waveOffset * deltaTime * 10.0f;
    rotation += deltaTime * 20.0f;
}

void Projectile::UpdateHolyBolt(float deltaTime) {
    // Holy Bolt: pulsing light effect
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

    // Render projectile (with rotation and scaling)
    float renderSize = size * scaleEffect;
    RenderImage(screenX, screenY, renderSize, renderSize, texture, 0, 1, 1);

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
    LoadTexture(device, "asset/Projectile_Fireball.png", &fireballTexture);
    LoadTexture(device, "asset/Projectile_IceShard.png", &iceShardTexture);
    LoadTexture(device, "asset/Projectile_MagicMissile.png", &magicMissileTexture);
    LoadTexture(device, "asset/Projectile_Lightning.png", &lightningTexture);
    LoadTexture(device, "asset/Projectile_PoisonDart.png", &poisonDartTexture);
    LoadTexture(device, "asset/Projectile_HolyBolt.png", &holyBoltTexture);

    // Set default texture (if loading fails)
    if (!fireballTexture) fireballTexture = g_enemyTexture;
    if (!iceShardTexture) iceShardTexture = g_enemyTexture;
    if (!magicMissileTexture) magicMissileTexture = fireballTexture;
    if (!lightningTexture) lightningTexture = fireballTexture;
    if (!poisonDartTexture) poisonDartTexture = fireballTexture;
    if (!holyBoltTexture) holyBoltTexture = fireballTexture;
}

// Predefined projectile creation functions
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