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

extern ProjectileManager& g_projectileManager;  // 全局射弹管理器
extern SceneManager sceneManager;
extern MapManager g_mapManager;
extern InputSystem g_inputSystem;
extern Camera g_camera;
class Enemy;

// Slash-count UI (1x4 spritesheet)
extern ID3D11ShaderResourceView* g_slashCountTexture;
extern Animation g_slashCountAnim;

// When a dash point is restored on enemy death, spawn the follower indicator from that world position.
extern float g_slashCountSpawnX;
extern float g_slashCountSpawnY;
extern bool g_slashCountSpawnPending;

// Game state enumeration
enum GameState {
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_PAUSED
};

// Global constant definitions
const float GRID_WIDTH = 0.0625f;
const float GRID_HEIGHT = 0.085f;
const float PLAYER_WIDTH = 0.08f;
const float PLAYER_HEIGHT = 0.12f;
const float GRAVITY = -0.002f;
const float JUMP_FORCE = 0.045f;
// Base move speed (slower than before). The old value was 0.01f.
// Normal state move speed now is 0.8x of the previous speed.
const float MOVE_SPEED = 0.008f; 
const float DASH_SPEED = 0.05f;      // Base dash speed (halved)
const float DASH_DURATION = 0.16f;   // Base dash duration
const float DASH_COOLDOWN = 0.0f;    // Dash cooldown time

// Acceleration (combo) state
const int ACCEL_COMBO_THRESHOLD = 5;        // comboCount > 5 to enter accelerated state
const float ACCEL_MOVE_SPEED_MULT = 1.75f;  // accelerated move speed multiplier
const float ACCEL_ANIM_SPEED_MULT = 1.75f;  // keep animation speed in sync with movement
const float ACCEL_CHARGE_SPEED_MULT = 1.5f; // accelerated charge speed multiplier

// Player structure
struct Player {
    float posX = 0.0f;
    float posY = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    bool isOnGround = false;
    bool isMoving = false;
    bool facingRight = true;
    // 死亡状态相关
    bool isDead = false;
    float deathTimer = 0.0f;
    const float DEATH_RESPAWN_TIME = 1.5f;  // 死亡后复活等待时间
    int deathCount = 0;  // 死亡计数（可选）

    // for the combo UI of the player when attacking
    int comboCount = 0;
    float comboTimer = 0.0f;   // for the time before the combo resets
    const float COMBO_RESET_TIME = 2.0f; // 2 seconds without killing, the combo will reset

    // Acceleration state (triggered by combos)
    bool isAccelerated = false;

    // for the gauge bar system
    int gaugePoints = 0;              // current gauge points
    const int MAX_GAUGE_POINTS = 12;  // maximum gauge points
    bool isInvincible = false;        // invincibility state
    float invincibleTimer = 0.0f;     // invincibility timer
    const float INVINCIBLE_DURATION = 5.0f;  // 5 seconds of invincibility

    // Invincibility source control:
    // true  -> invincible granted by full gauge / combo reward, use Invincible* animation set
    // false -> invincible granted by dash/slash window, do NOT switch animation set
    bool isGaugeInvincible = false;
	float g_gaugeEffectTimer = 0.0f;  // gauge effect timer
    bool g_gaugeEffectActive = false;


    // 生命值系统
    float health = 100.0f;
    float maxHealth = 100.0f;
    // 攻击系统
    float attackDamage = 30000.0f;  // 基础攻击力
    bool isAttacking = false;    // 攻击状态
    float attackTimer = 0.0f;
    const float ATTACK_DURATION = 0.2f;

    // Dash related variables
    bool isDashing = false;
    float dashTimer = 0.0f;
    float dashDirectionX = 0.0f;
    float dashDirectionY = 0.0f;
    int dashLevel = 0; 
    // 在 Player 结构体中添加以下变量
    bool isWallSliding = false;
    int wallSlideDirection = 0; // 0=无, -1=左墙, 1=右墙
    float wallSlideTimer = 0.0f;
    const float WALL_SLIDE_SPEED = -0.02f;    // 墙壁滑行速度

    float mouseTargetX = 0.0f;
    float mouseTargetY = 0.0f;
    bool hasMouseTarget = false;
    bool allowMoveWhileCharging = true;

    // Charge dash specific variables
    bool isCharging = false;
    float chargeTime = 0.0f;
    const float MAX_CHARGE_TIME = 1.0f;
    const float MIN_CHARGE_TIME = 0.01f;
    const float CHARGE_THRESHOLD_LOW = 0.2f;
    const float CHARGE_THRESHOLD_MID = 0.4f;
    const float CHARGE_THRESHOLD_HIGH = 0.8f;
    int hitStopTriggered = 0 ;     // 本次冲刺中已触发的顿刀次数
    float hitStopTimer = 0.0f;       // 顿刀计时器

    //蓄力层数系统
    float savedChargeTime = 0.0f;        // 保存的蓄力时间
    bool hasSavedCharge = false;         // 是否有保存的蓄力
    float chargeDecayTimer = 0.0f;        // 蓄力衰减计时器
    const float CHARGE_DECAY_TIME = 1.0f; // 蓄力保存时间

    Animation anim;
    float animLockTimer = 0.0f; // used for when changing from one animation to another
    float animLockDuration = 0.25f;

    // 冲刺点数系统
    int dashPoints = 3;
    const int MAX_DASH_POINTS = 3;
    float dashPointRecoverTimer = 0.0f;
    const float DASH_POINT_RECOVER_TIME = 0.1f;

    // 冲刺后硬直状态
    bool isInDashAftermath = false;
    float dashAftermathTimer = 0.0f;
    const float DASH_AFTERMATH_DURATION = 0.7f;

    // Dash-end slow-motion (real-time): duration also defines post-dash invincibility window.
    bool isInDashEndSlowMo = false;
    float dashEndSlowMoTimer = 0.0f;
    // Reduced by 1/3: 0.75s -> 0.5s
    const float DASH_END_SLOWMO_REALTIME = 0.5f;
    const float DASH_END_SLOWMO_FACTOR = 0.35f;
    
    const float AFTERIMAGE_DURATION = 0.35f;

     // 残像参数
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

    // 新增：攻击检测相关
    std::vector<Enemy*> hitEnemies; // 本次冲刺已击中的敌人（避免重复伤害）

    // 重置攻击状态
    void ResetAttack() {
        isAttacking = false;
        attackTimer = 0.0f;
        hitEnemies.clear();
    }

    // 受到伤害
    void TakeDamage(float damage) {
        if (isDead) return;

        health -= damage;
        if (health <= 0) {
            health = 0;
            isDead = false;
            // 玩家死亡处理
        }
    }

    // 获取当前蓄力等级
    int GetChargeLevel() const {
        if (chargeTime >= CHARGE_THRESHOLD_HIGH) return 3;
        if (chargeTime >= CHARGE_THRESHOLD_MID) return 2;
        if (chargeTime >= CHARGE_THRESHOLD_LOW) return 1;
        return 0;
    }

    // 保存当前蓄力
    void SaveCharge() {
        if (chargeTime > MIN_CHARGE_TIME) {
            savedChargeTime = chargeTime;
            hasSavedCharge = true;
            chargeDecayTimer = CHARGE_DECAY_TIME;
        }
    }

    // 加载保存的蓄力
    void LoadSavedCharge() {
        if (hasSavedCharge) {
            chargeTime = savedChargeTime;
            chargeDecayTimer = CHARGE_DECAY_TIME; // 重置衰减计时器
        }
    }

    // 清空保存的蓄力
    void ClearSavedCharge() {
        hasSavedCharge = false;
        savedChargeTime = 0.0f;
        chargeDecayTimer = 0.0f;
    }

    // 更新蓄力衰减
    void UpdateChargeDecay(float deltaTime) {
        if (hasSavedCharge) {
            chargeDecayTimer -= deltaTime;
            if (chargeDecayTimer <= 0.0f) {
                ClearSavedCharge(); // 时间到，清空保存的蓄力
            }
        }
    }
    // 根据时间获取蓄力等级
    int GetChargeLevelFromTime(float chargeTime) const {
        if (chargeTime >= CHARGE_THRESHOLD_HIGH) return 3;
        if (chargeTime >= CHARGE_THRESHOLD_MID) return 2;
        if (chargeTime >= CHARGE_THRESHOLD_LOW) return 1;
        return 0;
    }
};


// for the statistics of the game. deaths, kills, etc
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
   // no need construct now bc I initialied the variables above

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


extern ID3D11ShaderResourceView* g_groundTexture;
extern ID3D11ShaderResourceView* g_goalTexture;
extern ID3D11ShaderResourceView* g_oneWayPlatformTexture;
extern ID3D11ShaderResourceView* g_bossHealthBarTexture;
extern ID3D11ShaderResourceView* g_backgroundTexture1;
extern ID3D11ShaderResourceView* g_backgroundTexture2;
extern ID3D11ShaderResourceView* g_backgroundTexture3;
extern ID3D11ShaderResourceView* g_dashEffectTexture;
extern ID3D11ShaderResourceView* g_chargeEffectTexture;
extern ID3D11ShaderResourceView* g_hitEffectTexture;
extern ID3D11ShaderResourceView* g_numberTexture;
extern ID3D11ShaderResourceView* g_uiNumberTexture;
extern ID3D11ShaderResourceView* g_arrowTexture;
extern ID3D11ShaderResourceView* g_cursorTexture;
extern ID3D11ShaderResourceView* g_comboNumberTexture;
extern ID3D11ShaderResourceView* g_comboXTexture;
extern ID3D11ShaderResourceView* g_gaugeBarTexture;
extern ID3D11ShaderResourceView* g_gaugeBarEmptyTexture;
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

// for the timer counting
extern float g_gameElapsedTime;
extern int g_gameMinutes;
extern int g_gameSeconds;

// added november 27th for the pause 
extern ID3D11ShaderResourceView* g_pauseTexture;
extern ID3D11ShaderResourceView* g_paddingTitleAnim;

// Game initialization
void InitGameWorld();

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
void ResetGame();

void CleanUpGameWorld(void); // added  december 11th

void UpdateGame(float deltaTime);
void DashToMouse();
void StartMouseChargeDash();
void ExecuteMouseChargeDash();
void CancelChargeDash();
bool CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2);

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
}

namespace BackgroundMusic {
    const std::string MAIN_MENU = "asset/Music/main_menu.wav";
    const std::string LEVEL1 = "asset/Music/Ride_out.wav";
    const std::string LEVEL2 = "asset/Music/level2.wav";
    const std::string LEVEL3 = "asset/Music/level3.wav";
    const std::string BOSS_BATTLE = "asset/Music/0194_Red-Eyes.wav";
    const std::string GAME_OVER = "asset/Music/game_over.wav";
    const std::string VICTORY = "asset/Music/victory.wav";
}
