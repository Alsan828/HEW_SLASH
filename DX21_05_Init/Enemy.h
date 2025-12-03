#pragma once
#include "Game.h"
#include "Render.h"
#include "Camera.h"
#include "Map.h"
#include <algorithm>
#include <cmath>
#include <vector>

// 前向声明
struct Player;
class MapManager;

// 伤害数字结构 - 独立于敌人
struct DamageNumber {
    float posX, posY;
    int value;
    float timer;
    float velocityY;
    bool isCritical;
    float lifeTime; // 总生存时间

    DamageNumber(float x, float y, int damage, bool critical = false)
        : posX(x), posY(y), value(damage), timer(0.0f), velocityY(0.5f),
        isCritical(critical), lifeTime(1.5f) {
    } // 生存1.5秒
};

// 独立的伤害数字管理器
class DamageNumberManager {
public:
    static void AddDamageNumber(float x, float y, int damage, bool isCritical = false);
    static void Update(float deltaTime);
    static void Render(const Camera& camera);
    static void Clear();

private:
    static std::vector<DamageNumber> damageNumbers;
};

// 方向枚举 - 使用前后上下概念
enum Direction {
    DIR_FRONT = 0,      // 正面
    DIR_FRONT_UP,       // 前上
    DIR_UP,             // 上方
    DIR_BACK_UP,        // 后上
    DIR_BACK,           // 背面
    DIR_BACK_DOWN,      // 后下
    DIR_DOWN,           // 下方
    DIR_FRONT_DOWN      // 前下
};

// 敌人类声明
class Enemy {
    friend class Player;
public:
    Enemy(float x, float y, float hp = 100.0f);
    virtual ~Enemy() = default;

    // 设置伤害系数
    void SetDamageMultiplier(Direction dir, float multiplier);

    // 伤害处理
    float GetDamageMultiplier(float attackAngle);
    void TakeDamage(int damage, float attackAngle);

    // 状态更新
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr);
    virtual void Render(ID3D11ShaderResourceView* texture, const Camera& camera);
    void RenderHealthBar(const Camera& camera);

    // 攻击角度计算
    int CalculateDamageFromPlayer(int baseDamage, float playerDashAngle);

    // 获取敌人面向方向（true=右, false=左）
    bool IsFacingRight() const { return facingRight; }

    // 判断相对角度
    float GetRelativeAngle(float attackAngle) const;

    // 碰撞检测
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
    bool IsSplit() const { return isSplit; }
    void SetSplit(bool split) { isSplit = split; }
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
    bool facingRight;  // true=右, false=左

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
    void WorldToScreenPosition(float worldX, float worldY, float& screenX, float& screenY, const Camera& camera);

    // AI行为方法
    void PatrolBehavior(float deltaTime);
    void ChaseBehavior(float deltaTime);
    void AttackBehavior(float deltaTime);
    void FleeBehavior(float deltaTime);

    // 虚函数
    virtual void OnDeath();
    virtual void OnHit(int damage);

    // 碰撞检测辅助函数
    bool CheckCollisionWithTile(const MapTile& tile);

    // 受击状态
    bool isHit = false;
    float hitTimer = 0.0f;
    const float HIT_DURATION = 0.01f;

    // 删除原有的伤害数字相关静态成员
    void DrawSplitDeathEffect(ID3D11ShaderResourceView* texture, const Camera& camera);
    // 绘制分割的图像（辅助方法）
    void RenderSplitImage(float posX, float posY, float width, float height,
        ID3D11ShaderResourceView* textureSRV,
        float u0, float v0, float u1, float v1, float rotation);

    bool isDeathAnimating = false;
    float deathTimer = 0.0f;
    float deathDuration = 0.5f;
    float splitOffset = 0.0f;  // 分裂部分的偏移距离
    float alpha = 1.0f;        // 透明度

    // 新增分裂特效参数
    float splitAngle1 = 0.0f;  // 第一部分旋转角度
    float splitAngle2 = 0.0f;  // 第二部分旋转角度
    float rotateSpeed = 3.14f; // 旋转速度（弧度/秒）
    bool isSplit = false;      // 是否进入分裂状态

};
// 衍生敌人类
class ShieldEnemy : public Enemy {
public:
    ShieldEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;

protected:
    virtual void OnHit(int damage) override;
    virtual void OnDeath() override;
};

class MageEnemy : public Enemy {
public:
    MageEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;
    virtual void Render(ID3D11ShaderResourceView* texture, const Camera& camera);

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

// 在MageEnemy类声明后添加BombEnemy类
class BombEnemy : public Enemy {
public:
    BombEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;
    virtual void Render(ID3D11ShaderResourceView* texture, const Camera& camera) override;

private:

    void Explode(); // 死亡时爆炸的空函数

    // 视觉特效相关
    float pulseTimer;
    float baseSize;
};


// 敌人管理函数声明
void InitEnemies();
void UpdateEnemies(float deltaTime, MapManager* mapManager = nullptr);

void RenderEnemies(const Camera& camera);
void CleanupEnemies();

// 全局敌人列表和纹理
extern std::vector<Enemy*> g_enemies;
// 在全局纹理声明中添加爆炸敌人纹理
extern ID3D11ShaderResourceView* g_bombEnemyTexture;
extern ID3D11ShaderResourceView* g_enemyTexture;
extern ID3D11ShaderResourceView* g_shieldEnemyTexture;
extern ID3D11ShaderResourceView* g_mageEnemyTexture;
extern ID3D11ShaderResourceView* g_fastEnemyTexture;