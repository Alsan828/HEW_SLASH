#include "Game.h"
#include "Enemy.h"

// 定义所有全局变量
MapManager g_mapManager;
ProjectileManager& g_projectileManager = ProjectileManager::GetInstance();
Player g_player;
ID3D11ShaderResourceView* g_playerTexture = nullptr; // maybe we wont use this

// for the character
ID3D11ShaderResourceView* g_playerIdleLeftTexture = nullptr;
ID3D11ShaderResourceView* g_playerIdleRightTexture = nullptr;
ID3D11ShaderResourceView* g_playerJumpLeftTexture = nullptr;
ID3D11ShaderResourceView* g_playerJumpRightTexture = nullptr;
ID3D11ShaderResourceView* g_playerRunLeftTexture = nullptr;
ID3D11ShaderResourceView* g_playerRunRightTexture = nullptr;
ID3D11ShaderResourceView* g_playerSlashLeft1Texture = nullptr;
ID3D11ShaderResourceView* g_playerSlashRight1Texture = nullptr;
ID3D11ShaderResourceView* g_playerSlashLeft2Texture = nullptr;
ID3D11ShaderResourceView* g_playerSlashRight2Texture = nullptr;
ID3D11ShaderResourceView* g_playerSlashLeft3Texture = nullptr;
ID3D11ShaderResourceView* g_playerSlashRight3Texture = nullptr;
ID3D11ShaderResourceView* g_playerSlashLeft4Texture = nullptr;
ID3D11ShaderResourceView* g_playerSlashRight4Texture = nullptr;
ID3D11ShaderResourceView* g_playerAirChargeLeftTexture = nullptr;
ID3D11ShaderResourceView* g_playerAirChargeRightTexture = nullptr;
ID3D11ShaderResourceView* g_playerFallingLeftTexture = nullptr;
ID3D11ShaderResourceView* g_playerFallingRightTexture = nullptr;
ID3D11ShaderResourceView* g_playerGroundChargeLeftTexture = nullptr;
ID3D11ShaderResourceView* g_playerGroundChargeRightTexture = nullptr;

ID3D11ShaderResourceView* g_groundTexture = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture1 = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture2 = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture3 = nullptr;
ID3D11ShaderResourceView* g_dashEffectTexture = nullptr;
ID3D11ShaderResourceView* g_chargeEffectTexture = nullptr;
ID3D11ShaderResourceView* g_hitEffectTexture = nullptr;
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
 float camera_DeadZone = 0.2f;

// 敌人相关的全局变量
std::vector<Enemy*> g_enemies;

// Define global camera object
Camera g_camera;

MouseIndicatorSystem g_mouseIndicator;
SimpleAudio g_audioManager;
// 在Game.cpp开头定义全局变量
int g_windowWidth = 0;
int g_windowHeight = 0;