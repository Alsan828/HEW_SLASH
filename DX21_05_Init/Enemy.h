#pragma once
#include "Game.h"
#include "Render.h"
#include "Camera.h"
#include "Map.h"
#include <algorithm>
#include <cmath>
#include <vector>

// 前方宣言
struct Player;
class MapManager;

bool CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2);

// 敵が死亡時に射弾を放つときの方向定義
struct ProjectileDirection 
{
    float x;
    float y;
};

// 4 方向射弾用
const ProjectileDirection FOUR_DIRECTIONS[4] = 
{
    {1.0f, 0.0f},    // 右
    {-1.0f, 0.0f},   // 左
    {0.0f, 1.0f},    // 上
    {0.0f, -1.0f}    // 下
};

// 8 方向射弾用
const ProjectileDirection EIGHT_DIRECTIONS[8] = 
{
    {1.0f, 0.0f},       // 右
    {-1.0f, 0.0f},      // 左
    {0.0f, 1.0f},       // 上
    {0.0f, -1.0f},      // 下
    {0.707f, 0.707f},   // 右上
    {-0.707f, 0.707f},  // 左上
    {0.707f, -0.707f},  // 右下
    {-0.707f, -0.707f}  // 左下
};

// ダメージ数値構造体 - 敵本体とは独立
struct DamageNumber {
    float posX, posY;
    int value;
    float timer;
    float velocityY;
    bool isCritical;
    float lifeTime; // 総生存時間

    DamageNumber(float x, float y, int damage, bool critical = false)
        : posX(x), posY(y), value(damage), timer(0.0f), velocityY(0.5f),
        isCritical(critical), lifeTime(1.5f) {
    } // 1.5 秒間生存する
};

// 独立したダメージ数値管理器
class DamageNumberManager {
public:
    static void AddDamageNumber(float x, float y, int damage, bool isCritical = false);
    static void Update(float deltaTime);
    static void Render(const Camera& camera);
    static void Clear();

private:
    static std::vector<DamageNumber> damageNumbers;
};

// 方向列挙 - 前後上下の概念で扱う
enum Direction {
    DIR_FRONT = 0,      // 正面
    DIR_FRONT_UP,       // 前上
    DIR_UP,             // 上
    DIR_BACK_UP,        // 後上
    DIR_BACK,           // 背面
    DIR_BACK_DOWN,      // 後下
    DIR_DOWN,           // 下
    DIR_FRONT_DOWN      // 前下
};

// AI 状態列挙 - 巡回と追跡のみを残す
enum AIState { PATROL, CHASE };

// 敵クラス宣言
class Enemy {
public:
    // ボス戦テスト用に HP を変更できるようにしている
    void SetHealth(float hp) {
        health = hp;
        //maxHealth = hp;
    }
    void SetMaxHealth(float hp) { 
        maxHealth = hp; 
    }


    Enemy(float x, float y, float hp = 10.0f);
    virtual ~Enemy() = default;
    
    // ダメージ倍率を設定する
    void SetDamageMultiplier(Direction dir, float multiplier);

    // 可視判定メソッド
    bool IsVisible(const Camera& camera) const {
        return camera.IsRectVisible(posX, posY, width, height);
    }

    // 画面外に出ている時間を取得する
    float GetOffScreenTime() const { return offScreenTimer; }

    // 画面外タイマーをリセットする
    void ResetOffScreenTimer() { offScreenTimer = 0.0f; }

    // 敵が深い休眠状態に入ったかを確認する（完全に更新停止）
    bool IsFullySleeping(const Camera& camera) const {
        return !IsVisible(camera) && offScreenTimer > MAX_OFFSCREEN_TIME;
    }

    void UpdateMinimal(float deltaTime);
    // 画面外でも最小更新が必要かを確認する
    void UpdateAIMinimal(float deltaTime);
    bool NeedsMinimalUpdate() const {
        return offScreenTimer < MAX_OFFSCREEN_TIME ||
            currentState == CHASE || isHit || health < maxHealth;
    }
    // ダメージ処理
    float GetDamageMultiplier(float attackAngle);
    virtual void TakeDamage(int damage, float attackAngle);

    // 状態更新
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

    // 攻撃角度計算
    int CalculateDamageFromPlayer(int baseDamage, float playerDashAngle);

    // 敵の向きを取得する（true=右、false=左）
    bool IsFacingRight() const { return facingRight; }

    // 相対角度を判定する
    float GetRelativeAngle(float attackAngle) const;

    // 衝突判定
    bool CheckPlayerCollision();
    bool CheckCollisionWithTiles(MapManager* mapManager);

    bool CheckCollisionWithTilesAt(float checkX, float checkY, MapManager* mapManager);
    // プロパティ取得
    float GetX() const { return posX; }
    float GetY() const { return posY; }
    float GetHealth() const { return health; }
    float GetMaxHealth() const { return maxHealth; }
    bool IsAlive() const { return isAlive; }
    float GetWidth() const { return width; }
    float GetHeight() const { return height; }
    bool IsMarkedForDeletion() const { return markedForDeletion; } // 12 月 22 日追加

    // 最小限のセッター（投げられる等の特殊挙動で使用）
    void SetPosition(float x, float y) { posX = x; posY = y; }
    void SetVelocity(float vx, float vy) { velocityX = vx; velocityY = vy; }
    void SetFacingRight(bool right) { facingRight = right; }
    // 見た目のみの拡大縮小（衝突ボックスは変えない）
    void SetScale(float s) { scale = s; }

    // 描画用の色味（既定は白）
    void SetTint(float r, float g, float b) { tintR = r; tintG = g; tintB = b; }

    // 接触だけでプレイヤーへダメージを与えられるかどうか。
    // 通常敵は接触ダメージあり。特殊敵（例: ボス）は上書き可能。
    virtual bool CanDamageOnContact() const { return true; }

    // 現在の攻撃動作がプレイヤーにダメージを与えるべき状態なら true を返す。
    // （通常は接触ダメージが無効でも攻撃中なら有効にできる）
    virtual bool IsCurrentlyAttacking() const { return false; }

    Animation anim;  // アニメーションシステム

    // サブクラスが実行時状態をリセットできるようにする（基底は no-op）
    virtual void ResetState() {}

protected:
    // 向き変更クールダウン
    static constexpr float TURN_COOLDOWN_SECONDS = 1.0f;
    float turnCooldownTimer = 0.0f;
    bool useTurnCooldown = true;

    // AI 挙動メソッド
    virtual void PatrolBehavior(float deltaTime);
    virtual void ChaseBehavior(float deltaTime);

    // 補助関数
    float NormalizeAngle(float angle);
    int AngleToDirectionIndex(float angle);
    void UpdateAI(float deltaTime);
    void WorldToScreenPosition(float worldX, float worldY, float& screenX, float& screenY, const Camera& camera);

    // 水平衝突判定
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
                    // 地面に立っているかを確認する
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
                // 地面に立っているかを確認する
                return true;
            }
        }
        return false;
    }

    // 仮想関数
    virtual void OnDeath();
    virtual void OnHit(int damage);

    // 衝突判定補助関数
    bool CheckCollisionWithTile(const MapTile& tile);
    // 基本プロパティ
    float posX, posY;
    float width, height;
    float health;
    float maxHealth;
    float moveSpeed;
    // 色味
    float tintR = 1.0f;
    float tintG = 1.0f;
    float tintB = 1.0f;
    bool isAlive;
    bool markedForDeletion = false; // 倒した後もアニメーション表示のために保持する
    bool wasVisible = false;  // 前回更新時に可視だったか
    float offScreenTimer = 0.0f;  // 画面外にいる時間のタイマー
    static constexpr float MAX_OFFSCREEN_TIME = 5.0f;  // 最大画面外時間
    bool isDying = false;  // 追加: 死亡アニメーション再生中か

    // 移動関連
    float velocityX;
    float velocityY;
    bool facingRight;  // true=右、false=左
    bool weakSpotDeath = false;

    // ダメージシステム
    float damageMultipliers[8];

    float attackRange = 0.0f;  // 攻撃範囲。0 は近接を表す
    // AI 挙動状態
    AIState currentState;
    float patrolMinX, patrolMaxX;

    // 巡回関連パラメータ
    float patrolDirection = 1.0f;
    float patrolTimer = 0.0f;

    // 検知範囲
    float detectionRange = 3.0f;
    float loseSightRange = 5.0f;

    // 被弾状態
    bool isHit = false;
    float hitTimer = 0.0f;
    const float HIT_DURATION = 0.01f;

    // 既定値は 3.0。必要なら後で敵スケールを変えられるようにする
    float scale = 3.0f;
};

// 飛行敵クラス
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
    float patrolAltitude;  // 巡回高度
    float targetAltitude;  // 目標高度
    float altitudeChangeTimer = 0.0f;  // 高度変化タイマー
    float altitudeChangeRate = 0.05f;  // 高度変化速度
};

// 魔法敵クラス
class MageEnemy : public Enemy {
public:
    MageEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;

protected:
    virtual void ChaseBehavior(float deltaTime) override;

private:
    void CastProjectile();  // 射弾を放つ

    // 射弾関連パラメータ
    float spellCooldown = 3.0f;
    float currentSpellCooldown = 0.0f;
    float lastAttackTime = 0.0f;
    float attackCooldown = 1.5f;  // 攻撃クールダウン
    float projectileSpeed = 2.0f;  // 射弾速度
    float projectileDamage = 20.0f;  // 射弾ダメージ
};

// 高速敵クラス
class FastEnemy : public Enemy {
public:
    FastEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;

protected:
    virtual void ChaseBehavior(float deltaTime) override;

private:
    void DashAttack();  // ダッシュ攻撃

    float dashCooldown = 2.0f;
    float currentDashCooldown = 0.0f;
    float attackRange = 0.5f;  // 近接攻撃範囲
};

// 爆弾敵クラス
class BombEnemy : public Enemy {
public:
    BombEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;
    virtual void TakeDamage(int damage, float attackAngle) override;

protected:
    virtual void ChaseBehavior(float deltaTime) override;
    virtual void OnDeath() override;

private:
    void Explode();  // 爆発する
    void CreateProjectiles();  // 射弾を生成する

    // 爆発関連パラメータ
    float pulseTimer = 0.0f;
    float baseSize = 1.0f;
    float explosionRadius = 1.5f;  // 爆発半径
    float explosionDamage = 50.0f;  // 爆発ダメージ
};

// ボス敵クラス
class BossEnemy : public Enemy 
{
public:
    BossEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;
    virtual void TakeDamage(int damage, float attackAngle) override;
    virtual void Render(ID3D11ShaderResourceView* texture, const Camera& camera) override;

    // ボスは接触ダメージ挙動を上書きする: 単純接触ではダメージを与えない。
    virtual bool CanDamageOnContact() const override;
    virtual bool IsCurrentlyAttacking() const override;

    // スキル速度調整用インターフェース
    void SetDashSpeedMultiplier(float mul) { dashSpeedMultiplier = mul; }
    void SetSlashSpeed(float frameTimeSeconds) { slashFrameTime = frameTimeSeconds; }
    void SetChargeDuration(float seconds) { chargeDuration = seconds; }
    // ボス内部状態を初期値へ戻す（チェックポイント再出現時に使用）
    // 既定実装は何もしない。必要ならボス側で上書きする。
    virtual void ResetState() {}

protected:
    virtual void ChaseBehavior(float deltaTime) override;
    virtual void OnHit(int damage) override;
    virtual void OnDeath() override;

protected:
    // ボス関連の処理（フェーズ、攻撃種類、クールダウンなど）を書く
    void SpecialAttack(); // 特殊攻撃がある場合に使う

    // ボスステータス
    float specialAttackCooldown = 5.0f;
    float currentSpecialCooldown = 0.0f;
    int phase = 1;  // ボスフェーズ

    // 仕様ベースのボス挙動
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
    int hitsTaken = 0;               // 被弾回数の合計
    bool inDownImmortal = false;     // ダウン中は死亡しない
    int weakCycleIndex = 0;          // 弱点方向の循環インデックス
    bool hasSpawnedSlashProjectiles = false; // 斬撃連射の生成ガード

    // 攻撃発動中の向き固定
    bool facingLocked = false;
    bool fixedFacingRight = true;

    // 調整可能な時間設定
    float chargeDuration = 1.0f;     // さらに高速なチャージ（以前の 2 倍速）
    float dashAfterDuration = 0.5f;  // ダッシュ後の回復を速くする
    float slashActiveFrames = 2.0f;  // 3 フレーム相当の受付時間
    float slashFrameTime = 0.045f;    // 斬撃アニメーション 1 フレーム時間。発動速度に影響する
    float downDuration = 3.0f;       // 短めのダウン時間

    // 機械的な繰り返しを避けるためのランダム時間設定
    float timingVariance = 0.25f; // 既定で ±25%
    float randomizedIdleDuration = 1.0f;
    float randomizedChargeDuration = 1.0f;
    float randomizedDashMovingDuration = 1.0f;
    float randomizedDashAfterDuration = 0.5f;
    float randomizedDownDuration = 3.0f;

    // ダッシュ調整値
    float dashSpeedMultiplier = 20.0f;   // さらに高速なダッシュ
    float dashMaxDuration = 6.0f;       // 長めに許可する（多くの環境でマップ半分程度）
    float dashStopDistance = 0.1f;      // プレイヤーにかなり近いときだけ止まる
    int dashLevel = 1;                  // ダッシュレベル（速度へ影響）

    // Leap（ジャンプ + ダッシュ）調整値
    float leapChargeDuration = 6.0f;      // Leap 前のチャージ時間
    float leapInitialVy = -6.0f;          // 上向き速度（負値 = 上）
    float leapDashSpeedMultiplier = 1.0f; // 空中ダッシュ時の横速度倍率
    float leapAirDuration = 1.2f;         // Leap1 の最大空中時間

    // 補助メソッド
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

// 最終ボス敵クラス
class FinalBossEnemy : public BossEnemy 
{
public:
    FinalBossEnemy(float x, float y);
    virtual void Render(ID3D11ShaderResourceView* texture, const Camera& camera) override;
    virtual void ResetState() override;
};

// 四角敵クラス
class SquareEnemy : public Enemy 
{
public:
    SquareEnemy(float x, float y);
    virtual void Update(float deltaTime, MapManager* mapManager = nullptr) override;

protected:
    virtual void OnHit(int damage) override;
    virtual void OnDeath() override;

private:
    // 四角敵は移動せず、その場に留まる
    float pulseTimer = 0.0f;  // 任意: 脈動アニメーション効果用
};


// ビーム敵クラス
// todo: 改善が必要
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

    void CheckBeamDamage(); // プレイヤーへダメージを与える
    void CreateDeathExplosion(); // 他の敵にはダメージを与えるがプレイヤーには与えない

    // ビーム用パラメータ - 横と縦を分離する
    float beamHitboxWidth = 0.1f;        // 必要に応じて太くできる
    float beamHorizontalLength = 0.8f;    // 横方向の長さ
    float beamVerticalLength = 0.86f;      // 縦方向の長さ
    float deathExplosionRadius = 0.75f;    
    float deathExplosionDamage = 100000.0f; // 必要に応じて変更できる

    // 攻撃状態マシン
    BeamState beamState = BEAM_IDLE;
    float attackCooldown = 3.0f;
    float currentCooldown = 0.0f;
    float preAttackDuration = 1.5f;
    float attackDuration = 1.0f;
    float postAttackDuration = 0.5f;
    float stateTimer = 0.0f;

    // 死亡アニメーション管理
    int deathAnimationPhase = 0;
    bool hasExploded = false;
    bool hasKilledPlayerThisAttack = false;  // 1 回の攻撃で複数回倒さないようにする

    float pulseTimer = 0.0f;
};

// 投擲敵: プレイヤーを狙い、放物線で基本敵を投げる
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

// 盲目の通常敵: 左右巡回のみ行い、壁や崖で折り返す
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



// 敵管理関数宣言
void InitEnemies();
void UpdateEnemies(float deltaTime, MapManager* mapManager = nullptr);
void RenderEnemies(const Camera& camera);
void CleanupEnemies();

// グローバル敵リストとテクスチャ
extern std::vector<Enemy*> g_enemies;