#pragma once
// Projectile.h
#include "Enemy.h"
#include "Game.h"
#include "Map.h"
#include <vector>

class Enemy;
// Éäµ¯ÀàĞÍÃ¶¾Ù
enum class ProjectileType {
    FIREBALL,       // »ğÇE- Ö±Ïß·ÉĞĞ£¬Åö×²±¬Õ¨
    ICE_SHARD,      // ±ù¼ı - Ö±Ïß·ÉĞĞ£¬¼õËÙĞ§¹E
    MAGIC_MISSILE,  // Ä§·¨·Éµ¯ - ¸ú×ÙµĞÈË
    LIGHTNING,      // ÉÁµE- Ë²¼äÃEĞ
    POISON_DART,    // ¶¾E - ³ÖĞøÉËº¦
    HOLY_BOLT       // Ê¥¹â¼ı - ¶ÔÍöÁéÌØĞ§
};

// Éäµ¯Ğ§¹û½á¹¹
struct ProjectileEffect {
    float damage = 10.0f;
    float burnDamage = 0.0f;      // È¼ÉÕ³ÖĞøÉËº¦
    float slowEffect = 0.0f;      // ¼õËÙĞ§¹E(0-1)
    float stunDuration = 0.0f;    // Ñ£ÔÎÊ±¼E
    bool pierce = false;          // ÊÇ·ñ´©Í¸
    int maxPierceCount = 0;       // ×ûĞó´©Í¸ÊıÁ¿
    float areaRadius = 0.0f;      // ·¶Î§±¬Õ¨°E¶
};

// Éäµ¯ÀE
class Projectile {
public:
    Projectile(ProjectileType type, float startX, float startY,
        float targetX, float targetY, float speed,
        const ProjectileEffect& effect, bool fromPlayer = true);

    void Update(float deltaTime, MapManager* mapManager, std::vector<Enemy*>& enemies);
    void Render(const Camera& camera);
    bool IsActive() const { return isActive; }
    void Deactivate() { isActive = false; }

    // »ñÈ¡Éäµ¯ĞÅÏ¢
    float GetDamage() const { return effect.damage; }
    bool IsFromPlayer() const { return fromPlayer; }
    ProjectileType GetType() const { return type; }

private:
    // Éäµ¯ÊôĞÔ
    ProjectileType type;
    float posX, posY;
    float velocityX, velocityY;
    float speed;
    float lifeTime;
    float maxLifeTime;
    bool isActive;
    bool fromPlayer;

    // ÊÓ¾õĞ§¹E
    float size;
    float rotation;
    float scaleEffect;

    // Éäµ¯Ğ§¹E
    ProjectileEffect effect;

    // ¸ú×ÙÏà¹Ø
    Enemy* homingTarget;
    float homingStrength;
    int currentPierceCount;

    // ¸¨Öú·½·¨
    void Move(float deltaTime);
    bool CheckMapCollision(MapManager* mapManager);
    void CheckEnemyCollision(std::vector<Enemy*>& enemies);
    void ApplyEffectToEnemy(Enemy* enemy);
    void CreateImpactEffect();

    // ÀàĞÍÌØ¶¨ĞĞÎª
    void UpdateFireball(float deltaTime);
    void UpdateIceShard(float deltaTime);
    void UpdateMagicMissile(float deltaTime, std::vector<Enemy*>& enemies);
    void UpdateLightning(float deltaTime);
    void UpdatePoisonDart(float deltaTime);
    void UpdateHolyBolt(float deltaTime);
};

// Éäµ¯¹ÜÀúâ÷ÀE
class ProjectileManager {
public:
    static ProjectileManager& GetInstance();

    void AddProjectile(ProjectileType type, float startX, float startY,
        float targetX, float targetY, float speed,
        const ProjectileEffect& effect, bool fromPlayer = true);

    void Update(float deltaTime, MapManager* mapManager, std::vector<Enemy*>& enemies);
    void Render(const Camera& camera);
    void ClearAll();

    // ¹¤¾ßº¯Êı£º´´½¨Ô¤¶¨ÒåĞ§¹ûµÄÉäµ¯
    void CreateFireball(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreateIceShard(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreateMagicMissile(float startX, float startY, Enemy* target, bool fromPlayer = true);
    void CreateLightningStrike(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreatePoisonDart(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreateHolyBolt(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    ID3D11ShaderResourceView* GetTextureForType(ProjectileType type);
    void LoadTextures(ID3D11Device* device);

private:
    ProjectileManager() = default;
    std::vector<Projectile> projectiles;

    // Éäµ¯ÎÆÀE
    ID3D11ShaderResourceView* fireballTexture = nullptr;
    ID3D11ShaderResourceView* iceShardTexture = nullptr;
    ID3D11ShaderResourceView* magicMissileTexture = nullptr;
    ID3D11ShaderResourceView* lightningTexture = nullptr;
    ID3D11ShaderResourceView* poisonDartTexture = nullptr;
    ID3D11ShaderResourceView* holyBoltTexture = nullptr;
};