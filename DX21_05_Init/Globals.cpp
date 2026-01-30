#include "Game.h"
#include "Enemy.h"

// 定义所有全局变量
MapManager g_mapManager;
ProjectileManager& g_projectileManager = ProjectileManager::GetInstance();
Player g_player;

bool g_releaseDashChargeMode = true;
bool g_noGravityAftermathMode = false;
ID3D11ShaderResourceView* g_playerTexture = nullptr; // maybe we wont use this

// for the character// 在全局变量部分，删除向左的纹理变量
// 保留右边的纹理变量，并重命名为通用名称
ID3D11ShaderResourceView* g_playerIdleTexture = nullptr;    // 通用站立纹理
ID3D11ShaderResourceView* g_playerDeathTexture = nullptr;    // 通用站立纹理
ID3D11ShaderResourceView* g_playerJumpTexture = nullptr;   // 通用跳跃纹理
ID3D11ShaderResourceView* g_playerRunTexture = nullptr;     // 通用奔跑纹理
ID3D11ShaderResourceView* g_playerSlash1Texture = nullptr; // 通用斩击1纹理
ID3D11ShaderResourceView* g_playerSlash2Texture = nullptr; // 通用斩击2纹理
ID3D11ShaderResourceView* g_playerSlash3Texture = nullptr; // 通用斩击3纹理
ID3D11ShaderResourceView* g_playerSlash4Texture = nullptr; // 通用斩击4纹理
ID3D11ShaderResourceView* g_playerAirChargeTexture = nullptr; // 通用空中蓄力纹理
ID3D11ShaderResourceView* g_playerFallingTexture = nullptr;  // 通用下落纹理
ID3D11ShaderResourceView* g_playerGroundChargeTexture = nullptr; // 通用地面蓄力纹理
ID3D11ShaderResourceView* g_playerWallSlideTexture = nullptr; 
// for the character when invincible
ID3D11ShaderResourceView* g_invinciblePlayerIdleTexture = nullptr;    // 通用站立纹理
ID3D11ShaderResourceView* g_invinciblePlayerJumpTexture = nullptr;   // 通用跳跃纹理
ID3D11ShaderResourceView* g_invinciblePlayerRunTexture = nullptr;     // 通用奔跑纹理
ID3D11ShaderResourceView* g_invinciblePlayerSlash1Texture = nullptr; // 通用斩击1纹理
ID3D11ShaderResourceView* g_invinciblePlayerSlash2Texture = nullptr; // 通用斩击2纹理
ID3D11ShaderResourceView* g_invinciblePlayerSlash3Texture = nullptr; // 通用斩击3纹理
ID3D11ShaderResourceView* g_invinciblePlayerSlash4Texture = nullptr; // 通用斩击4纹理
ID3D11ShaderResourceView* g_invinciblePlayerAirChargeTexture = nullptr; // 通用空中蓄力纹理
ID3D11ShaderResourceView* g_invinciblePlayerFallingTexture = nullptr;  // 通用下落纹理
ID3D11ShaderResourceView* g_invinciblePlayerGroundChargeTexture = nullptr; // 通用地面蓄力纹理
ID3D11ShaderResourceView* g_invinciblePlayerWallSlideTexture = nullptr; // 通用地面蓄力纹理

ID3D11ShaderResourceView* g_groundBlackTexture = nullptr;
ID3D11ShaderResourceView* g_groundTexture = nullptr;
ID3D11ShaderResourceView* g_groundTopTexture = nullptr;
ID3D11ShaderResourceView* g_groundTopCornerLeftTexture = nullptr;
ID3D11ShaderResourceView* g_groundTopCornerRightTexture = nullptr;
ID3D11ShaderResourceView* g_groundLeftTexture = nullptr;
ID3D11ShaderResourceView* g_groundRightTexture = nullptr;
ID3D11ShaderResourceView* g_groundBottomTexture = nullptr;
ID3D11ShaderResourceView* g_groundBottomCornerLeftTexture = nullptr;
ID3D11ShaderResourceView* g_groundBottomCornerRightTexture = nullptr;
ID3D11ShaderResourceView* g_groundCornerFacingTopLeftTexture = nullptr;
ID3D11ShaderResourceView* g_groundCornerFacingTopRightTexture = nullptr;
ID3D11ShaderResourceView* g_groundCornerFacingBottomLeftTexture = nullptr;
ID3D11ShaderResourceView* g_groundCornerFacingBottomRightTexture = nullptr;
ID3D11ShaderResourceView* g_bossDecorationTexture = nullptr;

ID3D11ShaderResourceView* g_goalTexture = nullptr;
ID3D11ShaderResourceView* g_oneWayPlatformTexture = nullptr;
ID3D11ShaderResourceView* g_bossHealthBarTexture = nullptr;
ID3D11ShaderResourceView* g_bossInnerHPTexture = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture1 = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture2 = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture3 = nullptr;
// Scrolling background texture for tiling (platform wood for World3)
ID3D11ShaderResourceView* g_platformWoodTexture = nullptr;
ID3D11ShaderResourceView* g_dashEffectTexture = nullptr;
ID3D11ShaderResourceView* g_chargeEffectTexture = nullptr;
ID3D11ShaderResourceView* g_hitEffectTexture = nullptr;
ID3D11ShaderResourceView* g_numberTexture = nullptr;
ID3D11ShaderResourceView* g_uiNumberTexture = nullptr;
ID3D11ShaderResourceView* g_arrowTexture = nullptr;
ID3D11ShaderResourceView* g_cursorTexture = nullptr;
ID3D11ShaderResourceView* g_comboNumberTexture = nullptr;
ID3D11ShaderResourceView* g_comboXTexture = nullptr;
ID3D11ShaderResourceView* g_comboRemainingTimeTexture = nullptr;
ID3D11ShaderResourceView* g_slashCountTexture = nullptr;
Animation g_slashCountAnim;
// Health follower texture/animation (1x3 spritesheet)
ID3D11ShaderResourceView* g_healthTexture = nullptr;
Animation g_healthAnim;
ID3D11ShaderResourceView* g_gaugeBarTexture = nullptr;
ID3D11ShaderResourceView* g_gaugeBarFilledTexture = nullptr;
ID3D11ShaderResourceView* g_gaugeFullEffectTexture = nullptr;
ID3D11ShaderResourceView* g_gaugeTrailParticleTexture = nullptr;
ID3D11ShaderResourceView* g_gaugeKillParticleRedTexture = nullptr;

ID3D11ShaderResourceView* g_signWASDTexture = nullptr;
ID3D11ShaderResourceView* g_signSTexture = nullptr;
ID3D11ShaderResourceView* g_signReleaseTexture = nullptr;
ID3D11ShaderResourceView* g_signRedTexture = nullptr;
ID3D11ShaderResourceView* g_signPinkTexture = nullptr;
ID3D11ShaderResourceView* g_signLongClickTexture = nullptr;
ID3D11ShaderResourceView* g_signESCTexture = nullptr;
ID3D11ShaderResourceView* g_signRightTexture = nullptr;
ID3D11ShaderResourceView* g_signClickTexture = nullptr;
ID3D11ShaderResourceView* g_escTexture = nullptr;
Animation signAnim;

ID3D11ShaderResourceView* g_attackCountTestTexture = nullptr;

InputSystem g_inputSystem;
GameTimer g_gameTimer;
GameState g_gameState = STATE_PLAYING;

ID3D11ShaderResourceView* g_pauseTexture = nullptr; // added for pause overlay
ID3D11ShaderResourceView* g_paddingTitleAnim = nullptr; // added for pause overlay
Animation paddingTitleAnim;

 float camera_Smoothness = 0.02f;//0.02f
 float camera_LookAhead = 0.6f;//0.6f
 float camera_DeadZone = 0.02f;//0.2f

// 敌人相关的全局变量
std::vector<Enemy*> g_enemies;

// Define global camera object
Camera g_camera;

MouseIndicatorSystem g_mouseIndicator;
// 在Game.cpp开头定义全局变量
int g_windowWidth = 0;
int g_windowHeight = 0;

ID3D11ShaderResourceView* g_bossIdleTexture = nullptr;
ID3D11ShaderResourceView* g_bossAttackTexture = nullptr;
ID3D11ShaderResourceView* g_bossDeathTexture = nullptr;
ID3D11ShaderResourceView* g_bossChargeStage1Texture = nullptr;
ID3D11ShaderResourceView* g_bossChargeStage2Texture = nullptr;
ID3D11ShaderResourceView* g_bossDashTexture = nullptr;
ID3D11ShaderResourceView* g_bossDashOverTexture = nullptr;
ID3D11ShaderResourceView* g_bossSlashPrepTexture = nullptr;
ID3D11ShaderResourceView* g_bossSlashActiveTexture = nullptr;
ID3D11ShaderResourceView* g_bossDownBeforeTexture = nullptr;
ID3D11ShaderResourceView* g_bossDownHorizontalTexture = nullptr;
ID3D11ShaderResourceView* g_bossDownVarticalTexture = nullptr;
ID3D11ShaderResourceView* g_bossDownDiagonal1Texture = nullptr;
ID3D11ShaderResourceView* g_bossDownDiagonal2Texture = nullptr;
Animation g_bossSlashAnim;