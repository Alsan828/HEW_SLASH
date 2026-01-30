#pragma once
// Projectile.h
#include "Enemy.h"
#include "Game.h"
#include "Map.h"
#include <vector>

class Enemy;
// 射弹类型枚举
enum class ProjectileType {
    FIREBALL,       // 火莵E- 直线飞行，碰撞爆炸
    ICE_SHARD,      // 冰箭 - 直线飞行，减速效箒E
    MAGIC_MISSILE,  // 魔法飞弹 - 跟踪敌人
    LIGHTNING,      // 闪祦E- 瞬间脕E?
    POISON_DART,    // 毒丒 - 持续伤害
    HOLY_BOLT,       // 圣光箭 - 对亡灵特效
    BULLET
};

// 射弹效果结构
struct ProjectileEffect {
    float damage = 10.0f;
    float burnDamage = 0.0f;      // 燃烧持续伤害
    float slowEffect = 0.0f;      // 减速效箒E(0-1)
    float stunDuration = 0.0f;    // 眩晕时紒E
    bool pierce = false;          // 是否穿透
    int maxPierceCount = 0;       // 畜穿透数量
    float areaRadius = 0.0f;      // 范围爆炸皝E?
};

// 射弹纴E
class Projectile {
public:
    Projectile(ProjectileType type, float startX, float startY,
        float targetX, float targetY, float speed,
        const ProjectileEffect& effect, bool fromPlayer = true);

    void Update(float deltaTime, MapManager* mapManager, std::vector<Enemy*>& enemies);
    void Render(const Camera& camera);
    bool IsActive() const { return isActive; }
    void Deactivate() { isActive = false; }

    // Check collision with an arbitrary rectangle (used by player slash to hit projectiles)
    bool CheckCollisionWithRect(float rectX, float rectY, float rectW, float rectH) const;
    // Called when a player successfully hits this projectile
    void OnHitByPlayer();
    // Returns true if this projectile is hostile (not from player) and is aimed at the player
    bool IsHostileAndAimedAtPlayer() const;

    // 获取射弹信息
    float GetDamage() const { return effect.damage; }
    bool IsFromPlayer() const { return fromPlayer; }
    ProjectileType GetType() const { return type; }

private:
    // 射弹属性
    ProjectileType type;
    float posX, posY;
    float velocityX, velocityY;
    float speed;
    float lifeTime;
    float maxLifeTime = 5.0f;
    bool isActive;
    bool fromPlayer;

    // 视觉 
    float size = 0.5f;
    float rotation;
    float scaleEffect;

    // 射弹效箒E
    ProjectileEffect effect;

    // 跟踪相关
    Enemy* homingTarget;
    float homingStrength = 0.0f;
    int currentPierceCount;

    // 辅助方法
    void Move(float deltaTime);
    bool CheckMapCollision(MapManager* mapManager);
    void CheckEnemyCollision(std::vector<Enemy*>& enemies);
    void CheckPlayerCollision();
    void ApplyEffectToEnemy(Enemy* enemy);
    void CreateImpactEffect();

    float CalculateDirectionAngle()const;
    float GetRotationAngle()const;
    void SetRotation(float r);
    // 类型特定行为
    void UpdateFireball(float deltaTime);
    void UpdateBullet(float deltaTime);
    void UpdateIceShard(float deltaTime);
    void UpdateMagicMissile(float deltaTime, std::vector<Enemy*>& enemies);
    void UpdateLightning(float deltaTime);
    void UpdatePoisonDart(float deltaTime);
    void UpdateHolyBolt(float deltaTime);
};

// 射弹管历怊纴E
class ProjectileManager {
public:
    static ProjectileManager& GetInstance();

    void AddProjectile(ProjectileType type, float startX, float startY,
        float targetX, float targetY, float speed,
        const ProjectileEffect& effect, bool fromPlayer = true);

    void Update(float deltaTime, MapManager* mapManager, std::vector<Enemy*>& enemies);
    void Render(const Camera& camera);
    void ClearAll();

    // Called when player slash should try to hit enemy projectiles
    void HandlePlayerSlashHitRect(float rectX, float rectY, float rectW, float rectH);
    void HandlePlayerSlashHitCircle(float centerX, float centerY, float radius);

    // 工具函数：创建预定义效果的射弹
    void CreateFireball(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreateIceShard(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreateMagicMissile(float startX, float startY, Enemy* target, bool fromPlayer = true);
    void CreateLightningStrike(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreatePoisonDart(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreateHolyBolt(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreateBullet(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    ID3D11ShaderResourceView* GetTextureForType(ProjectileType type);
    void LoadTextures(ID3D11Device* device);

private:
    ProjectileManager() = default;
    std::vector<Projectile> projectiles;

    // 射弹纹纴E
    ID3D11ShaderResourceView* fireballTexture = nullptr;
    ID3D11ShaderResourceView* bulletTexture = nullptr;
    ID3D11ShaderResourceView* iceShardTexture = nullptr;
    ID3D11ShaderResourceView* magicMissileTexture = nullptr;
    ID3D11ShaderResourceView* lightningTexture = nullptr;
    ID3D11ShaderResourceView* poisonDartTexture = nullptr;
    ID3D11ShaderResourceView* holyBoltTexture = nullptr;
};