#include "Game.h"
#include "Enemy.h"

// 定义所有全局变量
MapManager g_mapManager;
Player g_player;
ID3D11ShaderResourceView* g_playerTexture = nullptr;
ID3D11ShaderResourceView* g_groundTexture = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture = nullptr;
ID3D11ShaderResourceView* g_dashEffectTexture = nullptr;
ID3D11ShaderResourceView* g_chargeEffectTexture = nullptr;
InputSystem g_inputSystem;
GameTimer g_gameTimer;
GameState g_gameState = STATE_PLAYING;
DashType g_currentDashType = DASH_INSTANT;

ID3D11ShaderResourceView* g_pauseTexture = nullptr; // added for pause overlay

 float camera_Smoothness = 0.004f;
 float camera_LookAhead = 0.5f;
 float camera_DeadZone = 0.3f;

// 敌人相关的全局变量
std::vector<Enemy*> g_enemies;
ID3D11ShaderResourceView* g_enemyTexture = nullptr;
ID3D11ShaderResourceView* g_shieldEnemyTexture = nullptr;
ID3D11ShaderResourceView* g_mageEnemyTexture = nullptr;
ID3D11ShaderResourceView* g_fastEnemyTexture = nullptr;

// Define global camera object
Camera g_camera;