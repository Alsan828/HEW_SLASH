#pragma once
#define NOMINMAX

#include <cstdint>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include "InputSystem.h"
#include "Render.h"
#include "Map.h"
#include "Camera.h"
#include "Animation.h"
#include "SceneManager.h"
#include "SceneBase.h"
#include "Pause.h"

extern SceneManager sceneManager;
extern MapManager g_mapManager;
extern InputSystem g_inputSystem;
extern Camera g_camera;
class Enemy;

// Game state enumeration
enum GameState {
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_PAUSED
};
// Player structure
struct Player {
    float posX = 0.0f;
    float posY = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    bool isOnGround = false;
    bool isMoving = false;
    bool facingRight = true;

    // 生命值系统
    float health = 100.0f;
    float maxHealth = 100.0f;
    bool isAlive = true;

    // 攻击系统
    float attackDamage = 30.0f;  // 基础攻击力
    bool isAttacking = false;    // 攻击状态
    float attackTimer = 0.0f;
    const float ATTACK_DURATION = 0.2f;

    // Dash related variables
    bool isDashing = false;
    float dashTimer = 0.0f;
    float dashDirectionX = 0.0f;
    float dashDirectionY = 0.0f;
    int dashLevel = 0;

    float mouseTargetX = 0.0f;
    float mouseTargetY = 0.0f;
    bool hasMouseTarget = false;
    bool allowMoveWhileCharging = true;

    // Charge dash specific variables
    bool isCharging = false;
    float chargeTime = 0.0f;
    const float MAX_CHARGE_TIME = 2.5f;
    const float MIN_CHARGE_TIME = 0.01f;
    const float CHARGE_THRESHOLD_LOW = 0.2f;
    const float CHARGE_THRESHOLD_MID = 0.7f;
    const float CHARGE_THRESHOLD_HIGH = 1.5f;

    Animation anim;

    // 冲刺点数系统
    int dashPoints = 3;
    const int MAX_DASH_POINTS = 3;
    float dashPointRecoverTimer = 0.0f;
    const float DASH_POINT_RECOVER_TIME = 0.55f;

    // 冲刺后硬直状态
    bool isInDashAftermath = false;
    float dashAftermathTimer = 0.0f;
    const float DASH_AFTERMATH_DURATION = 0.7f;

    const float AFTERIMAGE_DURATION = 0.1f;

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
        if (!isAlive) return;

        health -= damage;
        if (health <= 0) {
            health = 0;
            isAlive = false;
            // 玩家死亡处理
        }
    }
};

class GameTimer {
private:
    __int64 m_prevTime = 0;
    __int64 m_startTime = 0;
    __int64 m_currTime = 0;
    double m_secondsPerCount = 0.0;
    float m_deltaTime = 0.0f;
    double m_totalTime;       // 总游戏时间

public:
    GameTimer();
    void Tick();
    float GetDeltaTime() const;

    double GetTotalTime() const {
        return m_totalTime;
    }
};

// Global constant definitions
const float GRID_WIDTH = 0.0625f;
const float GRID_HEIGHT = 0.085f;
const float PLAYER_WIDTH = 0.08f;
const float PLAYER_HEIGHT = 0.12f;
const float GRAVITY = -0.003f;
const float JUMP_FORCE = 0.065f;
const float MOVE_SPEED = 0.01f;
const float DASH_SPEED = 0.15f;      // Base dash speed
const float DASH_DURATION = 0.05f;   // Base dash duration
const float DASH_COOLDOWN = 0.2f;    // Dash cooldown time

extern int g_windowWidth;
extern int g_windowHeight;

extern Player g_player;
extern ID3D11ShaderResourceView* g_playerTexture;
extern ID3D11ShaderResourceView* g_groundTexture;
extern ID3D11ShaderResourceView* g_backgroundTexture;
extern ID3D11ShaderResourceView* g_dashEffectTexture;
extern ID3D11ShaderResourceView* g_chargeEffectTexture;
extern InputSystem g_inputSystem;
extern GameTimer g_gameTimer;
extern GameState g_gameState;

// added november 27th for the pause 
extern ID3D11ShaderResourceView* g_pauseTexture;

// Game initialization
void InitGameWorld();

// Drawing function
void DrawGame();

// Input handling
void HandleInput();

// Reset game
void ResetGame(); 

void UpdateGame(float deltaTime);
void DashToMouse();
void StartMouseChargeDash();
void ExecuteMouseChargeDash();
void CancelChargeDash();
bool CheckCollision(float x1, float y1, float w1, float h1,
	float x2, float y2, float w2, float h2);


//Player Movement Control
void Jump();
void UpdateDash(float deltaTime);
void UpdatePlayerPhysics(float deltaTime);
void OnEnemyDefeated();
bool ConsumeDashPoint();
void UpdateDashPoints(float deltaTime);
void UpdateDashAftermath(float deltaTime);
void EnterDashAftermath();
void CheckDashAttack();


// 在Game.h中添加这些变量声明
class MouseIndicatorSystem {
private:
    float m_mouseWorldX, m_mouseWorldY;
    float m_arrowAngle;
    bool m_showMouseIndicator;
    ID3D11ShaderResourceView* m_mouseIndicatorTexture;
    ID3D11ShaderResourceView* m_arrowTexture;

public:
    void Initialize();
    void Update(float deltaTime);
    void Render(float cameraX, float cameraY);
    void Cleanup();
};

// 全局实例
extern MouseIndicatorSystem g_mouseIndicator;