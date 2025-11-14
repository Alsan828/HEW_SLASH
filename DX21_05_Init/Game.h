#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include "InputSystem.h"
#include "Render.h"


// Remove original IsKeyDown function declaration
// Add input system instance declaration
extern InputSystem g_inputSystem;

struct MapBlock;
class Enemy;

// Map block structure
struct MapBlock {
    float posX, posY;
    float width, height;
    bool isSolid;
};

// Game state enumeration
enum GameState {
    STATE_PLAYING,
    STATE_GAME_OVER
};

// Dash type enumeration
enum DashType {
    DASH_INSTANT,    // Dash immediately on key press
    DASH_CHARGE      // Charge dash on hold
};

// Player structure
struct Player {
    float posX = 0.0f;
    float posY = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    bool isOnGround = false;
    bool isMoving = false;
    bool facingRight = true; // Facing direction

    // Dash related variables
    bool isDashing = false;
    float dashTimer = 0.0f;
    float dashCooldown = 0.0f;
    float dashDirectionX = 0.0f;
    float dashDirectionY = 0.0f;

    // Charge dash specific variables
    bool isCharging = false;
    float chargeTime = 0.0f;
    const float MAX_CHARGE_TIME = 1.5f;  // Maximum charge time
    const float MIN_CHARGE_TIME = 0.01f; // Minimum valid charge time
};

class GameTimer {
private:
    __int64 m_prevTime = 0;
    __int64 m_currTime = 0;
    double m_secondsPerCount = 0.0;
    float m_deltaTime = 0.0f;

public:
    GameTimer();
    void Tick();
    float GetDeltaTime() const;
};


// Global constant definitions
const float GRID_WIDTH = 0.0625f;
const float GRID_HEIGHT = 0.085f;
const float PLAYER_WIDTH = 0.08f;
const float PLAYER_HEIGHT = 0.12f;
const float GRAVITY = -0.004f;
const float JUMP_FORCE = 0.075f;
const float MOVE_SPEED = 0.01f;
const float DASH_SPEED = 0.15f;      // Base dash speed
const float DASH_DURATION = 0.05f;   // Base dash duration
const float DASH_COOLDOWN = 0.2f;    // Dash cooldown time

// 在Game.h中声明
extern Player g_player;
extern std::vector<MapBlock> g_mapBlocks;
extern ID3D11ShaderResourceView* g_playerTexture;
extern ID3D11ShaderResourceView* g_groundTexture;
extern ID3D11ShaderResourceView* g_backgroundTexture;
extern ID3D11ShaderResourceView* g_dashEffectTexture;
extern ID3D11ShaderResourceView* g_chargeEffectTexture;
extern InputSystem g_inputSystem;
extern GameTimer g_gameTimer;
extern GameState g_gameState;
extern DashType g_currentDashType;

// Game initialization
void InitGameWorld();

// Game loop
//void GameLoop();

// Drawing function
void DrawGame();

// Input handling
void HandleInput();

// Reset game
void ResetGame(); 

void UpdateGame(float deltaTime);

bool CheckCollision(float x1, float y1, float w1, float h1,
	float x2, float y2, float w2, float h2);