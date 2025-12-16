#pragma once
// Projectile.h
#include "Enemy.h"
#include "Game.h"
#include "Map.h"
#include <vector>

class Enemy;
// 射弹类型枚举
enum class ProjectileType {
    FIREBALL,       // 火球 - 直线飞行，碰撞爆炸
    ICE_SHARD,      // 冰箭 - 直线飞行，减速效果
    MAGIC_MISSILE,  // 魔法飞弹 - 跟踪敌人
    LIGHTNING,      // 闪电 - 瞬间命中
    POISON_DART,    // 毒镖 - 持续伤害
    HOLY_BOLT       // 圣光箭 - 对亡灵特效
};

// 射弹效果结构
struct ProjectileEffect {
    float damage = 10.0f;
    float burnDamage = 0.0f;      // 燃烧持续伤害
    float slowEffect = 0.0f;      // 减速效果 (0-1)
    float stunDuration = 0.0f;    // 眩晕时间
    bool pierce = false;          // 是否穿透
    int maxPierceCount = 0;       // 最大穿透数量
    float areaRadius = 0.0f;      // 范围爆炸半径
};

// 射弹类
class Projectile {
public:
    Projectile(ProjectileType type, float startX, float startY,
        float targetX, float targetY, float speed,
        const ProjectileEffect& effect, bool fromPlayer = true);

    void Update(float deltaTime, MapManager* mapManager, std::vector<Enemy*>& enemies);
    void Render(const Camera& camera);
    bool IsActive() const { return isActive; }
    void Deactivate() { isActive = false; }

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
    float maxLifeTime;
    bool isActive;
    bool fromPlayer;

    // 视觉效果
    float size;
    float rotation;
    float scaleEffect;

    // 射弹效果
    ProjectileEffect effect;

    // 跟踪相关
    Enemy* homingTarget;
    float homingStrength;
    int currentPierceCount;

    // 辅助方法
    void Move(float deltaTime);
    bool CheckMapCollision(MapManager* mapManager);
    void CheckEnemyCollision(std::vector<Enemy*>& enemies);
    void ApplyEffectToEnemy(Enemy* enemy);
    void CreateImpactEffect();

    // 类型特定行为
    void UpdateFireball(float deltaTime);
    void UpdateIceShard(float deltaTime);
    void UpdateMagicMissile(float deltaTime, std::vector<Enemy*>& enemies);
    void UpdateLightning(float deltaTime);
    void UpdatePoisonDart(float deltaTime);
    void UpdateHolyBolt(float deltaTime);
};

// 射弹管理器类
class ProjectileManager {
public:
    static ProjectileManager& GetInstance();

    void AddProjectile(ProjectileType type, float startX, float startY,
        float targetX, float targetY, float speed,
        const ProjectileEffect& effect, bool fromPlayer = true);

    void Update(float deltaTime, MapManager* mapManager, std::vector<Enemy*>& enemies);
    void Render(const Camera& camera);
    void ClearAll();

    // 工具函数：创建预定义效果的射弹
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

    // 射弹纹理
    ID3D11ShaderResourceView* fireballTexture = nullptr;
    ID3D11ShaderResourceView* iceShardTexture = nullptr;
    ID3D11ShaderResourceView* magicMissileTexture = nullptr;
    ID3D11ShaderResourceView* lightningTexture = nullptr;
    ID3D11ShaderResourceView* poisonDartTexture = nullptr;
    ID3D11ShaderResourceView* holyBoltTexture = nullptr;
};