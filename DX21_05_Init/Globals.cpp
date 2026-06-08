#include "Game.h"
#include "Enemy.h"

// すべてのグローバル変数を定義する
MapManager g_mapManager;
ProjectileManager& g_projectileManager = ProjectileManager::GetInstance();
Player g_player;

bool g_releaseDashChargeMode = true;
bool g_noGravityAftermathMode = false;
ID3D11ShaderResourceView* g_playerTexture = nullptr; // 使わない可能性がある

// プレイヤー用。グローバル変数部分では左向き専用テクスチャを削除する。
// 右向きテクスチャを残し、汎用名へ変更する。
ID3D11ShaderResourceView* g_playerIdleTexture = nullptr;    // 汎用待機テクスチャ
ID3D11ShaderResourceView* g_playerDeathTexture = nullptr;    // 汎用死亡テクスチャ
ID3D11ShaderResourceView* g_playerJumpTexture = nullptr;   // 汎用ジャンプテクスチャ
ID3D11ShaderResourceView* g_playerRunTexture = nullptr;     // 汎用走行テクスチャ
ID3D11ShaderResourceView* g_playerSlash1Texture = nullptr; // 汎用斬撃 1 テクスチャ
ID3D11ShaderResourceView* g_playerSlash2Texture = nullptr; // 汎用斬撃 2 テクスチャ
ID3D11ShaderResourceView* g_playerSlash3Texture = nullptr; // 汎用斬撃 3 テクスチャ
ID3D11ShaderResourceView* g_playerSlash4Texture = nullptr; // 汎用斬撃 4 テクスチャ
ID3D11ShaderResourceView* g_playerAirChargeTexture = nullptr; // 汎用空中チャージテクスチャ
ID3D11ShaderResourceView* g_playerFallingTexture = nullptr;  // 汎用落下テクスチャ
ID3D11ShaderResourceView* g_playerGroundChargeTexture = nullptr; // 汎用地上チャージテクスチャ
ID3D11ShaderResourceView* g_playerWallSlideTexture = nullptr; 
// 無敵時のプレイヤー用
ID3D11ShaderResourceView* g_invinciblePlayerIdleTexture = nullptr;    // 汎用待機テクスチャ
ID3D11ShaderResourceView* g_invinciblePlayerJumpTexture = nullptr;   // 汎用ジャンプテクスチャ
ID3D11ShaderResourceView* g_invinciblePlayerRunTexture = nullptr;     // 汎用走行テクスチャ
ID3D11ShaderResourceView* g_invinciblePlayerSlash1Texture = nullptr; // 汎用斬撃 1 テクスチャ
ID3D11ShaderResourceView* g_invinciblePlayerSlash2Texture = nullptr; // 汎用斬撃 2 テクスチャ
ID3D11ShaderResourceView* g_invinciblePlayerSlash3Texture = nullptr; // 汎用斬撃 3 テクスチャ
ID3D11ShaderResourceView* g_invinciblePlayerSlash4Texture = nullptr; // 汎用斬撃 4 テクスチャ
ID3D11ShaderResourceView* g_invinciblePlayerAirChargeTexture = nullptr; // 汎用空中チャージテクスチャ
ID3D11ShaderResourceView* g_invinciblePlayerFallingTexture = nullptr;  // 汎用落下テクスチャ
ID3D11ShaderResourceView* g_invinciblePlayerGroundChargeTexture = nullptr; // 汎用地上チャージテクスチャ
ID3D11ShaderResourceView* g_invinciblePlayerWallSlideTexture = nullptr; // 汎用地上チャージテクスチャ

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
ID3D11ShaderResourceView* g_tutorialTexture = nullptr;

ID3D11ShaderResourceView* g_goalTexture = nullptr;
ID3D11ShaderResourceView* g_oneWayPlatformTexture = nullptr;
ID3D11ShaderResourceView* g_bossHealthBarTexture = nullptr;
ID3D11ShaderResourceView* g_bossInnerHPTexture = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture1 = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture2 = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture3 = nullptr;
// タイル表示用のスクロール背景テクスチャ（World3 の木製足場）
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
// 追従型体力アイコン用テクスチャ / アニメーション（1x3 スプライトシート）
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

ID3D11ShaderResourceView* g_pauseTexture = nullptr; // ポーズオーバーレイ用に追加
ID3D11ShaderResourceView* g_paddingTitleAnim = nullptr; // ポーズオーバーレイ用に追加
Animation paddingTitleAnim;

 float camera_Smoothness = 0.02f;//0.02f
 float camera_LookAhead = 0.6f;//0.6f
 float camera_DeadZone = 0.02f;//0.2f

// 敵関連のグローバル変数
std::vector<Enemy*> g_enemies;

// グローバルカメラオブジェクトを定義する
Camera g_camera;

MouseIndicatorSystem g_mouseIndicator;
// Game.cpp 冒頭でグローバル変数を定義する
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

ID3D11ShaderResourceView* g_finalbossIdleTexture = nullptr;
ID3D11ShaderResourceView* g_finalbossAttackTexture = nullptr;
ID3D11ShaderResourceView* g_finalbossDeathTexture = nullptr;
ID3D11ShaderResourceView* g_finalbossChargeStage1Texture = nullptr;
ID3D11ShaderResourceView* g_finalbossChargeStage2Texture = nullptr;
ID3D11ShaderResourceView* g_finalbossDashTexture = nullptr;
ID3D11ShaderResourceView* g_finalbossDashOverTexture = nullptr;
ID3D11ShaderResourceView* g_finalbossSlashPrepTexture = nullptr;
ID3D11ShaderResourceView* g_finalbossSlashActiveTexture = nullptr;
ID3D11ShaderResourceView* g_finalbossDownBeforeTexture = nullptr;
ID3D11ShaderResourceView* g_finalbossDownHorizontalTexture = nullptr;