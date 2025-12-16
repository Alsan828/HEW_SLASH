// Projectile.cpp
#include "Projectile.h"

// È«¾ÖÉäµ¯¹ÜÀúâ÷ÊµÀı
ProjectileManager& ProjectileManager::GetInstance() {
    static ProjectileManager instance;
    return instance;
}

// Projectile ÀàÊµÏÖ
Projectile::Projectile(ProjectileType type, float startX, float startY,
    float targetX, float targetY, float speed,
    const ProjectileEffect& effect, bool fromPlayer)
    : type(type), posX(startX), posY(startY), speed(speed), effect(effect),
    fromPlayer(fromPlayer), isActive(true), homingTarget(nullptr),
    currentPierceCount(0), rotation(0.0f), scaleEffect(1.0f) {

    // ¼ÆËã·½ÏòÏòÁ¿
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

    // ¸ù¾İÀàĞÍÉèÖÃ³õÊ¼ÊôĞÔ
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
        maxLifeTime = 0.5f; // ÉÁµç³ÖĞøÊ±¼äºÜ¶Ì
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

    // ¼EéÉúÃEÜÆÚ
    if (lifeTime >= maxLifeTime) {
        isActive = false;
        CreateImpactEffect();
        return;
    }

    // ÀàĞÍÌØ¶¨µÄ¸EÂÂß¼­
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

    // ÒÆ¶¯ºÍÅö×²¼EE
    Move(deltaTime);

    // ¼EéµØÍ¼Åö×²
    if (CheckMapCollision(mapManager)) {
        isActive = false;
        CreateImpactEffect();
        return;
    }

    // ¼EéµĞÈËÅö×²
    CheckEnemyCollision(enemies);
}

void Projectile::UpdateFireball(float deltaTime) {
    // »ğÇò£ºÖğ½¥±ä´ó²¢¼ÓËÙ
    scaleEffect = 1.0f + lifeTime * 0.5f;
    speed += deltaTime * 2.0f;
    rotation += deltaTime * 10.0f;
}

void Projectile::UpdateIceShard(float deltaTime) {
    // ±ù¼ı£ºĞı×ªĞ§¹E
    rotation += deltaTime * 15.0f;

    // ±ù¾§ÍÏÎ²Ğ§¹E
    if (fmod(lifeTime, 0.1f) < 0.05f) {
        // ¿ÉÒÔÔÚÕâÀEúØÓ±ù¾§Á£×ÓĞ§¹E
    }
}


void Projectile::UpdateMagicMissile(float deltaTime, std::vector<Enemy*>& enemies) {
    // Ä§·¨·Éµ¯£º¸ú×Ù×ûÙEÄµĞÈË
    if (homingTarget && !homingTarget->IsAlive()) {
        homingTarget = nullptr;
    }

    if (!homingTarget) {
        // Ñ°ÕÒ×ûÙEÄµĞÈË
        float closestDistance = 1000.0f;
        for (auto& enemy : enemies) {
            if (enemy->IsAlive()) {
                // Ê¹ÓÃGetº¯Êı»ñÈ¡µĞÈËÎ»ÖÃ
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
    // ¸ú×ÙÄ¿±E
    if (homingTarget && homingStrength > 0) {
        float dx = homingTarget->GetX() - posX;
        float dy = homingTarget->GetY() - posY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance > 0) {
            // Öğ²½µ÷Õû·½ÏE
            float targetVX = (dx / distance) * speed;
            float targetVY = (dy / distance) * speed;

            velocityX += (targetVX - velocityX) * homingStrength * deltaTime;
            velocityY += (targetVY - velocityY) * homingStrength * deltaTime;

            // ±E¼»¯ËÙ¶È
            float currentSpeed = sqrt(velocityX * velocityX + velocityY * velocityY);
            velocityX = (velocityX / currentSpeed) * speed;
            velocityY = (velocityY / currentSpeed) * speed;
        }
    }

    rotation += deltaTime * 8.0f;
}

void Projectile::UpdateLightning(float deltaTime) {
    // ÉÁµç£º¿EÙÉÁË¸Ğ§¹E
    scaleEffect = 0.8f + 0.4f * sin(lifeTime * 30.0f);
}

void Projectile::UpdatePoisonDart(float deltaTime) {
    // ¶¾E£ºÇáÎ¢ÕıÏÒ²¨ÒÆ¶¯
    float waveOffset = sin(lifeTime * 10.0f) * 0.02f;
    posX += waveOffset * deltaTime * 10.0f;
    rotation += deltaTime * 20.0f;
}

void Projectile::UpdateHolyBolt(float deltaTime) {
    // Ê¥¹â¼ı£ºÂö³å¹âĞ§
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
        // ¼òµ¥¾ØĞÎÅö×²¼EE
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

        // Ê¹ÓÃGetº¯Êı»ñÈ¡µĞÈËÊôĞÔ
        float enemyX = enemy->GetX();
        float enemyY = enemy->GetY();
        float enemyWidth = enemy->GetWidth();
        float enemyHeight = enemy->GetHeight();

        // Åö×²¼Eâ£¨Ê¹ÓÃGetº¯Êı»ñÈ¡µÄÖµ£©
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

    // ¼ÆËã¹¥»÷½Ç¶È£¨´ÓÉäµ¯µ½µĞÈËµÄ·½Ïò£©
    float dx = enemy->GetX() - posX;
    float dy = enemy->GetY() - posY;
    float attackAngle = atan2(dy, dx);

    // ÔEÉÉËº¦
    enemy->TakeDamage((int)effect.damage, attackAngle);

    // Ó¦ÓÃÌØÊâĞ§¹E
    // TODO: ÕâÀEÉÒÔÌúØÓÈ¼ÉÕ¡¢¼õËÙ¡¢Ñ£ÔÎµÈ×´Ì¬Ğ§¹E
    // ĞèÒªÎªEnemyÀàÌúØÓ×´Ì¬Ğ§¹ûÏµÍ³
}

void Projectile::CreateImpactEffect() {
    // TODO: ´´½¨Åö×²ÌØĞ§
    // ¿ÉÒÔÔÚÕâÀEúØÓÁ£×ÓĞ§¹û¡¢ÉùÒôĞ§¹ûµÈ
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

    // »ñÈ¡¶ÔÓ¦ÎÆÀE
    ID3D11ShaderResourceView* texture = ProjectileManager::GetInstance().GetTextureForType(type);
    if (!texture) return;

    // ×ª»»ÎªÆÁÄ»×ø±E
    float screenX, screenY;
    float cameraX = camera.GetX();
    float cameraY = camera.GetY();
    screenX = posX - cameraX;
    screenY = posY - cameraY;

    // ¸ù¾İÀàĞÍÉèÖÃÑÕÉ«
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

    // äÖÈ¾Éäµ¯£¨´øĞı×ªºÍËõ·Å£©
    float renderSize = size * scaleEffect;
    RenderImage(screenX, screenY, renderSize, renderSize, texture, 0, 1, 1);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

// ProjectileManager ÀàÊµÏÖ
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
    // ¼ÓÔØ¸÷ÖÖÉäµ¯ÎÆÀE
    LoadTexture(device, "asset/Projectile_Fireball.png", &fireballTexture);
    LoadTexture(device, "asset/Projectile_IceShard.png", &iceShardTexture);
    LoadTexture(device, "asset/Projectile_MagicMissile.png", &magicMissileTexture);
    LoadTexture(device, "asset/Projectile_Lightning.png", &lightningTexture);
    LoadTexture(device, "asset/Projectile_PoisonDart.png", &poisonDartTexture);
    LoadTexture(device, "asset/Projectile_HolyBolt.png", &holyBoltTexture);

    // ÉèÖÃÄ¬ÈÏÎÆÀú¿¨Èç¹û¼ÓÔØÊ§°Ü£©
    if (!fireballTexture) fireballTexture = g_enemyTexture;
    if (!iceShardTexture) iceShardTexture = g_enemyTexture;
    if (!magicMissileTexture) magicMissileTexture = fireballTexture;
    if (!lightningTexture) lightningTexture = fireballTexture;
    if (!poisonDartTexture) poisonDartTexture = fireballTexture;
    if (!holyBoltTexture) holyBoltTexture = fireballTexture;
}

// Ô¤¶¨ÒåÉäµ¯´´½¨º¯Êı
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
    effect.burnDamage = 3.0f; // ÕâÀE÷ÎªÖĞ¶¾³ÖĞøÉËº¦

    AddProjectile(ProjectileType::POISON_DART, startX, startY, targetX, targetY, 12.0f, effect, fromPlayer);
}

void ProjectileManager::CreateHolyBolt(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 30.0f;
    effect.pierce = true;
    effect.maxPierceCount = 3;

    AddProjectile(ProjectileType::HOLY_BOLT, startX, startY, targetX, targetY, 9.0f, effect, fromPlayer);
}