#include "Game.h"
#include "Enemy.h"

// 定义所有全局变量
Player g_player;
std::vector<MapBlock> g_mapBlocks;
ID3D11ShaderResourceView* g_playerTexture = nullptr;
ID3D11ShaderResourceView* g_groundTexture = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture = nullptr;
ID3D11ShaderResourceView* g_dashEffectTexture = nullptr;
ID3D11ShaderResourceView* g_chargeEffectTexture = nullptr;
InputSystem g_inputSystem;
GameTimer g_gameTimer;
GameState g_gameState = STATE_PLAYING;
DashType g_currentDashType = DASH_INSTANT;

// 敌人相关的全局变量
std::vector<Enemy*> g_enemies;
ID3D11ShaderResourceView* g_enemyTexture = nullptr;
ID3D11ShaderResourceView* g_shieldEnemyTexture = nullptr;
ID3D11ShaderResourceView* g_mageEnemyTexture = nullptr;
ID3D11ShaderResourceView* g_fastEnemyTexture = nullptr;