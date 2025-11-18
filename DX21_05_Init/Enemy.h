#pragma once
#include "Game.h"
#include "Render.h"
#include "Map.h"  // 包含地图头文件
#include <algorithm>
#include <cmath>

// 前向声明
struct Player;
class MapManager;

// 方向枚举
enum Direction {
    DIR_RIGHT = 0,
    DIR_UP_RIGHT,
    DIR_UP,
    DIR_UP_LEFT,
    DIR_LEFT,
    DIR_DOWN_LEFT,
    DIR_DOWN,
    DIR_DOWN_RIGHT
};

// 敌人类声明
class Enemy {
public:
    Enemy(float x, float y, float hp = 100.0f);
    virtual ~Enemy() = default;

    // 设置伤害系数
    void SetDamageMultiplier(Direction dir, float multiplier);

    // 伤害处理
    float GetDamageMultiplier(float attackAngle);
    void TakeDamage(float damage, float attackAngle);

    // 状态更新
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr);
    virtual void Render(ID3D11ShaderResourceView* texture);

    // 碰撞检测 - 使用新的地图系统
    bool CheckPlayerCollision();
    bool CheckCollisionWithTiles(const std::vector<MapTile>& solidTiles);

    // 获取属性
    float GetX() const { return posX; }
    float GetY() const { return posY; }
    float GetHealth() const { return health; }
    float GetMaxHealth() const { return maxHealth; }
    bool IsAlive() const { return isAlive; }
    float GetWidth() const { return width; }
    float GetHeight() const { return height; }

protected:
    // 基本属性
    float posX, posY;
    float width, height;
    float health;
    float maxHealth;
    float moveSpeed;
    bool isAlive;

    // 移动相关
    float velocityX;
    float velocityY;
    float facingAngle;

    // 伤害系统
    float damageMultipliers[8];

    // AI行为状态
    enum AIState { PATROL, CHASE, ATTACK, FLEE };
    AIState currentState;
    float patrolMinX, patrolMaxX;
    float attackRange;

    // 工具函数
    float NormalizeAngle(float angle);
    int AngleToDirectionIndex(float angle);
    void UpdateAI(float deltaTime);
    void RenderHealthBar();

    // AI行为方法
    void PatrolBehavior(float deltaTime);
    void ChaseBehavior(float deltaTime);
    void AttackBehavior(float deltaTime);
    void FleeBehavior(float deltaTime);

    // 虚函数
    virtual void OnDeath();
    virtual void OnHit(float damage);

    // 新的碰撞检测辅助函数
    bool CheckCollisionWithTile(const MapTile& tile);
};

// 衍生敌人类
class ShieldEnemy : public Enemy {
public:
    ShieldEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;

protected:
    virtual void OnHit(float damage) override;
    virtual void OnDeath() override;
};

class MageEnemy : public Enemy {
public:
    MageEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;
    virtual void Render(ID3D11ShaderResourceView* texture) override;

private:
    float spellCooldown;
    float currentSpellCooldown;
    void CastSpell();
};

class FastEnemy : public Enemy {
public:
    FastEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;

private:
    float dashCooldown;
    float currentDashCooldown;
    void DashAttack();
};

// 敌人管理函数声明
void InitEnemies();
void UpdateEnemies(float deltaTime, MapManager* mapManager = nullptr);
void RenderEnemies();
void CleanupEnemies();

// 全局敌人列表和纹理
extern std::vector<Enemy*> g_enemies;
extern ID3D11ShaderResourceView* g_enemyTexture;
extern ID3D11ShaderResourceView* g_shieldEnemyTexture;
extern ID3D11ShaderResourceView* g_mageEnemyTexture;
extern ID3D11ShaderResourceView* g_fastEnemyTexture;