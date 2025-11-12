#pragma once
#include "Game.h"


struct MapBlock;
// 方向枚举
enum Direction {
    DIR_RIGHT = 0,      // 0度
    DIR_UP_RIGHT,       // 45度
    DIR_UP,             // 90度
    DIR_UP_LEFT,        // 135度
    DIR_LEFT,           // 180度
    DIR_DOWN_LEFT,      // 225度
    DIR_DOWN,           // 270度
    DIR_DOWN_RIGHT      // 315度
};

// 敌人类声明
class Enemy {
public:
    Enemy(float x, float y, float hp = 100.0f);
    virtual ~Enemy() = default; // 虚析构函数

    // 设置伤害系数
    void SetDamageMultiplier(Direction dir, float multiplier);

    // 伤害处理
    float GetDamageMultiplier(float attackAngle);
    void TakeDamage(float damage, float attackAngle);

    // 状态更新（声明为虚函数）
    virtual void Update(float deltaTime);
    virtual void Render(ID3D11ShaderResourceView* texture);

    // 碰撞检测
    bool CheckPlayerCollision();
    bool CheckCollisionWithBlock(const MapBlock& block);

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

    // 声明为虚函数
    virtual void OnDeath();
    virtual void OnHit(float damage);
};

// 衍生敌人类：盾牌敌人
class ShieldEnemy : public Enemy {
public:
    ShieldEnemy(float x, float y);

protected:
    // 移除override关键字，或者确保基类函数是虚函数
    void OnHit(float damage); // 移除了override
    void OnDeath(); // 移除了override
};

// 衍生敌人类：法师敌人
class MageEnemy : public Enemy {
public:
    MageEnemy(float x, float y);

    // 声明为虚函数重写
    virtual void Update(float deltaTime) override;
    virtual void Render(ID3D11ShaderResourceView* texture) override;

private:
    float spellCooldown;
    float currentSpellCooldown;
    void CastSpell();
};

// 衍生敌人类：快速敌人
class FastEnemy : public Enemy {
public:
    FastEnemy(float x, float y);

    virtual void Update(float deltaTime) override;

private:
    float dashCooldown;
    float currentDashCooldown;
    void DashAttack();
};

// 敌人管理函数声明
void InitEnemies();
void CreateTestEnemies();
void UpdateEnemies(float deltaTime);
void RenderEnemies();
void CleanupEnemies();

// 全局敌人列表和纹理
extern std::vector<Enemy*> g_enemies;
extern ID3D11ShaderResourceView* g_enemyTexture;
extern ID3D11ShaderResourceView* g_shieldEnemyTexture;
extern ID3D11ShaderResourceView* g_mageEnemyTexture;
extern ID3D11ShaderResourceView* g_fastEnemyTexture;