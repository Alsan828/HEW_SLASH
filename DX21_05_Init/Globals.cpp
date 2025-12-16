#include "Game.h"
#include "Enemy.h"

// 定义所有全局变量
MapManager g_mapManager;
ProjectileManager& g_projectileManager = ProjectileManager::GetInstance();
Player g_player;
ID3D11ShaderResourceView* g_playerTexture = nullptr;
ID3D11ShaderResourceView* g_groundTexture = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture = nullptr;
ID3D11ShaderResourceView* g_dashEffectTexture = nullptr;
ID3D11ShaderResourceView* g_chargeEffectTexture = nullptr;
ID3D11ShaderResourceView* g_numberTexture = nullptr;
ID3D11ShaderResourceView* g_uiNumberTexture = nullptr;
ID3D11ShaderResourceView* g_arrowTexture = nullptr;
ID3D11ShaderResourceView* g_cursorTexture = nullptr;
InputSystem g_inputSystem;
GameTimer g_gameTimer;
GameState g_gameState = STATE_PLAYING;

ID3D11ShaderResourceView* g_pauseTexture = nullptr; // added for pause overlay

 float camera_Smoothness = 0.02f;
 float camera_LookAhead = 0.6f;
 float camera_DeadZone = 0.4f;

// 敌人相关的全局变量
std::vector<Enemy*> g_enemies;
ID3D11ShaderResourceView* g_enemyTexture = nullptr;
ID3D11ShaderResourceView* g_shieldEnemyTexture = nullptr;
ID3D11ShaderResourceView* g_mageEnemyTexture = nullptr;
ID3D11ShaderResourceView* g_fastEnemyTexture = nullptr;

// Define global camera object
Camera g_camera;

MouseIndicatorSystem g_mouseIndicator;
SimpleAudio g_audioManager;
// 在Game.cpp开头定义全局变量
int g_windowWidth = 0;
int g_windowHeight = 0;