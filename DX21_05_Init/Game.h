#pragma once
#define NOMINMAX

#include <cstdint>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <string>
#include "InputSystem.h"
#include "Render.h"
#include "Map.h"
#include "Camera.h"
#include "Animation.h"
#include "SceneManager.h"
#include "SceneBase.h"
#include "Pause.h"
#include "Projectile.h"
#include "Enemy.h"
#include "Texture1.h"
#include "Audio.h"

class ProjectileManager;

extern ProjectileManager& g_projectileManager;  // グローバル射弾管理器
extern SceneManager sceneManager;
extern MapManager g_mapManager;
extern InputSystem g_inputSystem;
extern Camera g_camera;
class Enemy;

// Slash-count UI（1x4 スプライトシート）
extern ID3D11ShaderResourceView* g_slashCountTexture;
extern Animation g_slashCountAnim;
// 追従型体力アイコンのテクスチャ / アニメーション（1x3 スプライトシート）
extern ID3D11ShaderResourceView* g_healthTexture;
extern Animation g_healthAnim;

// 弱点ヒット / 撃破エフェクト生成（Game.cpp で定義）
void SpawnWeakPointHitEffect(float worldX, float worldY);
void SpawnWeakPointHitEffectScaled(float worldX, float worldY, float scale);
void SpawnWeakPointKillEffect(float worldX, float worldY);

// 敵撃破でダッシュポイントが回復したとき、そのワールド座標から追従インジケータを出す。
extern float g_slashCountSpawnX;
extern float g_slashCountSpawnY;
extern bool g_slashCountSpawnPending;

// ゲーム状態列挙
enum GameState {
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_PAUSED
};

// グローバル定数定義
const float GRID_WIDTH = 0.0625f;
const float GRID_HEIGHT = 0.085f;
const float PLAYER_WIDTH = 0.08f;
const float PLAYER_HEIGHT = 0.12f;
const float GRAVITY = -0.002f;
const float JUMP_FORCE = 0.045f;
// 基本移動速度（以前より遅い）。旧値は 0.01f。
// 通常状態の移動速度は以前の 0.8 倍。
const float MOVE_SPEED = 0.008f; 
const float DASH_SPEED = 0.05f;      // 基本ダッシュ速度（半減）
const float DASH_DURATION = 0.16f;   // 基本ダッシュ継続時間
const float DASH_COOLDOWN = 0.0f;    // ダッシュクールダウン時間

// 加速（コンボ）状態
const int ACCEL_COMBO_THRESHOLD = 5;        // comboCount > 5 で加速状態へ入る
const float ACCEL_MOVE_SPEED_MULT = 1.75f;  // 加速時の移動速度倍率
const float ACCEL_ANIM_SPEED_MULT = 1.75f;  // アニメ速度も移動に同期させる
const float ACCEL_CHARGE_SPEED_MULT = 1.5f; // 加速時のチャージ速度倍率

// プレイヤー構造体
struct Player {
    float posX = 0.0f;
    float posY = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    bool isOnGround = false;
    bool isMoving = false;
    bool facingRight = true;
    // 死亡状態関連
    bool isDead = false;
    float deathTimer = 0.0f;
    const float DEATH_RESPAWN_TIME = 1.5f;  // 死亡後の再出現待機時間
    int deathCount = 0;  // 死亡回数（任意）

    // 攻撃中のプレイヤー用コンボ UI
    int comboCount = 0;
    float comboTimer = 0.0f;   // コンボがリセットされるまでの時間
    const float COMBO_RESET_TIME = 2.0f; // 2 秒撃破がなければコンボをリセット

    // 加速状態（コンボで発動）
    bool isAccelerated = false;

    // ゲージバーシステム用
    int gaugePoints = 0;              // 現在のゲージポイント
    const int MAX_GAUGE_POINTS = 12;  // 最大ゲージポイント
    bool isInvincible = false;        // 無敵状態
    float invincibleTimer = 0.0f;     // 無敵タイマー
    const float INVINCIBLE_DURATION = 5.0f;  // 無敵時間 5 秒

    // 無敵状態の由来制御:
    // true  -> フルゲージ / コンボ報酬由来の無敵。Invincible* アニメを使う
    // false -> ダッシュ / 斬撃の無敵窓由来。アニメセットは切り替えない
    bool isGaugeInvincible = false;
    float g_gaugeEffectTimer = 0.0f;  // ゲージエフェクトタイマー
    bool g_gaugeEffectActive = false;


    // 体力システム
    float health = 100.0f;
    float maxHealth = 100.0f;
    // 攻撃システム
    float attackDamage = 10.0f;  // 基本攻撃力
    bool isAttacking = false;    // 攻撃状態
    float attackTimer = 0.0f;
    const float ATTACK_DURATION = 0.2f;

    // ダッシュ関連変数
    bool isDashing = false;
    float dashTimer = 0.0f;
    float dashDirectionX = 0.0f;
    float dashDirectionY = 0.0f;
    int dashLevel = 0; 
    // Player 構造体へ追加した変数
    bool isWallSliding = false;
    int wallSlideDirection = 0; // 0=无, -1=左墙, 1=右墙
    float wallSlideTimer = 0.0f;
    const float WALL_SLIDE_SPEED = -0.02f;    // 墙壁滑行速度

    float mouseTargetX = 0.0f;
    float mouseTargetY = 0.0f;
    bool hasMouseTarget = false;
    bool allowMoveWhileCharging = true;

    // チャージダッシュ専用変数
    bool isCharging = false;
    float chargeTime = 0.0f;
    const float MAX_CHARGE_TIME = 1.0f;
    const float MIN_CHARGE_TIME = 0.01f;
    const float CHARGE_THRESHOLD_LOW = 0.2f;
    const float CHARGE_THRESHOLD_MID = 0.4f;
    const float CHARGE_THRESHOLD_HIGH = 0.8f;
    int hitStopTriggered = 0 ;     // 今回のダッシュで発生したヒットストップ回数
    float hitStopTimer = 0.0f;       // ヒットストップタイマー

    // 蓄力段階システム
    float savedChargeTime = 0.0f;        // 保存した蓄力時間
    bool hasSavedCharge = false;         // 蓄力を保存しているか
    float chargeDecayTimer = 0.0f;        // 蓄力減衰タイマー
    const float CHARGE_DECAY_TIME = 1.0f; // 蓄力保持時間

    Animation anim;
    float animLockTimer = 0.0f; // アニメ切り替え時に使う
    float animLockDuration = 0.25f;

    // ダッシュポイントシステム
    int dashPoints = 3;
    const int MAX_DASH_POINTS = 3;
    float dashPointRecoverTimer = 0.0f;
    const float DASH_POINT_RECOVER_TIME = 0.1f;

    // 一方向プラットフォーム下降猶予: 一度発動後、短時間衝突を無視する
    float oneWayPlatformDropTimer = 0.0f;
    const float ONE_WAY_PLATFORM_DROP_GRACE = 0.18f;

    // 今回のダッシュで消費したポイント（ダメージ加算計算用）
    int lastDashConsumedPoints = 1;

    // 蓄力消費ポイント（蓄力中に仮引きし、発動時に確定する）
    int chargePendingCost = 0;            // 現在の蓄力で累計した消費ポイント（0~3）
    float chargeCostTimer = 0.0f;         // 時間ごとの消費蓄積用タイマー
    const float CHARGE_COST_INTERVAL = 0.25f; // 何秒ごとに 1 ポイント消費を増やすか
    bool isChargeCostHighlight = false;   // UI ハイライト: 蓄力時の消費フィードバック表示

    // ダッシュ後の硬直状態
    bool isInDashAftermath = false;
    float dashAftermathTimer = 0.0f;
    const float DASH_AFTERMATH_DURATION = 0.7f;

    // ダッシュ終了時のスローモーション（実時間）。継続時間はダッシュ後無敵時間も兼ねる。
    bool isInDashEndSlowMo = false;
    float dashEndSlowMoTimer = 0.0f;
    // 1/3 短縮: 0.75s -> 0.5s
    const float DASH_END_SLOWMO_REALTIME = 0.5f;
    const float DASH_END_SLOWMO_FACTOR = 0.35f;
    
    const float AFTERIMAGE_DURATION = 0.35f;

     // 残像パラメータ
     float afterImageSpawnTimer = 0.0f;
     float afterImageSpawnInterval = 0.025f;
     float afterImageMinSpeed = 0.06f;

    float GetMoveSpeedMultiplier() const {
        return isAccelerated ? ACCEL_MOVE_SPEED_MULT : 1.0f;
    }

    float GetAnimSpeedMultiplier() const {
        return isAccelerated ? ACCEL_ANIM_SPEED_MULT : 1.0f;
    }

    float GetChargeSpeedMultiplier() const {
        return isAccelerated ? ACCEL_CHARGE_SPEED_MULT : 1.0f;
    }

    // 追加: 攻撃判定関連
    std::vector<Enemy*> hitEnemies; // 今回のダッシュで当たった敵（重複ダメージ防止）

    // 攻撃状態をリセットする
    void ResetAttack() {
        isAttacking = false;
        attackTimer = 0.0f;
        hitEnemies.clear();
    }

    // ダメージを受ける
    void TakeDamage(float damage) {
        if (isDead) return;

        health -= damage;
        if (health <= 0) {
            health = 0;
            isDead = false;
            // プレイヤー死亡処理
        }
    }

    // 現在の蓄力レベルを取得する
    int GetChargeLevel() const {
        if (chargeTime >= CHARGE_THRESHOLD_HIGH) return 3;
        if (chargeTime >= CHARGE_THRESHOLD_MID) return 2;
        if (chargeTime >= CHARGE_THRESHOLD_LOW) return 1;
        return 0;
    }

    // 現在の蓄力を保存する
    void SaveCharge() {
        if (chargeTime > MIN_CHARGE_TIME) {
            savedChargeTime = chargeTime;
            hasSavedCharge = true;
            chargeDecayTimer = CHARGE_DECAY_TIME;
        }
    }

    // 保存済み蓄力を読み込む
    void LoadSavedCharge() {
        if (hasSavedCharge) {
            chargeTime = savedChargeTime;
            chargeDecayTimer = CHARGE_DECAY_TIME; // 減衰タイマーをリセットする
        }
    }

    // 保存済み蓄力を消去する
    void ClearSavedCharge() {
        hasSavedCharge = false;
        savedChargeTime = 0.0f;
        chargeDecayTimer = 0.0f;
    }

    // 蓄力減衰を更新する
    void UpdateChargeDecay(float deltaTime) {
        if (hasSavedCharge) {
            chargeDecayTimer -= deltaTime;
            if (chargeDecayTimer <= 0.0f) {
                ClearSavedCharge(); // 時間切れで保存蓄力を消去する
            }
        }
    }
    // 時間から蓄力レベルを取得する
    int GetChargeLevelFromTime(float chargeTime) const {
        if (chargeTime >= CHARGE_THRESHOLD_HIGH) return 3;
        if (chargeTime >= CHARGE_THRESHOLD_MID) return 2;
        if (chargeTime >= CHARGE_THRESHOLD_LOW) return 1;
        return 0;
    }
};


// ゲーム統計用（死亡数、撃破数など）
class GameStatistics {
private:
    int enemiesKilled = 0;
    int weakPointKills = 0;
    int totalDeaths = 0;
    float totalTime = 0.0f;
    int totalScore = 0;
    int penalizableDeaths = 0;  // deaths happens when I have points

    int maxCombo = 0;           // Track maximum combo reached
    int currentMaxCombo = 0;
    int currentAreaEnemyPoints = 0;  // Points from current area
    int totalEnemyPoints = 0;   // Total accumulated enemy points

    // resets when the player dies
    int currentScore = 0;                
    int currentKills = 0;
    int currentWeakKills = 0;

    // used for the final result
    int lifetimeEnemyPoints = 0;  
    int lifetimeKills = 0;        
    int lifetimeWeakKills = 0;

public:
   // no need construct now bc I initialed the variables above

    // Reset all statistics
    void Reset();

    // Increment counters
    void IncrementKills();
    void IncrementWeakPointKills();
    void IncrementDeaths();

    // Update time
    void UpdateTime(float ime);

    // Calculate final score
    void CalculateFinalScore();

    void AddScore(int points);

    void IncrementPenalizableDeaths() { penalizableDeaths++; }
    int GetPenalizableDeaths() const { return penalizableDeaths; }

    // Getters (read-only access to private data)
    int GetEnemiesKilled() const { return enemiesKilled; }
    int GetWeakPointKills() const { return weakPointKills; }
    int GetTotalDeaths() const { return totalDeaths; }
    float GetTotalTime() const { return totalTime; }
    int GetTotalScore() const { return totalScore; }
    int GetLifetimeEnemyPoints() const { return lifetimeEnemyPoints; }
    int GetLifetimeKills() const { return lifetimeKills; }
    int GetLifetimeWeakKills() const { return lifetimeWeakKills; }

    void UpdateMaxCombo(int combo);
    void ResetAreaProgress();  // Call when player dies
    int GetMaxCombo() const { return maxCombo; }
    int GetCurrentAreaEnemyPoints() const { return currentAreaEnemyPoints; }
    int GetTotalEnemyPoints() const { return totalEnemyPoints; }
    void AddEnemyPoints(int points);  // Add points for killing enemies

    void ResetCurrentStats(); // to reset the dtats when I die
};
extern GameStatistics g_gameStats;

struct HitEffectInstance {
    float x = 0.0f;
    float y = 0.0f;
    float scale = 1.0f;
    float timer = 0.0f;
    float frameTime = 0.04f;
    int frame = 0;
    bool active = false;
    int rows = 1;
    int columns = 1;
    int frameCount = 1;
    ID3D11ShaderResourceView* texture = nullptr;
};

void SpawnWeakPointHitEffect(float worldX, float worldY);
void SpawnGaugeKillParticlesRed(float worldX, float worldY);



class GameTimer {
private:
    __int64 m_prevTime = 0;
    __int64 m_startTime = 0;
    __int64 m_currTime = 0;
    double m_secondsPerCount = 0.0;
    float m_deltaTime = 0.0f;
    double m_totalTime = 0.0f;       // 总游戏时间

public:
    GameTimer();
    void Tick();
    float GetDeltaTime() const;

    double GetTotalTime() const {
        return m_totalTime;
    }
};

extern int g_windowWidth;
extern int g_windowHeight;

extern Player g_player;
extern ProjectileManager& g_projectileManager;

// Gameplay toggles (runtime)
extern bool g_releaseDashChargeMode;      // VK_T: dash executes on mouse release; short hold chains saved charge
extern bool g_noGravityAftermathMode;     // VK_G: dash aftermath ignores gravity; movement input breaks aftermath
extern ID3D11ShaderResourceView* g_playerTexture;

// for the characteer
extern ID3D11ShaderResourceView* g_playerIdleTexture;    // 通用站立纹理
extern ID3D11ShaderResourceView* g_playerDeathTexture;   // for when the character dies
extern ID3D11ShaderResourceView* g_playerJumpTexture;   // 通用跳跃纹理
extern ID3D11ShaderResourceView* g_playerRunTexture;     // 通用奔跑纹理
extern ID3D11ShaderResourceView* g_playerSlash1Texture; // 通用斩击1纹理
extern ID3D11ShaderResourceView* g_playerSlash2Texture; // 通用斩击2纹理
extern ID3D11ShaderResourceView* g_playerSlash3Texture; // 通用斩击3纹理
extern ID3D11ShaderResourceView* g_playerSlash4Texture; // 通用斩击4纹理
extern ID3D11ShaderResourceView* g_playerAirChargeTexture; // 通用空中蓄力纹理
extern ID3D11ShaderResourceView* g_playerFallingTexture;  // 通用下落纹理
extern ID3D11ShaderResourceView* g_playerGroundChargeTexture; // 通用地面蓄力纹理
extern ID3D11ShaderResourceView* g_playerWallSlideTexture; // 通用地面蓄力纹理
// for the character when invincible
extern ID3D11ShaderResourceView* g_invinciblePlayerIdleTexture;    // 通用站立纹理
extern ID3D11ShaderResourceView* g_invinciblePlayerJumpTexture;   // 通用跳跃纹理
extern ID3D11ShaderResourceView* g_invinciblePlayerRunTexture;     // 通用奔跑纹理
extern ID3D11ShaderResourceView* g_invinciblePlayerSlash1Texture; // 通用斩击1纹理
extern ID3D11ShaderResourceView* g_invinciblePlayerSlash2Texture; // 通用斩击2纹理
extern ID3D11ShaderResourceView* g_invinciblePlayerSlash3Texture; // 通用斩击3纹理
extern ID3D11ShaderResourceView* g_invinciblePlayerSlash4Texture; // 通用斩击4纹理
extern ID3D11ShaderResourceView* g_invinciblePlayerAirChargeTexture; // 通用空中蓄力纹理
extern ID3D11ShaderResourceView* g_invinciblePlayerFallingTexture;  // 通用下落纹理
extern ID3D11ShaderResourceView* g_invinciblePlayerGroundChargeTexture; // 通用地面蓄力纹理
extern ID3D11ShaderResourceView* g_invinciblePlayerWallSlideTexture; // 通用地面蓄力纹理


extern ID3D11ShaderResourceView* g_groundBlackTexture;
extern ID3D11ShaderResourceView* g_groundTexture;
extern ID3D11ShaderResourceView* g_groundTopTexture;
extern ID3D11ShaderResourceView* g_groundTopCornerLeftTexture;
extern ID3D11ShaderResourceView* g_groundTopCornerRightTexture;
extern ID3D11ShaderResourceView* g_groundLeftTexture;
extern ID3D11ShaderResourceView* g_groundRightTexture;
extern ID3D11ShaderResourceView* g_groundBottomTexture;
extern ID3D11ShaderResourceView* g_groundBottomCornerLeftTexture;
extern ID3D11ShaderResourceView* g_groundBottomCornerRightTexture;
extern ID3D11ShaderResourceView* g_groundCornerFacingTopLeftTexture;
extern ID3D11ShaderResourceView* g_groundCornerFacingTopRightTexture;
extern ID3D11ShaderResourceView* g_groundCornerFacingBottomLeftTexture;
extern ID3D11ShaderResourceView* g_groundCornerFacingBottomRightTexture;
extern ID3D11ShaderResourceView* g_bossDecorationTexture;
extern ID3D11ShaderResourceView* g_tutorialTexture;

extern ID3D11ShaderResourceView* g_goalTexture;
extern ID3D11ShaderResourceView* g_oneWayPlatformTexture;
extern ID3D11ShaderResourceView* g_bossHealthBarTexture;
extern ID3D11ShaderResourceView* g_bossInnerHPTexture;
extern ID3D11ShaderResourceView* g_backgroundTexture1;
extern ID3D11ShaderResourceView* g_backgroundTexture2;
extern ID3D11ShaderResourceView* g_backgroundTexture3;
extern ID3D11ShaderResourceView* g_platformWoodTexture;
extern ID3D11ShaderResourceView* g_dashEffectTexture;
extern ID3D11ShaderResourceView* g_chargeEffectTexture;
extern ID3D11ShaderResourceView* g_hitEffectTexture;
extern ID3D11ShaderResourceView* g_numberTexture;
extern ID3D11ShaderResourceView* g_uiNumberTexture;
extern ID3D11ShaderResourceView* g_arrowTexture;
extern ID3D11ShaderResourceView* g_cursorTexture;
extern ID3D11ShaderResourceView* g_comboNumberTexture;
extern ID3D11ShaderResourceView* g_comboXTexture;
extern ID3D11ShaderResourceView* g_comboRemainingTimeTexture;
extern ID3D11ShaderResourceView* g_gaugeBarTexture;
extern ID3D11ShaderResourceView* g_gaugeBarFilledTexture;
extern ID3D11ShaderResourceView* g_gaugeFullEffectTexture;
extern ID3D11ShaderResourceView* g_gaugeTrailParticleTexture;
extern ID3D11ShaderResourceView* g_gaugeKillParticleRedTexture;

extern ID3D11ShaderResourceView* g_signWASDTexture;
extern ID3D11ShaderResourceView* g_signSTexture;
extern ID3D11ShaderResourceView* g_signReleaseTexture;
extern ID3D11ShaderResourceView* g_signRedTexture;
extern ID3D11ShaderResourceView* g_signPinkTexture;
extern ID3D11ShaderResourceView* g_signLongClickTexture;
extern ID3D11ShaderResourceView* g_signESCTexture;
extern ID3D11ShaderResourceView* g_signRightTexture;
extern ID3D11ShaderResourceView* g_signClickTexture;
extern ID3D11ShaderResourceView* g_escTexture;


extern ID3D11ShaderResourceView* g_attackCountTestTexture;

extern InputSystem g_inputSystem;
extern GameTimer g_gameTimer;
extern GameState g_gameState;
extern Animation signAnim;

class GameCursor {
private:
    ID3D11ShaderResourceView* m_texture = nullptr;
    float m_width = 0.04f;
    float m_height = 0.12f;
    bool m_visible = true;
    float m_offsetX = 0.0f;
    float m_offsetY = 0.0f;

public:
    void Initialize(ID3D11ShaderResourceView* texture);
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }
    void Render(float cameraX, float cameraY);
};

struct GaugeTrailParticleInstance {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float scale = 1.0f;
    float timer = 0.0f;
    float frameTimer = 0.0f;
    float frameTime = 0.06f;
    int frame = 0;
    float rotation = 0.0f;
    float angularVelocity = 0.0f;
    static constexpr int rows = 1;
    static constexpr int columns = 5;
    static constexpr int frameCount = 5;
    bool active = false;
    ID3D11ShaderResourceView* texture = nullptr;
};

extern GameCursor g_gameCursor;

// Simple cursor mode control:
// - enabled: show in-game cursor, hide OS cursor
// - disabled: hide in-game cursor, show OS cursor
void SetInGameCursorEnabled(bool enabled);

// 在Game.h的全局变量部分添加
extern float g_slowMoTimer;      // 时间减慢计时器
extern float g_slowMoFactor;     // 时间减慢倍率
extern bool g_isSlowMotion;      // 是否处于时间减慢状态

// Tutorial active flag: when true, gameplay input & time are suspended for tutorial overlay
extern bool g_tutorialActive;

// for the timer counting
extern float g_gameElapsedTime;
extern int g_gameMinutes;
extern int g_gameSeconds;

// added november 27th for the pause 
extern ID3D11ShaderResourceView* g_pauseTexture;
extern ID3D11ShaderResourceView* g_paddingTitleAnim;

// Game initialization
void InitGameWorld();

// Clear gauge-related state when player dies
void ClearGaugeOnDeath();

// Provide the game window handle so mouse->client conversion is correct.
void SetGameWindowHandle(HWND hwnd);
// Drawing function
void DrawGame();

// used for statistics in the result
void DrawNumber(int number, float x, float y, float width, float height, ID3D11ShaderResourceView* texture);
void DrawTime(int minutes, int seconds, float x, float y, float size, ID3D11ShaderResourceView* texture);

// Input handling
void HandleInput();

void TriggerSlowMotion(float i, float a); // 1秒时间，减慢到20%速度
// Reset game
void ResetGame(bool fullReload = false);

void CleanUpGameWorld(void); // added  december 11th

void UpdateGame(float deltaTime);
void DashToMouse();
void StartMouseChargeDash();
void ExecuteMouseChargeDash();
void CancelChargeDash();
bool CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2);

// for the ui in the stages
struct InGameUI {
    float x;
    float y;
    float width;
    float height;
};

void DrawComboUI(void);
void DrawGaugeUI(void);
void DrawScoreUI(void);


//Player Movement Control
void Jump();
void CheckPlayerDeath();
void OnPlayerDeath();
void UpdatePlayerDeath(float deltaTime);
void UpdateDash(float deltaTime);
void UpdatePlayerPhysics(float deltaTime);
void OnEnemyDefeated();
void OnEnemyDefeated(bool wasWeakPointKill, float enemyWorldX, float enemyWorldY);
bool ConsumeDashPoint();
void UpdateDashPoints(float deltaTime);
void UpdateDashAftermath(float deltaTime);
void EnterDashAftermath();
void CheckDashAttack();


// 在Game.h中添加这些变量声明
class MouseIndicatorSystem  {
private:
    float m_mouseWorldX, m_mouseWorldY;
    float m_arrowAngle;
    bool m_showMouseIndicator;
    ID3D11ShaderResourceView* m_mouseIndicatorTexture;
    ID3D11ShaderResourceView* m_cursorTexture;
    bool m_arrowShow;
public:
    void showArrow(bool show) {
        m_arrowShow = show;
    }
    void Initialize();
    void Update(float deltaTime);
    void Render(float cameraX, float cameraY);
    void Cleanup();
    void ShowMouseIndicator(bool i);
};

// 全局实例
extern MouseIndicatorSystem g_mouseIndicator;


// 音效文件常量
namespace SoundEffect {
    const std::string JUMP = "asset/SE/p_footstep.wav";
    const std::string DASH = "asset/SE/slash.wav";
    const std::string CHARGE_START = "asset/SE/charge.wav";
    const std::string CHARGE_RELEASE = "asset/SE/slash.wav";
    const std::string SHOOT = "asset/SE/slash.wav";
    const std::string DEATH = "asset/SE/player_death.wav";
    const std::string SLASHCOUNT = "asset/SE/charge_3.wav";
    const std::string ENEMY_HIT = "asset/SE/hit.wav";
    const std::string ENEMY_DEATH = "asset/SE/damage.wav";
    const std::string SLOWMO_START = "asset/SE/slowmo_start.wav";
    const std::string SLOWMO_END = "asset/SE/slowmo_end.wav";
    const std::string LEVEL_COMPLETE = "asset/SE/level_complete.wav";
    const std::string UI_HOVER = "asset/SE/cursor.wav";
    const std::string UI_CLICK = "asset/SE/enter.wav";
    const std::string PAUSE = "asset/SE/pause.wav";
    const std::string RESUME = "asset/SE/resume.wav";

    // Gauge / limitbreak
    const std::string LIMITBREAK = "asset/SE/limitbreak.wav";
    const std::string INVINCIBLE_WARNING = "asset/SE/invincible_warning.wav";

    // Boss
    const std::string BOSS_SLASH1 = "asset/SE/oni_slash.wav";
    const std::string BOSS_CHARGE = "asset/SE/enemy_charge.wav";
    const std::string BOSS_CHARGE2 = "asset/SE/enemy_charge2.wav";
    const std::string BOSS_DASH = "asset/SE/oni_dash.wav";
    const std::string BOSS_DEATH = "asset/SE/boss_death.wav";
    const std::string BOSS_DOWN = "asset/SE/down.wav";
}

namespace BackgroundMusic {
    const std::string MAIN_MENU = "asset/Music/level1.wav";
    const std::string LEVEL1 = "asset/Music/Dancer.wav";
    const std::string LEVEL2 = "asset/Music/0246_Nightmare-Assemblage.wav";
    const std::string LEVEL3 = "asset/Music/planetarium_garden.wav";
    const std::string BOSS_BATTLE = "asset/Music/0194_Red-Eyes.wav";
    const std::string GAME_OVER = "asset/Music/game_over.wav";
    const std::string THE_CAKE = "asset/Music/0242_Walk_Among_the_Rubble.wav";
    const std::string VICTORY = "asset/Music/Sagittarius.wav";
}


// boss animation textures
extern ID3D11ShaderResourceView* g_bossIdleTexture;
extern ID3D11ShaderResourceView* g_bossAttackTexture;
extern ID3D11ShaderResourceView* g_bossDeathTexture;
extern ID3D11ShaderResourceView* g_bossChargeStage1Texture;
extern ID3D11ShaderResourceView* g_bossChargeStage2Texture;
extern ID3D11ShaderResourceView* g_bossDashTexture;
extern ID3D11ShaderResourceView* g_bossDashOverTexture;
extern ID3D11ShaderResourceView* g_bossSlashPrepTexture;
extern ID3D11ShaderResourceView* g_bossSlashActiveTexture;
extern ID3D11ShaderResourceView* g_bossDownBeforeTexture;
extern ID3D11ShaderResourceView* g_bossDownHorizontalTexture;
extern ID3D11ShaderResourceView* g_bossDownVarticalTexture;
extern ID3D11ShaderResourceView* g_bossDownDiagonal1Texture;
extern ID3D11ShaderResourceView* g_bossDownDiagonal2Texture;

extern ID3D11ShaderResourceView* g_finalbossIdleTexture;
extern ID3D11ShaderResourceView* g_finalbossAttackTexture;
extern ID3D11ShaderResourceView* g_finalbossDeathTexture;
extern ID3D11ShaderResourceView* g_finalbossChargeStage1Texture;
extern ID3D11ShaderResourceView* g_finalbossChargeStage2Texture;
extern ID3D11ShaderResourceView* g_finalbossDashTexture;
extern ID3D11ShaderResourceView* g_finalbossDashOverTexture;
extern ID3D11ShaderResourceView* g_finalbossSlashPrepTexture;
extern ID3D11ShaderResourceView* g_finalbossSlashActiveTexture;
extern ID3D11ShaderResourceView* g_finalbossDownBeforeTexture;
extern ID3D11ShaderResourceView* g_finalbossDownHorizontalTexture;

