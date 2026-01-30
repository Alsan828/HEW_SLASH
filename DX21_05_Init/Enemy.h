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

bool CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2);

// for the direction of the projectiles when the enemy throws them after dying
struct ProjectileDirection 
{
    float x;
    float y;
};

// for 4 direction projectile
const ProjectileDirection FOUR_DIRECTIONS[4] = 
{
    {1.0f, 0.0f},    // right
    {-1.0f, 0.0f},   // left
    {0.0f, 1.0f},    // up
    {0.0f, -1.0f}    // down
};

// for 8 direction projectile
const ProjectileDirection EIGHT_DIRECTIONS[8] = 
{
    {1.0f, 0.0f},       // right
    {-1.0f, 0.0f},      // left
    {0.0f, 1.0f},       // up
    {0.0f, -1.0f},      // down
    {0.707f, 0.707f},   // up right
    {-0.707f, 0.707f},  // up left
    {0.707f, -0.707f},  // down right
    {-0.707f, -0.707f}  // down left
};

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

// AI状态枚举 - 只保留巡逻和追逐
enum AIState { PATROL, CHASE };

// 敌人类声明
class Enemy {
public:
    // this is for a test of the boss fight so I can change the HP of the boss for testing
    void SetHealth(float hp) {
        health = hp;
        //maxHealth = hp;
    }
    void SetMaxHealth(float hp) { 
        maxHealth = hp; 
    }


    Enemy(float x, float y, float hp = 10.0f);
    virtual ~Enemy() = default;
    
    // 设置伤害系数
    void SetDamageMultiplier(Direction dir, float multiplier);

    // 添加可见性检测方法
    bool IsVisible(const Camera& camera) const {
        return camera.IsRectVisible(posX, posY, width, height);
    }

    // 获取离开屏幕的时间
    float GetOffScreenTime() const { return offScreenTimer; }

    // 重置离开屏幕计时器
    void ResetOffScreenTimer() { offScreenTimer = 0.0f; }

    // 检查敌人是否进入深度休眠（完全停止更新）
    bool IsFullySleeping(const Camera& camera) const {
        return !IsVisible(camera) && offScreenTimer > MAX_OFFSCREEN_TIME;
    }

    void UpdateMinimal(float deltaTime);
    // 检查是否需要最小更新（即使不在屏幕内）
    void UpdateAIMinimal(float deltaTime);
    bool NeedsMinimalUpdate() const {
        return offScreenTimer < MAX_OFFSCREEN_TIME ||
            currentState == CHASE || isHit || health < maxHealth;
    }
    // 伤害处理
    float GetDamageMultiplier(float attackAngle);
    virtual void TakeDamage(int damage, float attackAngle);

    // 状态更新
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr);
    virtual void Render(ID3D11ShaderResourceView* texture, const Camera& camera);
    void RenderHealthBar(const Camera& camera);

    // Health follower state per enemy (icons that smoothly follow the enemy)
    struct HealthFollower {
        float x = 0.0f;
        float y = 0.0f;
        bool init = false;
    };

    // Per-enemy followers (one per 10 HP segment of maxHealth)
    std::vector<HealthFollower> healthFollowers;

    // 攻击角度计算
    int CalculateDamageFromPlayer(int baseDamage, float playerDashAngle);

    // 获取敌人面向方向（true=右, false=左）
    bool IsFacingRight() const { return facingRight; }

    // 判断相对角度
    float GetRelativeAngle(float attackAngle) const;

    // 碰撞检测
    bool CheckPlayerCollision();
    bool CheckCollisionWithTiles(MapManager* mapManager);

    bool CheckCollisionWithTilesAt(float checkX, float checkY, MapManager* mapManager);
    // 获取属性
    float GetX() const { return posX; }
    float GetY() const { return posY; }
    float GetHealth() const { return health; }
    float GetMaxHealth() const { return maxHealth; }
    bool IsAlive() const { return isAlive; }
    float GetWidth() const { return width; }
    float GetHeight() const { return height; }
    bool IsMarkedForDeletion() const { return markedForDeletion; } // added december 22nd

    // Minimal setters (used by special behaviors like being thrown)
    void SetPosition(float x, float y) { posX = x; posY = y; }
    void SetVelocity(float vx, float vy) { velocityX = vx; velocityY = vy; }
    void SetFacingRight(bool right) { facingRight = right; }
    // Visual-only scaling (does not change collision box)
    void SetScale(float s) { scale = s; }

    // Tint for rendering (default white)
    void SetTint(float r, float g, float b) { tintR = r; tintG = g; tintB = b; }

    // Whether this enemy can deal damage to the player by simple contact (collision).
    // Default enemies can damage on contact; special types (e.g. boss) may override.
    virtual bool CanDamageOnContact() const { return true; }

    // Returns true if the enemy is currently performing an attack that should
    // be able to hurt the player (even if contact damage is normally disabled).
    virtual bool IsCurrentlyAttacking() const { return false; }

    Animation anim;  // 动画系统

    // Allow resetting runtime state for subclasses (no-op for base).
    virtual void ResetState() {}

protected:
    // turning/facing cooldown
    static constexpr float TURN_COOLDOWN_SECONDS = 1.0f;
    float turnCooldownTimer = 0.0f;
    bool useTurnCooldown = true;

    // AI行为方法
    virtual void PatrolBehavior(float deltaTime);
    virtual void ChaseBehavior(float deltaTime);

    // 工具函数
    float NormalizeAngle(float angle);
    int AngleToDirectionIndex(float angle);
    void UpdateAI(float deltaTime);
    void WorldToScreenPosition(float worldX, float worldY, float& screenX, float& screenY, const Camera& camera);

    // 水平碰撞检测
    bool CheckHorizontalCollision(MapManager* mapManager, float oldX, float oldY) {
        if (!mapManager || !mapManager->GetCurrentMap()) {
            return false;
        }

        SpatialGrid* grid = mapManager->GetCurrentMap()->GetSpatialGrid();
        if (!grid) {
            auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
            for (const auto& tile : solidTiles) {
                if (CheckCollision(posX, posY, width, height,
                    tile.posX, tile.posY, tile.width, tile.height)) {
                    return true;
                }
            }
            return false;
        }

        std::vector<MapTile*> nearbyTiles;
        grid->GetTilesInArea(
            posX - 1.0f,
            posY - 0.1f,
            width + 2.0f,
            height + 0.2f,
            nearbyTiles
        );

        for (const auto& tile : nearbyTiles) {
            if (tile->tileInfo.isSolid &&
                CheckCollision(posX, posY, width, height,
                    tile->posX, tile->posY, tile->width, tile->height)) {
                return true;
            }
        }
        return false;
    }

    // 垂直碰撞检测
    bool CheckVerticalCollision(MapManager* mapManager, float oldX, float oldY) {
        if (!mapManager || !mapManager->GetCurrentMap()) {
            return false;
        }

        SpatialGrid* grid = mapManager->GetCurrentMap()->GetSpatialGrid();
        if (!grid) {
            auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
            for (const auto& tile : solidTiles) {
                if (CheckCollision(posX, posY, width, height,
                    tile.posX, tile.posY, tile.width, tile.height)) {
                    // 检查是否站在地面上
                    return true;
                }
            }
            return false;
        }

        std::vector<MapTile*> nearbyTiles;
        grid->GetTilesInArea(
            posX - 0.1f,
            posY - 1.0f,
            width + 0.2f,
            height + 2.0f,
            nearbyTiles
        );

        for (const auto& tile : nearbyTiles) {
            if (tile->tileInfo.isSolid &&
                CheckCollision(posX, posY, width, height,
                    tile->posX, tile->posY, tile->width, tile->height)) {
                // 检查是否站在地面上
                return true;
            }
        }
        return false;
    }

    // 虚函数
    virtual void OnDeath();
    virtual void OnHit(int damage);

    // 碰撞检测辅助函数
    bool CheckCollisionWithTile(const MapTile& tile);
    // 基本属性
    float posX, posY;
    float width, height;
    float health;
    float maxHealth;
    float moveSpeed;
    // tint color
    float tintR = 1.0f;
    float tintG = 1.0f;
    float tintB = 1.0f;
    bool isAlive;
    bool markedForDeletion = false; // to see the animation after I kill the enemy
    bool wasVisible = false;  // 上次更新时是否可见
    float offScreenTimer = 0.0f;  // 离开屏幕的时间计时器
    static constexpr float MAX_OFFSCREEN_TIME = 5.0f;  // 最大离开屏幕时间
    bool isDying = false;  // 新增：是否正在播放死亡动画

    // 移动相关
    float velocityX;
    float velocityY;
    bool facingRight;  // true=右, false=左
    bool weakSpotDeath = false;

    // 伤害系统
    float damageMultipliers[8];

    float attackRange = 0.0f;  // 攻击范围，0表示近战
    // AI行为状态
    AIState currentState;
    float patrolMinX, patrolMaxX;

    // 巡逻相关参数
    float patrolDirection = 1.0f;
    float patrolTimer = 0.0f;

    // 检测范围
    float detectionRange = 3.0f;
    float loseSightRange = 5.0f;

    // 受击状态
    bool isHit = false;
    float hitTimer = 0.0f;
    const float HIT_DURATION = 0.01f;

    // default to 3.0, so I can change the scale of the enemy later if needed
    float scale = 3.0f;
};

// 飞行敌人类
class FlyEnemy : public Enemy {
public:
    FlyEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;

protected:
    virtual void PatrolBehavior(float deltaTime) override;
    virtual void ChaseBehavior(float deltaTime) override;
    virtual void OnHit(int damage) override;
    virtual void OnDeath() override;

private:
    float patrolAltitude;  // 巡逻高度
    float targetAltitude;  // 目标高度
    float altitudeChangeTimer = 0.0f;  // 高度变化计时器
    float altitudeChangeRate = 0.05f;  // 高度变化速度
};

// 法师敌人类
class MageEnemy : public Enemy {
public:
    MageEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;

protected:
    virtual void ChaseBehavior(float deltaTime) override;

private:
    void CastProjectile();  // 发射射弹

    // 射弹相关参数
    float spellCooldown = 3.0f;
    float currentSpellCooldown = 0.0f;
    float lastAttackTime = 0.0f;
    float attackCooldown = 1.5f;  // 攻击冷却时间
    float projectileSpeed = 2.0f;  // 射弹速度
    float projectileDamage = 20.0f;  // 射弹伤害
};

// 快速敌人类
class FastEnemy : public Enemy {
public:
    FastEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;

protected:
    virtual void ChaseBehavior(float deltaTime) override;

private:
    void DashAttack();  // 冲刺攻击

    float dashCooldown = 2.0f;
    float currentDashCooldown = 0.0f;
    float attackRange = 0.5f;  // 近战攻击范围
};

// 炸弹敌人类
class BombEnemy : public Enemy {
public:
    BombEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;
    virtual void TakeDamage(int damage, float attackAngle) override;

protected:
    virtual void ChaseBehavior(float deltaTime) override;
    virtual void OnDeath() override;

private:
    void Explode();  // 爆炸
    void CreateProjectiles();  // 创建射弹

    // 爆炸相关参数
    float pulseTimer = 0.0f;
    float baseSize = 1.0f;
    float explosionRadius = 1.5f;  // 爆炸半径
    float explosionDamage = 50.0f;  // 爆炸伤害
};

// for the boss enemy
class BossEnemy : public Enemy 
{
public:
    BossEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;
    virtual void TakeDamage(int damage, float attackAngle) override;
    virtual void Render(ID3D11ShaderResourceView* texture, const Camera& camera) override;

    // Boss overrides contact damage behavior: boss does not damage by simple contact.
    virtual bool CanDamageOnContact() const override;
    virtual bool IsCurrentlyAttacking() const override;

    // 调整技能速度的接口
    void SetDashSpeedMultiplier(float mul) { dashSpeedMultiplier = mul; }
    void SetSlashSpeed(float frameTimeSeconds) { slashFrameTime = frameTimeSeconds; }
    void SetChargeDuration(float seconds) { chargeDuration = seconds; }
    // Reset boss internal state to initial values (used when respawning at checkpoint)
    // Default implementation does nothing; bosses may override.
    virtual void ResetState() {}

protected:
    virtual void ChaseBehavior(float deltaTime) override;
    virtual void OnHit(int damage) override;
    virtual void OnDeath() override;

private:
    //write here anything related to the boss (phases, types of attacks if there are, attack cooldown, etc....)
    void SpecialAttack(); // special attack if there is one

    // Boss stats
    float specialAttackCooldown = 5.0f;
    float currentSpecialCooldown = 0.0f;
    int phase = 1;  // Boss phases

    // Spec-driven boss behavior
    enum BossState {
        BOSS_IDLE,
        BOSS_DASH_CHARGE,
        BOSS_DASH_MOVING,
        BOSS_DASH_AFTER,
        BOSS_LEAP_CHARGE,
        BOSS_LEAP_MOVING,
        BOSS_LEAP_AFTER,
        BOSS_SLASH_CHARGE,
        BOSS_SLASH_ACTIVE,
        BOSS_DOWN_BEFORE,
        BOSS_DOWN,
        BOSS_DOWN_AFTER
    };

    BossState bossState = BOSS_IDLE;
    float stateTimer = 0.0f;
    int hitsTaken = 0;               // total hits received
    bool inDownImmortal = false;     // cannot die during down
    int weakCycleIndex = 0;          // weakline direction cycle
    bool hasSpawnedSlashProjectiles = false; // slash barrage spawn guard

    // Facing lock during attack release
    bool facingLocked = false;
    bool fixedFacingRight = true;

    // Tunable timings
    float chargeDuration = 1.0f;     // even faster charge (2x faster than previous)
    float dashAfterDuration = 0.5f;  // faster recovery after dash
    float slashActiveFrames = 2.0f;  // 3 frames window
    float slashFrameTime = 0.045f;    // 斩击动画的每帧时间，影响斩击释放速度
    float downDuration = 3.0f;       // shorter down time

    // Dash tuning
    float dashSpeedMultiplier = 20.0f;   // even faster dash
    float dashMaxDuration = 6.0f;       // allow dash for longer time (approx half-map in many setups)
    float dashStopDistance = 0.1f;      // stop only when extremely close to player
    int dashLevel = 1;                  // dash level (affects speed)

    // Leap (jump + dash) tuning
    float leapChargeDuration = 6.0f;      // charge before leap
    float leapInitialVy = -6.0f;          // upward velocity (negative = up)
    float leapDashSpeedMultiplier = 1.0f; // faster horizontal dash while airborne
    float leapAirDuration = 1.2f;         // max air time for leap1

    // Helpers
    void EnterState(BossState s);
    void UpdateDashCharge(float dt);
    void UpdateDashMoving(float dt, MapManager* mapManager);
    void UpdateDashAfter(float dt);
    void UpdateLeapCharge(float dt);
    void UpdateLeapMoving(float dt, MapManager* mapManager);
    void UpdateLeapAfter(float dt);
    void UpdateSlashCharge(float dt);
    void UpdateSlashActive(float dt);
    void UpdateDownBefore(float dt);
    void UpdateDown(float dt);
    void UpdateDownAfter(float dt);
    void RecomputeWeakMultipliers();
};


// for the square enemy class
class SquareEnemy : public Enemy 
{
public:
    SquareEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;

protected:
    virtual void OnHit(int damage) override;
    virtual void OnDeath() override;

private:
    // Square enemy doesn't move, just stays in place
    float pulseTimer = 0.0f;  // Optional: for pulsing animation effect
};


// for the beam enemy
// todo: needs to be improved
class BeamEnemy : public Enemy {
public:
    BeamEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;
    virtual void TakeDamage(int damage, float attackAngle) override;

protected:
    virtual void OnDeath() override;

private:
    enum BeamState {
        BEAM_IDLE,
        BEAM_PRE_ATTACK,
        BEAM_ATTACKING,
        BEAM_POST_ATTACK
    };

    void CheckBeamDamage(); // damages the player
    void CreateDeathExplosion(); // Damages other enemies but not the player

    // Beam parameters - separate horizontal and vertical!
    float beamHitboxWidth = 0.1f;        // make it wider as you want
    float beamHorizontalLength = 0.8f;    // make it lager horiontally
    float beamVerticalLength = 0.86f;      // make it larger vertically
    float deathExplosionRadius = 0.75f;    
    float deathExplosionDamage = 100000.0f; // change this as you want

    // Attack state machine
    BeamState beamState = BEAM_IDLE;
    float attackCooldown = 3.0f;
    float currentCooldown = 0.0f;
    float preAttackDuration = 1.5f;
    float attackDuration = 1.0f;
    float postAttackDuration = 0.5f;
    float stateTimer = 0.0f;

    // Death animation tracking
    int deathAnimationPhase = 0;
    bool hasExploded = false;
    bool hasKilledPlayerThisAttack = false;  // Prevent multiple kills per attack

    float pulseTimer = 0.0f;
};

// 投掷者敌人：瞄准玩家，抛物线扔出一个基础敌人
class ThrowerEnemy : public Enemy {
public:
    ThrowerEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;

protected:
    virtual void PatrolBehavior(float deltaTime) override;
    virtual void ChaseBehavior(float deltaTime) override;

private:
    void TryThrow(MapManager* mapManager);
    bool CanThrow() const;

    float throwCooldown = 3.0f;
    float currentThrowCooldown = 0.0f;
    float throwRange = 6.0f;
    float throwFlyTime = 0.65f;
};

// 盲眼普通敌人：只会左右巡逻，遇到墙/悬崖掉头
class BlindEyeEnemy : public Enemy {
public:
    BlindEyeEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;

protected:
    virtual void PatrolBehavior(float deltaTime) override;
    virtual void ChaseBehavior(float deltaTime) override;

private:
    bool IsGroundAhead(MapManager* mapManager, float directionSign) const;
};



// 敌人管理函数声明
void InitEnemies();
void UpdateEnemies(float deltaTime, MapManager* mapManager = nullptr);
void RenderEnemies(const Camera& camera);
void CleanupEnemies();

// 全局敌人列表和纹理
extern std::vector<Enemy*> g_enemies;