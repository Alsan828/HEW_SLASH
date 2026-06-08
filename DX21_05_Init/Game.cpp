#include "Game.h"
// Game.cpp のグローバル変数定義セクションへ追加
float g_slowMoTimer = 0.0f;
float g_slowMoFactor = 1.0f;
bool g_isSlowMotion = false;

// チュートリアル用グローバルフラグ
bool g_tutorialActive = false;

// 12 月 4 日追加
// ゲームタイマー用
float g_gameElapsedTime = 0.0f;
int g_gameMinutes = 0;
int g_gameSeconds = 0;


// 効果音インスタンス ID の保存領域
int g_jumpSoundId = -1;
int g_dashSoundId = -1;
int g_chargeSoundId = -1;
int g_shootSoundId = -1;
int g_slowMoTimerSoundId = -1;

static std::vector<HitEffectInstance> g_weakPointHitEffects;
static constexpr int WEAKPOINT_HIT_EFFECT_FRAMES = 3;
static constexpr int WEAKPOINT_HIT_EFFECT_COLUMNS = 3;
static constexpr int WEAKPOINT_HIT_EFFECT_ROWS = 1;

static ID3D11ShaderResourceView* g_slashFlashTextures[4] = { nullptr, nullptr, nullptr, nullptr };

struct AfterImageInstance {
    float x;
    float y;
    float width;
    float height;
    float timer;
    float duration;
    int frameIndex;
    int splitX;
    int splitY;
    bool flipHorizontal;
    ID3D11ShaderResourceView* texture;
};

static std::vector<AfterImageInstance> g_playerAfterImages;

static std::vector<GaugeTrailParticleInstance> g_gaugeTrailParticles;
static float g_gaugeTrailSpawnTimer = 0.0f;
static constexpr float GAUGE_TRAIL_SPAWN_INTERVAL = 0.03f;
static constexpr float GAUGE_TRAIL_SPAWN_RATE_MULT = 1.5f;
static constexpr float GAUGE_TRAIL_ANIM_SPEED_MULT = 0.75f;

struct GaugeKillParticleInstance {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float scale = 1.0f;
    float timer = 0.0f;
    float frameTimer = 0.0f;
    float frameTime = 0.05f;
    int frame = 0;
    float rotation = 0.0f;
    float angularVelocity = 0.0f;
    static constexpr int rows = 1;
    static constexpr int columns = 5;
    static constexpr int frameCount = 5;
    bool active = false;
    ID3D11ShaderResourceView* texture = nullptr;
};

static std::vector<GaugeKillParticleInstance> g_gaugeKillParticlesRed;

static float Rand01() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}
// ゲージ状態とパーティクルをクリアする（他モジュールから呼び出し可）
void ClearGaugeOnDeath()
{
    // プレイヤーのゲージ値をリセットする
    g_player.gaugePoints = 0;
    g_player.isInvincible = false;
    g_player.isGaugeInvincible = false;
    g_player.g_gaugeEffectActive = false;
    g_player.g_gaugeEffectTimer = 0.0f;

    // パーティクルとスポーンタイマーをクリアする
    g_gaugeTrailParticles.clear();
    g_gaugeKillParticlesRed.clear();
    g_gaugeTrailSpawnTimer = 0.0f;
}

void SpawnGaugeKillParticlesRed(float worldX, float worldY) {
    if (!g_gaugeKillParticleRedTexture) return;

    const int count = 4 + (rand() % 4); // 4..7 個
    for (int i = 0; i < count; ++i) {
        GaugeKillParticleInstance p;
        p.x = worldX;
        p.y = worldY;
        p.texture = g_gaugeKillParticleRedTexture;
        p.active = true;

        // ランダムな放射状バースト
        const float angle = Rand01() * 6.2831853f;
        const float speed = (0.06f + Rand01() * 0.09f) * 10.0f;
        p.vx = cosf(angle) * speed;
        p.vy = sinf(angle) * speed;

        p.scale = (0.45f * (0.85f + Rand01() * 0.35f)) * 2.0f;
        p.rotation = (Rand01() * 2.0f - 1.0f) * 3.14159f;
        p.angularVelocity = (Rand01() * 2.0f - 1.0f) * 6.0f;
        p.frame = 0;
        p.timer = 0.0f;
        p.frameTimer = 0.0f;
        // アニメ速度をおよそ 0.6 倍に落とす（フレーム時間を延ばす）
        p.frameTime = 0.05f / 0.6f;

        g_gaugeKillParticlesRed.push_back(p);
    }
}

// ゲージ満タン時（プレイヤー無敵化）に、より大きく派手なバーストを出す。
// 通常の赤い撃破バーストより多くの粒子・高い速度・広い拡散を持たせる。
void SpawnGaugeFullBurst(float worldX, float worldY) {
    // より変化のある見た目にするため trail 用パーティクルシートを使う
    if (!g_gaugeTrailParticleTexture) return;

    const int count = 20 + (rand() % 11); // 非常に大きいバースト用に 20..30 個
    for (int i = 0; i < count; ++i) {
        GaugeKillParticleInstance p;
        p.x = worldX;
        p.y = worldY;
        p.texture = g_gaugeTrailParticleTexture; // particle_sheet.png
        p.active = true;

        // より強く、ばらつきの大きい放射状バースト
        const float angle = Rand01() * 6.2831853f;
        const float speed = (0.15f + Rand01() * 0.35f) * 10.0f; // より高速
        p.vx = cosf(angle) * speed;
        p.vy = sinf(angle) * speed;

        // より大きく、ばらつきのあるスケール
        p.scale = 1.2f * (0.9f + Rand01() * 0.6f);
        p.rotation = (Rand01() * 2.0f - 1.0f) * 3.14159f;
        p.angularVelocity = (Rand01() * 2.0f - 1.0f) * 12.0f; // 強めの回転
        p.frame = 0;
        p.timer = 0.0f;
        p.frameTimer = 0.0f;
        // キビキビ見えるようアニメを速める
        p.frameTime = 0.035f;

        g_gaugeKillParticlesRed.push_back(p);
    }
}

static void SpawnGaugeTrailParticle(float worldX, float worldY) {
    if (!g_gaugeTrailParticleTexture) return;

    GaugeTrailParticleInstance p;
    p.x = worldX;
    p.y = worldY;
    p.texture = g_gaugeTrailParticleTexture;
    p.active = true;

    // 自然な拡散:
    // - ランダムな流れ方向
    // - 向きに対して少し後方へ寄せる
    // - ランダムなスケールと回転
    const float r01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    const float r02 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    const float r03 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    const float r04 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    const float angle = (r01 * 2.0f - 1.0f) * 1.1f; // およそ [-1.1, 1.1] rad
    const float speed = 0.02f + r02 * 0.03f;        // 0.02..0.05

    float backBias = g_player.facingRight ? -0.015f : 0.015f;
    p.vx = cosf(angle) * speed + backBias;
    p.vy = sinf(angle) * speed + 0.01f;

    // サイズ: 以前の半分（基準 1.0 -> 0.5）で、少しランダム差を付ける。
    p.scale = 0.5f * (0.85f + r03 * 0.30f);
    p.rotation = (r04 * 2.0f - 1.0f) * 0.35f;
    p.angularVelocity = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f) * 2.2f;
    p.frame = 0;
    p.timer = 0.0f;
    p.frameTimer = 0.0f;
    p.frameTime = 0.06f / GAUGE_TRAIL_ANIM_SPEED_MULT;

    g_gaugeTrailParticles.push_back(p);
}

GameStatistics g_gameStats;
Animation g_gaugeEffectAnim;

void SpawnWeakPointHitEffect(float worldX, float worldY) {
    SpawnWeakPointHitEffectScaled(worldX, worldY, 0.0f);
}

void SpawnWeakPointHitEffectScaled(float worldX, float worldY, float overrideScale /*=0*/) {
    // 新しい slash flash テクスチャを優先し、なければ旧単体テクスチャへフォールバックする。
    ID3D11ShaderResourceView* chosen = nullptr;
    int availableCount = 0;
    ID3D11ShaderResourceView* available[4] = { nullptr,nullptr,nullptr,nullptr };

    for (auto* t : g_slashFlashTextures) {
        if (t) available[availableCount++] = t;
    }

    if (availableCount > 0) {
        chosen = available[rand() % availableCount];
    }
    else {
        chosen = g_hitEffectTexture;
    }

    if (!chosen) return;

    HitEffectInstance e;
    e.x = worldX;
    e.y = worldY;
    // 弱点用の slash flash は通常ヒットより目立たせる。
    float baseScale = (chosen == g_hitEffectTexture) ? 1.0f : 2.0f;
    if (overrideScale > 0.0f) baseScale = overrideScale;
    e.scale = baseScale;
    e.timer = 0.0f;
    e.frameTime = 0.08f;
    e.frame = 0;
    e.active = true;
    e.texture = chosen;

    // テクスチャによってはスプライトシート（slash flash）で、他は単体画像（旧 hit）である。
    if (chosen == g_hitEffectTexture) {
        e.rows = 1;
        e.columns = 1;
        e.frameCount = 1;
    }
    else {
        e.rows = WEAKPOINT_HIT_EFFECT_ROWS;
        e.columns = WEAKPOINT_HIT_EFFECT_COLUMNS;
        e.frameCount = WEAKPOINT_HIT_EFFECT_FRAMES;
    }
    g_weakPointHitEffects.push_back(e);
}

// 弱点撃破用に、より大きな視覚エフェクトを生成する
void SpawnWeakPointKillEffect(float worldX, float worldY) {
    // 新しい slash flash テクスチャを優先し、なければ旧単体テクスチャへフォールバックする。
    ID3D11ShaderResourceView* chosen = nullptr;
    int availableCount = 0;
    ID3D11ShaderResourceView* available[4] = { nullptr,nullptr,nullptr,nullptr };

    for (auto* t : g_slashFlashTextures) {
        if (t) available[availableCount++] = t;
    }

    if (availableCount > 0) {
        chosen = available[rand() % availableCount];
    }
    else {
        chosen = g_hitEffectTexture;
    }

    if (!chosen) return;

    HitEffectInstance e;
    e.x = worldX;
    e.y = worldY;
    // 撃破エフェクトは通常の弱点ヒットより大きくする
    e.scale = (chosen == g_hitEffectTexture) ? 1.5f : 3.0f;
    e.timer = 0.0f;
    // より劇的に見えるようフレーム時間を少し遅くする
    e.frameTime = 0.10f;
    e.frame = 0;
    e.active = true;
    e.texture = chosen;

    if (chosen == g_hitEffectTexture) {
        e.rows = 1;
        e.columns = 1;
        e.frameCount = 1;
    }
    else {
        e.rows = WEAKPOINT_HIT_EFFECT_ROWS;
        e.columns = WEAKPOINT_HIT_EFFECT_COLUMNS;
        e.frameCount = WEAKPOINT_HIT_EFFECT_FRAMES;
    }
    g_weakPointHitEffects.push_back(e);
}

HWND g_gameHwnd = nullptr;

// slash-count follower のスポーン要求（敵撃破時に設定）
float g_slashCountSpawnX = 0.0f;
float g_slashCountSpawnY = 0.0f;
bool g_slashCountSpawnPending = false;

void SetGameWindowHandle(HWND hwnd) {
    g_gameHwnd = hwnd;
}

GameCursor g_gameCursor;

void GameCursor::Initialize(ID3D11ShaderResourceView* texture) {
    m_texture = texture;
    // MouseIndicatorSystem が従来使っていたカーソル配置に合わせる。
    // カーソルテクスチャはマウスのワールド位置を左上として描画される。
    m_offsetX = -m_width * 0.5f;
    m_offsetY = -m_height * 0.5f;
}

void GameCursor::Render(float cameraX, float cameraY) {
    if (!m_visible || !m_texture) return;

    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY);

    auto worldToScreen = [cameraX, cameraY](float worldX, float worldY) -> std::pair<float, float> {
        return { worldX - cameraX, worldY - cameraY };
        };

    auto pos = worldToScreen(mouseX + m_offsetX, mouseY + m_offsetY);
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    RenderImage(pos.first, pos.second, m_width, m_height, m_texture, 0, 1, 1, false, 0.0f);
}

void SetInGameCursorEnabled(bool enabled)
{
    g_gameCursor.SetVisible(enabled);

    // ShowCursor の内部カウンタを安定させるため、望む表示状態になるまで呼び出す。
    const bool showOsCursor = !enabled;
    if (showOsCursor) {
        while (ShowCursor(TRUE) < 0) {}
    }
    else {
        while (ShowCursor(FALSE) >= 0) {}
    }
}

// ...既存コード...

static void SpawnPlayerAfterImage() {
    ID3D11ShaderResourceView* tex = g_player.anim.GetCurrentClipTexture();
    if (!tex) return;

    float scale = 6.6f;
    float width = PLAYER_WIDTH * scale;
    float height = PLAYER_HEIGHT * scale;
    float offsetX = (width - PLAYER_WIDTH) * 0.5f;
    float offsetY = (height - PLAYER_HEIGHT) * 0.5f;

    AfterImageInstance a;
    a.x = g_player.posX - offsetX;
    a.y = g_player.posY - offsetY;
    a.width = width;
    a.height = height;
    a.timer = 0.0f;
    a.duration = g_player.AFTERIMAGE_DURATION;
    a.texture = tex;
    a.frameIndex = g_player.anim.GetCurrentFrame();
    a.splitX = g_player.anim.GetSplitX();
    a.splitY = g_player.anim.GetSplitY();

    // プレイヤー描画ロジックと同じ向き情報を使う。
    // 壁滑り中は facingRight が見た目の向きと一時的にずれることがある。
    bool facingRightForAfterImage = g_player.facingRight;
    if (g_player.isWallSliding && g_player.wallSlideDirection != 0) {
        // Player.cpp の wall-slide 判定では:
        //   左壁  => wallSlideDirection = -1, facingRight = true
        //   右壁  => wallSlideDirection =  1, facingRight = false
        facingRightForAfterImage = (g_player.wallSlideDirection == -1);
    }

    a.flipHorizontal = !facingRightForAfterImage;
    g_playerAfterImages.push_back(a);
}

// GameTimer の実装
GameTimer::GameTimer()
{
    __int64 countsPerSec;
    QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
    m_secondsPerCount = 1.0 / static_cast<double>(countsPerSec);
}

void GameTimer::Tick() {
    QueryPerformanceCounter((LARGE_INTEGER*)&m_currTime);
    if (m_prevTime != 0) {
        m_deltaTime = static_cast<float>((m_currTime - m_prevTime) * m_secondsPerCount);
    }
    m_prevTime = m_currTime;
    m_totalTime = (m_currTime - m_startTime) * m_secondsPerCount;
}

float GameTimer::GetDeltaTime() const {
    return m_deltaTime;
}

// スローモーション効果を発動する
void TriggerSlowMotion(float duration = 1.0f, float factor = 0.3f) {
    g_isSlowMotion = true;
    g_slowMoTimer = duration;
    g_slowMoFactor = factor;
}

// ResetGame: 既定ではソフトリセットを行う。fullReload が true のときは
// 敵もクリアして現在のマップを再読込する（非ボスステージでの死亡時の完全復帰や
// プレイヤーによる手動リセット時に有用）。
void ResetGame(bool fullReload) {
    // 一時的な射弾と視覚効果は常に消す
    g_projectileManager.ClearAll();
    g_weakPointHitEffects.clear();
    g_playerAfterImages.clear();

    if (fullReload) {
        // 完全クリーンアップ / 再読込: 全敵を破棄してマップから再生成する
        CleanupEnemies();
        if (g_mapManager.IsMapLoaded()) {
            g_mapManager.ReloadCurrentMap();
        }
    }

    g_player.comboCount = 0;
    g_player.comboTimer = 0.0f;

    // チャージ状態（現在のチャージ / 保存済みチャージ）をリセットする
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
    g_player.hitStopTriggered = 0;
    g_player.hitStopTimer = 0.0f;
    g_player.savedChargeTime = 0.0f;
    g_player.hasSavedCharge = false;
    g_player.chargeDecayTimer = 0.0f;

    // レベル遷移をまたいでゲージポイントと無敵状態を維持する。
    // ここでは g_player.gaugePoints / g_player.isInvincible / g_player.isGaugeInvincible /
    // g_player.invincibleTimer をリセットしない。

    g_gameState = STATE_PLAYING;
}

// 12 月 11 日追加
void CleanUpGameWorld()
{
    g_projectileManager.ClearAll();
    CleanupEnemies();
    g_mouseIndicator.Cleanup();
    g_weakPointHitEffects.clear();
    g_playerAfterImages.clear();
    // 注意: マップ切替時にゲージ関連のパーティクル / 状態を保持し、
    // 新しいレベル突入時にプレイヤーのゲージ（ポイントや有効中エフェクト）が
    // 意図せずリセットされないようにする。ここではゲージ用パーティクル配列や
    // スポーンタイマーをクリアしない。

    // すべてのテクスチャを解放する - 右向きテクスチャのみ残す
    ReleaseTexture(g_playerTexture);
    // 右向き側のテクスチャだけ残す
    ReleaseTexture(g_playerIdleTexture);
    ReleaseTexture(g_playerJumpTexture);
    ReleaseTexture(g_playerRunTexture);
    ReleaseTexture(g_playerSlash1Texture);
    ReleaseTexture(g_playerSlash2Texture);
    ReleaseTexture(g_playerSlash3Texture);
    ReleaseTexture(g_playerSlash4Texture);
    ReleaseTexture(g_playerAirChargeTexture);
    ReleaseTexture(g_playerFallingTexture);
    ReleaseTexture(g_playerGroundChargeTexture);
    ReleaseTexture(g_playerWallSlideTexture);
    ReleaseTexture(g_playerDeathTexture);

    // プレイヤー無敵時用
    ReleaseTexture(g_invinciblePlayerIdleTexture);
    ReleaseTexture(g_invinciblePlayerJumpTexture);
    ReleaseTexture(g_invinciblePlayerRunTexture);
    ReleaseTexture(g_invinciblePlayerSlash1Texture);
    ReleaseTexture(g_invinciblePlayerSlash2Texture);
    ReleaseTexture(g_invinciblePlayerSlash3Texture);
    ReleaseTexture(g_invinciblePlayerSlash4Texture);
    ReleaseTexture(g_invinciblePlayerAirChargeTexture);
    ReleaseTexture(g_invinciblePlayerFallingTexture);
    ReleaseTexture(g_invinciblePlayerGroundChargeTexture);
    ReleaseTexture(g_invinciblePlayerWallSlideTexture);

    // 地形タイル用テクスチャ
    ReleaseTexture(g_groundBlackTexture);
    ReleaseTexture(g_groundTexture);
    ReleaseTexture(g_groundTopTexture);
    ReleaseTexture(g_groundTopCornerLeftTexture);
    ReleaseTexture(g_groundTopCornerRightTexture);
    ReleaseTexture(g_groundLeftTexture);
    ReleaseTexture(g_groundRightTexture);
    ReleaseTexture(g_groundBottomTexture);
    ReleaseTexture(g_groundBottomCornerLeftTexture);
    ReleaseTexture(g_groundBottomCornerRightTexture);
    ReleaseTexture(g_groundCornerFacingTopLeftTexture);
    ReleaseTexture(g_groundCornerFacingTopRightTexture);
    ReleaseTexture(g_groundCornerFacingBottomLeftTexture);
    ReleaseTexture(g_groundCornerFacingBottomRightTexture);
    ReleaseTexture(g_tutorialTexture);

    // ボスステージの装飾用
    ReleaseTexture(g_bossDecorationTexture);

    ReleaseTexture(g_goalTexture);
    ReleaseTexture(g_oneWayPlatformTexture);
    ReleaseTexture(g_backgroundTexture1);
    ReleaseTexture(g_dashEffectTexture);
    ReleaseTexture(g_chargeEffectTexture);

    ReleaseTexture(g_signWASDTexture);
    ReleaseTexture(g_signSTexture);
    ReleaseTexture(g_signRedTexture);
    ReleaseTexture(g_signPinkTexture);
    ReleaseTexture(g_signLongClickTexture);
    ReleaseTexture(g_signClickTexture);
    ReleaseTexture(g_signReleaseTexture);
    ReleaseTexture(g_signRightTexture);
    ReleaseTexture(g_signESCTexture);

    // ヒットエフェクト用テクスチャを解放する
    ReleaseTexture(g_hitEffectTexture);
    for (auto& t : g_slashFlashTextures) {
        ReleaseTexture(t);
    }
    ReleaseTexture(g_numberTexture);
    ReleaseTexture(g_uiNumberTexture);
    ReleaseTexture(g_arrowTexture);
    ReleaseTexture(g_cursorTexture);

    ReleaseTexture(g_escTexture);

    // コンボ用テクスチャ
    ReleaseTexture(g_comboNumberTexture);
    ReleaseTexture(g_comboXTexture);
    ReleaseTexture(g_comboRemainingTimeTexture);

    // ゲージバー用
    ReleaseTexture(g_gaugeBarTexture);
    ReleaseTexture(g_gaugeBarFilledTexture);
    ReleaseTexture(g_gaugeFullEffectTexture);
    ReleaseTexture(g_gaugeTrailParticleTexture);
    ReleaseTexture(g_gaugeKillParticleRedTexture);

    ReleaseTexture(g_attackCountTestTexture);

    ReleaseTexture(g_bossHealthBarTexture);
    ReleaseTexture(g_bossInnerHPTexture);
    // 木製プラットフォーム背景を解放する
    ReleaseTexture(g_platformWoodTexture);
}

// 改良版衝突判定関数
bool CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
        y1 < y2 + h2 && y1 + h1 > y2);
}

// 敵命中時のプレイヤー用コンボ UI
void DrawComboUI(void)
{
    if (g_player.comboCount < 1)
    {
        return;
    }

    // コンボ X 記号 UI
    InGameUI comboXUI;
    comboXUI.x = 0.3f;
    comboXUI.y = 0.45f;
    comboXUI.width = 0.45f;
    comboXUI.height = 0.5f;

    // コンボ数字 UI
    InGameUI comboDigitUI;
    comboDigitUI.width = 0.15f;
    comboDigitUI.height = 0.35f;
    comboDigitUI.y = 0.55f;

    // 間隔設定
    float spaceBetweenDigits = 0.12f;
    float spaceBetweenXandDigit = 0.08f;

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // "X" 記号を描画する
    RenderImage(comboXUI.x, comboXUI.y, comboXUI.width, comboXUI.height, g_comboXTexture, 0, 1, 1);

    char buffer[32];
    sprintf_s(buffer, "%d", g_player.comboCount); // 数値を各桁へ変換する

    float digitXaxis = comboXUI.x + (comboXUI.width * 0.5f) + spaceBetweenXandDigit; // 先頭桁の X 座標

    // コンボ数を描画する
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        int digit = buffer[i] - '0';  // 1 はフレーム 1、2 はフレーム 2、3 はフレーム 3 ...
        RenderImage(digitXaxis, comboDigitUI.y, comboDigitUI.width, comboDigitUI.height,
            g_comboNumberTexture, digit, 1, 10);

        digitXaxis += spaceBetweenDigits; // 次の桁位置へ進める
    }

    // コンボ残り時間バー（コンボ UI の下）
    // comboTimer は COMBO_RESET_TIME から 0 へ向けて減少する。
    float ratio = 0.0f;
    if (g_player.COMBO_RESET_TIME > 0.0f) {
        ratio = g_player.comboTimer / g_player.COMBO_RESET_TIME;
    }
    ratio = std::clamp(ratio, 0.0f, 1.0f);

    // コンボタイマーバー UI
    InGameUI comboTimerBarUI;
    comboTimerBarUI.width = 0.28f;
    comboTimerBarUI.height = 0.025f;
    comboTimerBarUI.x = comboXUI.x + 0.2f;
    comboTimerBarUI.y = 0.5f - 0.02f;

    // 背景
    SetColor(0.05f, 0.05f, 0.05f, 0.75f);
    RenderImage(comboTimerBarUI.x, comboTimerBarUI.y, comboTimerBarUI.width, comboTimerBarUI.height, g_comboRemainingTimeTexture, 0, 1, 1);

    // 塗り部分
    SetColor(1.0f, 0.0f, 0.0f, 0.95f);
    RenderImage(comboTimerBarUI.x - (comboTimerBarUI.width * (1.0f - ratio) * 0.5f), comboTimerBarUI.y,
        comboTimerBarUI.width * ratio, comboTimerBarUI.height, g_comboRemainingTimeTexture, 0, 1, 1);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

// ゲージバー UI 用
void DrawGaugeUI(void)
{
    // ゲージバー外枠用
    InGameUI gaugeFrameUI;
    gaugeFrameUI.x = -1.0f;
    gaugeFrameUI.y = -0.5f;
    gaugeFrameUI.width = 0.35f;
    gaugeFrameUI.height = 1.15f;

    // ゲージバー内側用
    float barOffsetX = -0.0015f;  // 正なら右、負なら左へ移動
    float barOffsetY = 0.32f;    // 正なら上、負なら下へ移動

    // ゲージバー UI 構造体（内側の充填部分）
    InGameUI gaugeBarUI;
    gaugeBarUI.width = gaugeFrameUI.width * 0.5f;
    gaugeBarUI.height = gaugeFrameUI.height * 0.49f;
    // 水平方向中央寄せ、下端基準
    gaugeBarUI.x = gaugeFrameUI.x + (gaugeFrameUI.width - gaugeBarUI.width) * 0.5f + barOffsetX;
    gaugeBarUI.y = gaugeFrameUI.y + (gaugeFrameUI.height - gaugeBarUI.height) * 0.0f + barOffsetY;

    // ゲージバー外枠の描画
    SetColor(1, 1, 1, 1);
    if (g_gaugeBarTexture)
        RenderImage(gaugeFrameUI.x, gaugeFrameUI.y, gaugeFrameUI.width, gaugeFrameUI.height,
            g_gaugeBarTexture, 0, 1, 1);

    // ゲージ満タン時のエフェクトアニメを描画する
    if (g_player.g_gaugeEffectActive)
    {
        if (g_gaugeEffectAnim.GetClipCount() > 0)
        {
            ID3D11ShaderResourceView* tex = g_gaugeEffectAnim.GetCurrentClipTexture();
            if (tex)
            {
                int rows = g_gaugeEffectAnim.GetSplitY();
                int columns = g_gaugeEffectAnim.GetSplitX();
                if (rows > 0 && columns > 0)
                {
                    int currentFrame = g_gaugeEffectAnim.GetCurrentFrame();

                    RenderImage(gaugeFrameUI.x, gaugeFrameUI.y, gaugeFrameUI.width, gaugeFrameUI.height,
                        tex, currentFrame, rows, columns,
                        false, 0.0f, false);
                }
            }
        }
    }

    // 充填バー計算用
    float fillRatio = 0.0f;

    // 通常プレイ時: ゲージポイントから計算する
    if (g_player.MAX_GAUGE_POINTS > 0) {
        fillRatio = (float)g_player.gaugePoints / (float)g_player.MAX_GAUGE_POINTS;
    }

    // 無敵中は減少進行を上から下へ表現する
    if (g_player.isInvincible && g_player.isGaugeInvincible)
    {
        float drainProgress = g_player.invincibleTimer / g_player.INVINCIBLE_DURATION;
        fillRatio = drainProgress;  // 上から下へ減っていく
        if (fillRatio < 0.0f) {
            fillRatio = 0.0f;
        }
    }

    // ゲージバーの充填部分を描画する
    if (fillRatio > 0.0f && g_gaugeBarFilledTexture)
    {
        if (fillRatio > 1.0f) {
            fillRatio = 1.0f;
        }
        SetColor(1, 1, 1, 1);
        RenderGaugeFillImage(gaugeBarUI.x, gaugeBarUI.y, gaugeBarUI.width, gaugeBarUI.height,
            g_gaugeBarFilledTexture, fillRatio);
    }

    SetColor(1, 1, 1, 1);
}

// スコア UI 用
void DrawScoreUI(void)
{
    if (!g_uiNumberTexture) return;

    InGameUI scoreUI;
    scoreUI.x = -0.768f;
    scoreUI.y = 0.66f;
    scoreUI.width = 0.03f;
    scoreUI.height = 0.07f;

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    int enemyPoints = g_gameStats.GetTotalEnemyPoints();

    // 敵ポイントを描画する
    DrawNumber(enemyPoints, scoreUI.x, scoreUI.y, scoreUI.width, scoreUI.height, g_numberTexture);
}


// ゲーム初期化
void InitGameWorld() {
    g_projectileManager.LoadTextures(g_pDevice);

    // プレイヤー用
    LoadTexture(g_pDevice, "asset/character/idle_right.png", &g_playerIdleTexture);
    LoadTexture(g_pDevice, "asset/character/jump_right.png", &g_playerJumpTexture);
    LoadTexture(g_pDevice, "asset/character/run_right.png", &g_playerRunTexture);
    LoadTexture(g_pDevice, "asset/character/slash_right1.png", &g_playerSlash1Texture);
    LoadTexture(g_pDevice, "asset/character/slash_right2.png", &g_playerSlash2Texture);
    LoadTexture(g_pDevice, "asset/character/slash_right3.png", &g_playerSlash3Texture);
    LoadTexture(g_pDevice, "asset/character/slash_right4.png", &g_playerSlash4Texture);
    LoadTexture(g_pDevice, "asset/character/air_charge_right.png", &g_playerAirChargeTexture);
    LoadTexture(g_pDevice, "asset/character/falling_right.png", &g_playerFallingTexture);
    LoadTexture(g_pDevice, "asset/character/ground_charge_right.png", &g_playerGroundChargeTexture);
    LoadTexture(g_pDevice, "asset/character/wall_slide_right.png", &g_playerWallSlideTexture);
    LoadTexture(g_pDevice, "asset/character/death_right.png", &g_playerDeathTexture);
    // アニメーションクリップへ共通名を付ける（左右を区別しない）
    g_player.anim.AddClip("Idle", 0, 3, 4, 1, 0.25f, true, g_playerIdleTexture);
    g_player.anim.AddClip("Jump", 0, 10, 11, 1, 0.06f, false, g_playerJumpTexture);
    g_player.anim.AddClip("Run", 0, 3, 4, 1, 0.1f, true, g_playerRunTexture);
    g_player.anim.AddClip("Slash1", 0, 3, 4, 1, 0.06f, false, g_playerSlash1Texture);
    g_player.anim.AddClip("Slash2", 0, 3, 4, 1, 0.06f, false, g_playerSlash2Texture);
    g_player.anim.AddClip("Slash3", 0, 3, 4, 1, 0.06f, false, g_playerSlash3Texture);
    g_player.anim.AddClip("Slash4", 0, 3, 4, 1, 0.06f, false, g_playerSlash4Texture);
    g_player.anim.AddClip("AirCharge", 0, 0, 1, 1, 0.25f, true, g_playerAirChargeTexture);
    g_player.anim.AddClip("Falling", 0, 0, 1, 1, 0.25f, true, g_playerFallingTexture);
    g_player.anim.AddClip("GroundCharge", 0, 0, 1, 1, 0.25f, true, g_playerGroundChargeTexture);
    g_player.anim.AddClip("WallSlide", 0, 0, 1, 1, 0.25f, true, g_playerWallSlideTexture);
    g_player.anim.AddClip("Death", 0, 10, 11, 1, 0.1f, false, g_playerDeathTexture);


    // プレイヤー無敵時用
    LoadTexture(g_pDevice, "asset/character_invincible/lb_idle_right.png", &g_invinciblePlayerIdleTexture);
    LoadTexture(g_pDevice, "asset/character_invincible/lb_jump_right.png", &g_invinciblePlayerJumpTexture);
    LoadTexture(g_pDevice, "asset/character_invincible/lb_run_right.png", &g_invinciblePlayerRunTexture);
    LoadTexture(g_pDevice, "asset/character_invincible/lb_slash_right1.png", &g_invinciblePlayerSlash1Texture);
    LoadTexture(g_pDevice, "asset/character_invincible/lb_slash_right2.png", &g_invinciblePlayerSlash2Texture);
    LoadTexture(g_pDevice, "asset/character_invincible/lb_slash_right3.png", &g_invinciblePlayerSlash3Texture);
    LoadTexture(g_pDevice, "asset/character_invincible/lb_slash_right4.png", &g_invinciblePlayerSlash4Texture);
    LoadTexture(g_pDevice, "asset/character_invincible/lb_air_charge_right.png", &g_invinciblePlayerAirChargeTexture);
    LoadTexture(g_pDevice, "asset/character_invincible/lb_falling_right.png", &g_invinciblePlayerFallingTexture);
    LoadTexture(g_pDevice, "asset/character_invincible/lb_ground_charge_right.png", &g_invinciblePlayerGroundChargeTexture);
    LoadTexture(g_pDevice, "asset/character_invincible/lb_wall_slide_right.png", &g_invinciblePlayerWallSlideTexture);
    // アニメーションクリップへ共通名を付ける（左右を区別しない）
    g_player.anim.AddClip("InvincibleIdle", 0, 3, 4, 1, 0.25f, true, g_invinciblePlayerIdleTexture);
    g_player.anim.AddClip("InvincibleJump", 0, 10, 11, 1, 0.06f, false, g_invinciblePlayerJumpTexture);
    g_player.anim.AddClip("InvincibleRun", 0, 3, 4, 1, 0.1f, true, g_invinciblePlayerRunTexture);
    g_player.anim.AddClip("InvincibleSlash1", 0, 3, 4, 1, 0.06f, false, g_invinciblePlayerSlash1Texture);
    g_player.anim.AddClip("InvincibleSlash2", 0, 3, 4, 1, 0.06f, false, g_invinciblePlayerSlash2Texture);
    g_player.anim.AddClip("InvincibleSlash3", 0, 3, 4, 1, 0.06f, false, g_invinciblePlayerSlash3Texture);
    g_player.anim.AddClip("InvincibleSlash4", 0, 3, 4, 1, 0.06f, false, g_invinciblePlayerSlash4Texture);
    g_player.anim.AddClip("InvincibleAirCharge", 0, 0, 1, 1, 0.25f, true, g_invinciblePlayerAirChargeTexture);
    g_player.anim.AddClip("InvincibleFalling", 0, 0, 1, 1, 0.25f, true, g_invinciblePlayerFallingTexture);
    g_player.anim.AddClip("InvincibleGroundCharge", 0, 0, 1, 1, 0.25f, true, g_invinciblePlayerGroundChargeTexture);
    g_player.anim.AddClip("InvincibleWallSlide", 0, 0, 1, 1, 0.25f, true, g_invinciblePlayerWallSlideTexture);

    // 地面テクスチャ用
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_black.png", &g_groundBlackTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3.png", &g_groundTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_up.png", &g_groundTopTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_dia_left_up.png", &g_groundTopCornerLeftTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_dia_right_up.png", &g_groundTopCornerRightTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_left.png", &g_groundLeftTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_right.png", &g_groundRightTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga_down.png", &g_groundBottomTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_dia_left_down.png", &g_groundBottomCornerLeftTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_dia_right_down.png", &g_groundBottomCornerRightTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_inversiondia_left_up.png", &g_groundCornerFacingTopLeftTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_inversiondia_right_up.png", &g_groundCornerFacingTopRightTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_inversiondia_left_down.png", &g_groundCornerFacingBottomLeftTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_inversiondia_right_down.png", &g_groundCornerFacingBottomRightTexture);
    LoadTexture(g_pDevice, "asset/platform/platformrenga3_oni.png", &g_bossDecorationTexture);
    LoadTexture(g_pDevice, "asset/platform/tutorial2.png", &g_tutorialTexture);

    LoadTexture(g_pDevice, "asset/goal.png", &g_goalTexture);
    LoadTexture(g_pDevice, "asset/platform/platform_pass4.png", &g_oneWayPlatformTexture);
    LoadTexture(g_pDevice, "asset/platform/platformtest.png", &g_comboRemainingTimeTexture);
    LoadTexture(g_pDevice, "asset/UI/boss_HP/boss_HP_background.png", &g_bossHealthBarTexture);
    LoadTexture(g_pDevice, "asset/UI/boss_HP/boss_HP.png", &g_bossInnerHPTexture);
    LoadTexture(g_pDevice, "asset/background/1-6background.png", &g_backgroundTexture1);

    // 標識用
    LoadTexture(g_pDevice, "asset/UI/sign/sign_wasd.png", &g_signWASDTexture);
    signAnim.AddClip("SignWASD", 0, 3, 1, 4, 0.2f, true, g_signWASDTexture);
    LoadTexture(g_pDevice, "asset/UI/sign/sign_s.png", &g_signSTexture);
    signAnim.AddClip("SignS", 0, 3, 1, 4, 0.2f, true, g_signSTexture);
    LoadTexture(g_pDevice, "asset/UI/sign/sign_right.png", &g_signRightTexture);
    signAnim.AddClip("SignRight", 0, 4, 1, 5, 0.08f, true, g_signRightTexture);
    LoadTexture(g_pDevice, "asset/UI/sign/sign_release.png", &g_signReleaseTexture);
    signAnim.AddClip("SignRelease", 0, 3, 1, 4, 0.1f, true, g_signReleaseTexture);
    LoadTexture(g_pDevice, "asset/UI/sign/sign_red_is_safe_seriously_it_really_is.png", &g_signRedTexture);
    signAnim.AddClip("SignRed", 0, 3, 1, 4, 0.2f, true, g_signRedTexture);
    LoadTexture(g_pDevice, "asset/UI/sign/sign_pink.png", &g_signPinkTexture);
    signAnim.AddClip("SignPink", 0, 3, 1, 4, 0.2f, true, g_signPinkTexture);
    LoadTexture(g_pDevice, "asset/UI/sign/sign_long_click.png", &g_signLongClickTexture);
    signAnim.AddClip("SignLongClick", 0, 3, 1, 4, 0.2f, true, g_signLongClickTexture);
    LoadTexture(g_pDevice, "asset/UI/sign/sign_esc.png", &g_signESCTexture);
    signAnim.AddClip("SignESC", 0, 3, 1, 4, 0.2f, true, g_signESCTexture);
    LoadTexture(g_pDevice, "asset/UI/sign/sign_click.png", &g_signClickTexture);
    signAnim.AddClip("SignClick", 0, 3, 1, 4, 0.2f, true, g_signClickTexture);

    LoadTexture(g_pDevice, "asset/UI/number.png", &g_numberTexture);
    LoadTexture(g_pDevice, "asset/UI/UI_score＆time.png", &g_uiNumberTexture);

    LoadTexture(g_pDevice, "asset/UI/arrow.png", &g_arrowTexture);
    LoadTexture(g_pDevice, "asset/UI/cursor.png", &g_cursorTexture);

    LoadTexture(g_pDevice, "asset/UI/combo/combo_number.png", &g_comboNumberTexture);
    LoadTexture(g_pDevice, "asset/UI/combo/combo_X.png", &g_comboXTexture);

    // ダッシュ / slash-count UI（プレイヤー追従）: attack_count.png は 1x3 シート
    LoadTexture(g_pDevice, "asset/UI/attack_count.png", &g_slashCountTexture);
    g_slashCountAnim.AddClip("SlashCount", 0, 2, 1, 3, 0.12f, true, g_slashCountTexture);
    g_slashCountAnim.SetClip("SlashCount");

    // HP follower 用スプライトシート（1x3）
    LoadTexture(g_pDevice, "asset/UI/Health.png", &g_healthTexture);
    g_healthAnim.AddClip("Health", 0, 2, 1, 3, 0.12f, true, g_healthTexture);
    g_healthAnim.SetClip("Health");

    LoadTexture(g_pDevice, "asset/effect/effect_hit.png", &g_hitEffectTexture);

    LoadTexture(g_pDevice, "asset/effect/slash_flash1.png", &g_slashFlashTextures[0]);
    LoadTexture(g_pDevice, "asset/effect/slash_flash2.png", &g_slashFlashTextures[1]);
    LoadTexture(g_pDevice, "asset/effect/slash_flash3.png", &g_slashFlashTextures[2]);
    LoadTexture(g_pDevice, "asset/effect/slash_flash4.png", &g_slashFlashTextures[3]);

    // 背景テクスチャ: シンプルなタイル背景
    LoadTexture(g_pDevice, "asset/platform/platformrenga3.png", &g_backgroundTexture1); // world1 / world2 用
    LoadTexture(g_pDevice, "asset/platform/platformwood.png", &g_platformWoodTexture); // world3 用

    // ゲージバー用
    LoadTexture(g_pDevice, "asset/UI/gauge/gauge_frame.png", &g_gaugeBarTexture);
    LoadTexture(g_pDevice, "asset/UI/gauge/gauge_filled.png", &g_gaugeBarFilledTexture);
    LoadTexture(g_pDevice, "asset/UI/gauge/gauge_effect.png", &g_gaugeFullEffectTexture);
    g_gaugeEffectAnim.AddClip("GaugeFull", 0, 9, 10, 1, 0.08f, true, g_gaugeFullEffectTexture);

    // ゲージモード用トレイルパーティクル（1x5）
    LoadTexture(g_pDevice, "asset/effect/particle_sheet.png", &g_gaugeTrailParticleTexture);

    // ゲージ撃破バーストパーティクル（1x5）
    LoadTexture(g_pDevice, "asset/effect/particle_sheet_red.png", &g_gaugeKillParticleRedTexture);

    LoadTexture(g_pDevice, "asset/UI/attack_count.png", &g_attackCountTestTexture);

    LoadTexture(g_pDevice, "asset/UI/UI_esc.png", &g_escTexture);


    InitEnemies();
    g_mapManager.InitializeMaps();
    g_mouseIndicator.Initialize();
    g_gameCursor.Initialize(g_cursorTexture);

    g_camera.SetSmoothness(camera_Smoothness);
    g_camera.SetLookAhead(camera_LookAhead);
    g_camera.SetDeadZone(camera_DeadZone);
}

// 修正版ゲーム更新関数
void UpdateGame(float deltaTime) {
    if (g_gameState != STATE_PLAYING) {
        return;
    }
    // `animLockDuration` は固定時間、`animLockTimer` は進行中のカウントダウン。
    // duration 自体を減らすとロック / 解除ロジックが壊れ、アニメ遷移が止まることがある。
    if (g_player.animLockTimer > 0.0f) {
        g_player.animLockTimer -= deltaTime;
        if (g_player.animLockTimer < 0.0f) {
            g_player.animLockTimer = 0.0f;
        }
    }

    // オーディオマネージャーを更新する
    g_gameTimer.Tick(); // 12 月 3 日追加

    signAnim.Update(deltaTime);
    g_slashCountAnim.Update(deltaTime);

    // 12 月 4 日追加
    g_gameElapsedTime += deltaTime;
    g_gameMinutes = static_cast<int>(g_gameElapsedTime) / 60;
    g_gameSeconds = static_cast<int>(g_gameElapsedTime) % 60;

    g_gameStats.UpdateTime(g_gameElapsedTime); // 総経過時間を記録する

    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY);

    // プレイヤー位置からマウス位置へ fireball を撃つ
    /*g_projectileManager.CreateFireball(
        g_player.posX + PLAYER_WIDTH / 2,  // プレイヤー中心から発射
        g_player.posY + PLAYER_HEIGHT / 2,
        mouseX,
        mouseY,
        true  // プレイヤー発射
    );*/

    // スローモーション効果を更新する
    if (g_isSlowMotion) {
        g_slowMoTimer -= deltaTime;
        if (g_slowMoTimer <= 0.0f) {
            g_isSlowMotion = false;
            g_slowMoFactor = 1.0f; // 通常時間へ戻す
        }
    }

    // 時間倍率効果を適用する（優先度: 全体スロー > ダッシュ終了スロー > チャージ効果）
    float timeScale = 1.0f;
    if (g_isSlowMotion) {
        timeScale = g_slowMoFactor; // スローモーション倍率を使う

    }
    else if (g_player.isInDashEndSlowMo) {
        timeScale = g_player.DASH_END_SLOWMO_FACTOR;
    }
    else if (g_player.isCharging) {
        float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;
        chargeRatio = std::min(chargeRatio * 8, 1.0f);
        timeScale = 1.0f - chargeRatio * 0.8f;
    }


    // コンボタイマー更新用
    if (g_player.comboCount > 0) {
        g_player.comboTimer -= deltaTime;
        if (g_player.comboTimer <= 0.0f) {
            g_player.comboCount = 0;
            g_player.comboTimer = 0.0f;
        }
        g_gameStats.UpdateMaxCombo(g_player.comboCount);
    }

    // 加速状態:
    // - comboCount がしきい値を超えたとき有効
    // - ゲージ由来無敵中（キル継続報酬のゲージ / フルゲージ中）も有効
    g_player.isAccelerated = (g_player.comboCount > ACCEL_COMBO_THRESHOLD) || (g_player.isInvincible && g_player.isGaugeInvincible);

    // 無敵タイマー更新用
    if (g_player.isInvincible) {
        const float prevInvTime = g_player.invincibleTimer;
        g_player.invincibleTimer -= deltaTime;

        // ゲージ無敵: 残り 1 秒警告（1 回だけ再生）
        if (g_player.isGaugeInvincible && prevInvTime > 1.0f && g_player.invincibleTimer <= 1.0f) {
            Audio::PlaySE(SoundEffect::INVINCIBLE_WARNING);
        }

        if (g_player.invincibleTimer <= 0.0f) {
            g_player.isInvincible = false;
            g_player.isGaugeInvincible = false;
            g_player.invincibleTimer = 0.0f;
        }
    }

    // ゲージ満タン時に自動で無敵を発動する
    if (!g_player.isInvincible && g_player.gaugePoints >= g_player.MAX_GAUGE_POINTS) {
        g_player.isInvincible = true;
        g_player.isGaugeInvincible = true;
        g_player.invincibleTimer = g_player.INVINCIBLE_DURATION;
        g_player.g_gaugeEffectActive = true; 
        g_player.g_gaugeEffectTimer = g_player.INVINCIBLE_DURATION;
        g_player.gaugePoints = 0;

        Audio::PlaySE(SoundEffect::LIMITBREAK, 2.0f);
        // ゲージ発動時、プレイヤー中心へ派手なパーティクルバーストを出す
        {
            float centerX = g_player.posX + PLAYER_WIDTH * 0.5f;
            float centerY = g_player.posY + PLAYER_HEIGHT * 0.5f;
            SpawnGaugeFullBurst(centerX, centerY);
        }
    }

    // ゲージエフェクトタイマー用
    if (g_player.g_gaugeEffectActive) { 
        g_gaugeEffectAnim.Update(deltaTime); 
        g_player.g_gaugeEffectTimer -= deltaTime;

        if (g_player.g_gaugeEffectTimer <= 0.0f) 
        { 
            g_player.g_gaugeEffectActive = false; 
            g_gaugeEffectAnim.Reset(); 
        } 
    }

    float scaledDeltaTime = deltaTime * timeScale;

    // ゲージトレイルパーティクル: ゲージ由来無敵中のみ有効
    if (g_player.isInvincible && g_player.isGaugeInvincible && !g_player.isDead) {
        g_gaugeTrailSpawnTimer -= scaledDeltaTime;
        if (g_gaugeTrailSpawnTimer <= 0.0f) {
            // プレイヤーの体の周囲へ生成する（楕円内ランダム）
            const float r01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            const float r02 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            const float ang = r01 * 6.2831853f;
            const float rad = sqrtf(r02);

            const float rx = PLAYER_WIDTH * 0.75f;
            const float ry = PLAYER_HEIGHT * 0.75f;
            const float offsetX = cosf(ang) * rad * rx;
            const float offsetY = sinf(ang) * rad * ry;

            const float spawnX = g_player.posX + PLAYER_WIDTH * 0.5f + offsetX;
            const float spawnY = g_player.posY + PLAYER_HEIGHT * 0.5f + offsetY;
            SpawnGaugeTrailParticle(spawnX, spawnY);

            // 1.5 倍の発生率 => 間隔は短くなる
            g_gaugeTrailSpawnTimer = GAUGE_TRAIL_SPAWN_INTERVAL / GAUGE_TRAIL_SPAWN_RATE_MULT;
        }
    }
    else {
        g_gaugeTrailSpawnTimer = 0.0f;
    }

    for (auto it = g_gaugeTrailParticles.begin(); it != g_gaugeTrailParticles.end();) {
        if (!it->active || !it->texture) {
            it = g_gaugeTrailParticles.erase(it);
            continue;
        }

        it->x += it->vx * scaledDeltaTime;
        it->y += it->vy * scaledDeltaTime;
        it->rotation += it->angularVelocity * scaledDeltaTime;

        it->frameTimer += scaledDeltaTime;
        while (it->frameTimer >= it->frameTime) {
            it->frameTimer -= it->frameTime;
            it->frame++;
        }

        if (it->frame >= GaugeTrailParticleInstance::frameCount) {
            it = g_gaugeTrailParticles.erase(it);
        }
        else {
            ++it;
        }
    }

    for (auto it = g_gaugeKillParticlesRed.begin(); it != g_gaugeKillParticlesRed.end();) {
        if (!it->active || !it->texture) {
            it = g_gaugeKillParticlesRed.erase(it);
            continue;
        }

        it->x += it->vx * scaledDeltaTime;
        it->y += it->vy * scaledDeltaTime;
        it->rotation += it->angularVelocity * scaledDeltaTime;

        it->frameTimer += scaledDeltaTime;
        while (it->frameTimer >= it->frameTime) {
            it->frameTimer -= it->frameTime;
            it->frame++;
        }

        if (it->frame >= GaugeKillParticleInstance::frameCount) {
            it = g_gaugeKillParticlesRed.erase(it);
        }
        else {
            ++it;
        }
    }

    // プレイヤー残像の更新 / 生成
    for (auto it = g_playerAfterImages.begin(); it != g_playerAfterImages.end();) {
        it->timer += scaledDeltaTime;
        if (it->timer >= it->duration) {
            it = g_playerAfterImages.erase(it);
        }
        else {
            ++it;
        }
    }

    // 入力 / 物理更新で速度が一時的に上書き・リセットされても残像が出るよう、
    // 変位ベースの速度を使う。
    static float s_prevPlayerX = g_player.posX;
    static float s_prevPlayerY = g_player.posY;
    float dtForSpeed = (scaledDeltaTime > 1e-6f) ? scaledDeltaTime : 1e-6f;
    float dx = g_player.posX - s_prevPlayerX;
    float dy = g_player.posY - s_prevPlayerY;
    float speed = sqrtf(dx * dx + dy * dy) / dtForSpeed;
    s_prevPlayerX = g_player.posX;
    s_prevPlayerY = g_player.posY;

    // 残像:
    // - ダッシュ中は常に出す
    // - 加速状態（コンボ）では移動 / 走行中にも出す
    // - 非加速時のダッシュ残像は半分の頻度で出す（間隔 2 倍）
    bool shouldSpawnAfterImage = !g_player.isDead && (g_player.isDashing || g_player.isAccelerated);
    if (shouldSpawnAfterImage) {
        float interval = g_player.afterImageSpawnInterval;
        // 非加速ダッシュ時のみ残像発生率を下げる
        if (g_player.isDashing && !g_player.isAccelerated) {
            interval *= 2.0f;
        }

        // 加速だけでダッシュしていない場合は、実際の移動があるときだけ出す
        if (!g_player.isDashing && g_player.isAccelerated && !g_player.isMoving) {
            g_player.afterImageSpawnTimer = 0.0f;
        }
        else {
            g_player.afterImageSpawnTimer -= scaledDeltaTime;
            if (g_player.afterImageSpawnTimer <= 0.0f) {
                SpawnPlayerAfterImage();
                g_player.afterImageSpawnTimer = interval;
            }
        }
    }
    else {
        g_player.afterImageSpawnTimer = 0.0f;
    }

    for (auto it = g_weakPointHitEffects.begin(); it != g_weakPointHitEffects.end();) {
        if (!it->active) {
            it = g_weakPointHitEffects.erase(it);
            continue;
        }

        it->timer += scaledDeltaTime;
        while (it->timer >= it->frameTime) {
            it->timer -= it->frameTime;
            it->frame++;
        }

        if (it->frame >= it->frameCount) {
            it = g_weakPointHitEffects.erase(it);
        }
        else {
            ++it;
        }
    }

    g_camera.Update(scaledDeltaTime);
	g_player.hitStopTimer -= scaledDeltaTime;
    if (g_player.hitStopTimer <= 0.0f) {
        // 調整後時間でゲームロジックを更新する
        UpdateDash(deltaTime);
        UpdatePlayerPhysics(scaledDeltaTime);

        UpdateEnemies(scaledDeltaTime, &g_mapManager);
        // すべての射弾を更新する
        g_projectileManager.Update(scaledDeltaTime, &g_mapManager, g_enemies);
        // UpdateGame 関数内のアニメ設定部分
        if (g_player.animLockTimer <= 0.0f)
        {
            // 無敵アニメセットはゲージ由来無敵中にだけ使う。
            // ダッシュ / slash 後の短い無敵では現在アニメを上書きしない。
            if (g_player.isInvincible && g_player.isGaugeInvincible)
            {
                // 無敵時アニメーション
                if (g_player.isDead)
                {
                    // 死亡アニメは通常版のまま（無敵用 death テクスチャは使わない）
                    if (g_player.anim.GetCurrentClipName() != "Death") {
                        g_player.anim.SetClip("Death");
                    }
                }
                else if (g_player.isCharging && !g_player.isOnGround)
                {
                    if (g_player.anim.GetCurrentClipName() != "InvincibleAirCharge") {
                        g_player.anim.SetClip("InvincibleAirCharge");
                    }
                }
                else if (g_player.isCharging)
                {
                    if (g_player.anim.GetCurrentClipName() != "InvincibleGroundCharge") {
                        g_player.anim.SetClip("InvincibleGroundCharge");
                    }
                }
                else if (g_player.isDashing)
                {
                    // slash の方向を判定する
                    float dx = g_player.dashDirectionX;
                    float dy = g_player.dashDirectionY;

                    const float DIAG_RATIO = 0.70710678f;
                    const float adx = fabsf(dx);
                    const float ady = fabsf(dy);

                    const char* clip;
                    if (dy > 0.0f && ady >= adx * DIAG_RATIO) {
                        clip = "InvincibleSlash2";  // 上
                    }
                    else if (dy < 0.0f && ady >= adx * DIAG_RATIO) {
                        clip = "InvincibleSlash4";  // 下
                    }
                    else if (adx >= ady * DIAG_RATIO) {
                        clip = "InvincibleSlash3";  // 横
                    }
                    else {
                        clip = "InvincibleSlash1";  // 斜め
                    }

                    if (g_player.anim.GetCurrentClipName() != clip) {
                        g_player.anim.SetClip(clip);
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
                else if (g_player.isWallSliding)
                {
                    if (g_player.anim.GetCurrentClipName() != "InvincibleWallSlide") {
                        g_player.anim.SetClip("InvincibleWallSlide");
                    }
                }
                else if (!g_player.isOnGround)
                {
                    if (g_player.velocityY < 0.0f) // 落下中
                    {
                        if (g_player.anim.GetCurrentClipName() != "InvincibleFalling") {
                            g_player.anim.SetClip("InvincibleFalling");
                        }
                    }
                    else // ジャンプ中
                    {
                        if (g_player.anim.GetCurrentClipName() != "InvincibleJump") {
                            g_player.anim.SetClip("InvincibleJump");
                        }
                    }
                }
                else if (g_player.isMoving)
                {
                    if (g_player.anim.GetCurrentClipName() != "InvincibleRun") {
                        g_player.anim.SetClip("InvincibleRun");
                    }
                }
                else // 待機
                {
                    if (g_player.anim.GetCurrentClipName() != "InvincibleIdle") {
                        g_player.anim.SetClip("InvincibleIdle");
                    }
                }
            }
            else
            {
                if (g_player.isDead) // 死亡時
                {
                    if (g_player.anim.GetCurrentClipName() != "Death") {
                        g_player.anim.SetClip("Death");
                    }
                }

                else if (g_player.isCharging) // プレイヤーがチャージ中なら
                {
                    if (!g_player.isOnGround) // 空中チャージ中なら
                    {
                        if (g_player.anim.GetCurrentClipName() != "AirCharge") {
                            g_player.anim.SetClip("AirCharge");
                        }
                    }
                    else // 地上チャージ中なら
                    {
                        if (g_player.anim.GetCurrentClipName() != "GroundCharge") {
                            g_player.anim.SetClip("GroundCharge");
                        }
                    }
                }
                else if (g_player.isDashing) // プレイヤーがダッシュ中なら
                {
                    // slash クリップはダッシュ方向で決まる。
                    // 45° 境界で 4 区分する:
                    //   上:    |dy| 優勢かつ dy > 0  -> Slash2
                    //   下:    |dy| 優勢かつ dy < 0  -> Slash4
                    //   横:    それ以外で |dx| 優勢 -> Slash3
                    //   斜め下寄りのフォールバック   -> Slash1
                    // 左右は facing / flip で処理する。
                    float dx = g_player.dashDirectionX;
                    float dy = g_player.dashDirectionY;

                    const float DIAG_RATIO = 0.70710678f; // cos(45°)
                    const float adx = fabsf(dx);
                    const float ady = fabsf(dy);

                    const char* clip;
                    if (dy > 0.0f && ady >= adx * DIAG_RATIO) {
                        clip = "Slash2";
                    }
                    else if (dy < 0.0f && ady >= adx * DIAG_RATIO) {
                        clip = "Slash4";
                    }
                    else if (adx >= ady * DIAG_RATIO) {
                        clip = "Slash3";
                    }
                    else {
                        // 斜め寄りなら専用の斜め下 slash を選ぶ
                        clip = "Slash1";
                    }

                    if (g_player.anim.GetCurrentClipName() != clip) {
                        g_player.anim.SetClip(clip);
                        g_player.animLockTimer = g_player.animLockDuration;
                    }
                }
                else if (g_player.isWallSliding) {
                    if (g_player.anim.GetCurrentClipName() != "WallSlide") {
                        g_player.anim.SetClip("WallSlide");
                    }
                }
                else if (!g_player.isOnGround) // プレイヤーが地上にいないなら
                {
                    if (g_player.velocityY < 0.0f) // 落下
                    {
                        if (g_player.anim.GetCurrentClipName() != "Falling") {
                            g_player.anim.SetClip("Falling");
                        }
                    }
                    else // ジャンプ
                    {
                        if (g_player.anim.GetCurrentClipName() != "Jump") {
                            g_player.anim.SetClip("Jump");
                        }
                    }
                }
                else if (g_player.isMoving) // プレイヤーが移動中なら
                {
                    if (g_player.anim.GetCurrentClipName() != "Run") {
                        g_player.anim.SetClip("Run");
                    }
                }
                else // プレイヤーが静止中なら
                {
                    if (g_player.anim.GetCurrentClipName() != "Idle") {
                        g_player.anim.SetClip("Idle");
                    }
                }
            }
        }

        // アニメ速度を加速状態と同期させる
        g_player.anim.Update(scaledDeltaTime * g_player.GetAnimSpeedMultiplier());
        UpdatePlayerDeath(scaledDeltaTime);
    }
    g_mouseIndicator.Update(scaledDeltaTime);
}

// 補助関数: タイルコードに応じたテクスチャを取得する
ID3D11ShaderResourceView* GetTextureForTile(const std::string& tileCode) {
    if (tileCode == "G1" ) {
        return g_groundTexture;
    }
    else if (tileCode == "G2"){
        return g_groundTopTexture;
    }
    else if (tileCode == "G3") {
        return g_groundTopCornerLeftTexture;
    }
    else if (tileCode == "G4") {
        return g_groundTopCornerRightTexture;
    }
    else if (tileCode == "G5") {
        return g_groundLeftTexture;
    }
    else if (tileCode == "G6") {
        return g_groundRightTexture;
    }
    else if (tileCode == "G7") {
        return g_groundBottomTexture;
    }
    else if (tileCode == "G8") {
        return g_groundBottomCornerLeftTexture;
    }
    else if (tileCode == "G9") {
        return g_groundBottomCornerRightTexture;
    }
    else if (tileCode == "C1") {
        return g_groundCornerFacingTopLeftTexture;
    }
    else if (tileCode == "C2") {
        return g_groundCornerFacingTopRightTexture;
    }
    else if (tileCode == "C3") {
        return g_groundCornerFacingBottomLeftTexture;
    }
    else if (tileCode == "C4") {
        return g_groundCornerFacingBottomRightTexture;
    }
    else if (tileCode == "BD") {
        return g_bossDecorationTexture;
    }
    else if (tileCode == "BB") {
        return g_groundBlackTexture;
    }
    else if (tileCode == "W1" || tileCode == "W2") {
        return g_groundTexture;
    }
    else if (tileCode == "P1" || tileCode == "P2") {
        return g_groundTexture;
    }
    else if (tileCode == "DF" || tileCode == "DI" || tileCode == "DT" || tileCode == "D4" || tileCode == "D5" || tileCode == "D6" || tileCode == "D7" || tileCode == "DB" ||
             tileCode == "21" || tileCode == "22" || tileCode == "23" || tileCode == "24" || tileCode == "25" || tileCode == "26" || tileCode == "27" ||
             tileCode == "31" || tileCode == "32" || tileCode == "33" || tileCode == "34" || tileCode == "35" || tileCode == "36" || tileCode == "37") {
        return g_goalTexture;
    }
    else if (tileCode == "B1") {
        return g_signWASDTexture;
    }
    else if (tileCode == "B2") {
        return g_signSTexture;
    }
    else if (tileCode == "B3") {
        return g_signRightTexture;
    }
    else if (tileCode == "B4") {
        return g_signReleaseTexture;
    }
    else if (tileCode == "B5") {
        return g_signRedTexture;
    }
    else if (tileCode == "B6") {
        return g_signPinkTexture;
    }
    else if (tileCode == "B7") {
        return g_signLongClickTexture;
    }
    else if (tileCode == "B8") {
        return g_signClickTexture;
    }
    else if (tileCode == "B9") {
        return g_signESCTexture;
    }
    else if (tileCode == "OP") {
        return g_oneWayPlatformTexture;
    }
    else if (tileCode == "T1" || tileCode == "T2" || tileCode == "T3" || tileCode == "T4") {
        return g_tutorialTexture; // チュートリアルタイル用の地面テクスチャ
    }
    else {
        return g_groundTexture;
    }
}

// 補助関数: タイルコードに応じて色を設定する
void SetTileColor(const std::string& tileCode) {
    if (tileCode == "G1") {
        //SetColor(0.4f, 0.8f, 0.3f, 1.0f);
        //SetColor(0.7f, 0.7f, 0.7f, 1.0f);
    }
    else if (tileCode == "G2") {
        //SetColor(0.6f, 0.4f, 0.2f, 1.0f);
    }
    else if (tileCode == "G3") {
        //SetColor(0.5f, 0.5f, 0.5f, 1.0f);
    }
    else if (tileCode == "W1") {
        SetColor(0.7f, 0.3f, 0.2f, 1.0f);
    }
    else if (tileCode == "W2") {
        SetColor(0.4f, 0.4f, 0.4f, 1.0f);
    }
    else if (tileCode == "P1") {
        SetColor(0.8f, 0.6f, 0.3f, 1.0f);
    }
    else if (tileCode == "P2") {
        SetColor(0.7f, 0.7f, 0.8f, 1.0f);
    }
    else if (tileCode == "PF") {
        SetColor(0.3f, 0.7f, 0.3f, 1.0f);
    }
    else if (tileCode == "PI") {
        SetColor(0.3f, 0.5f, 0.9f, 1.0f);
    }
    else if (tileCode == "PT") {
        SetColor(0.8f, 0.3f, 0.8f, 1.0f);
    }
    else if (tileCode == "D2") {
        SetColor(0.5f, 0.5f, 0.5f, 1.0f);
    }
    else if (tileCode == "OP") {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void DrawGame() {
    // 現在マップに応じたシンプルなタイル背景を描画する
    // 視差やタイル拡大率を指定できるタイル背景描画。
    // parallax: 1.0 = カメラに完全追従、<1.0 = より遅く動く（遠景）
    // tileScale: タイルを大きく描画する（遠景層の繰り返し軽減に有効）
    auto DrawTiledBackground = [&](ID3D11ShaderResourceView* tex, float tileWorldW, float tileWorldH, float parallax = 1.0f, float tileScale = 1.0f) {
        if (!tex) return;

        // 背景タイルが見えているビューポートにそろうよう、カメラ中心基準で配置する。
        // これにより、カメラ先読み時でも背景タイルがずれにくくなる。
        // parallax (<1.0) 使用時は、視差補正後のカメラ位置を使って中心を求め、
        // オフセット増加時も画面周囲にタイルが残るようにする。
        float camX = g_camera.GetX();
        float camY = g_camera.GetY();
        // parallax != 1.0 のとき背景タイルがカメラから流れないよう、
        // 視差補正後の中心を使ってタイル選択する
        float centerWorldX = camX * parallax;
        float centerWorldY = camY * parallax;

        // 表示矩形をワールド単位で計算する（pixel->world スケールはおよそ 100）
        float visibleW = static_cast<float>(g_camera.GetWidth()) / (g_camera.GetZoom() * 100.0f);
        float visibleH = static_cast<float>(g_camera.GetHeight()) / (g_camera.GetZoom() * 100.0f);

        // タイル拡大率を適用する
        float tw = tileWorldW * tileScale;
        float th = tileWorldH * tileScale;

        // タイルインデックス中心をプレイヤー周辺へ合わせる
        int centerX = static_cast<int>(std::floor(centerWorldX / tw));
        int centerY = static_cast<int>(std::floor(centerWorldY / th));

        // 横 / 縦に何枚入るかのおおよその半径を求める
        int halfTilesX = static_cast<int>(std::ceil((visibleW / tw) * 0.5f)) + 1;
        int halfTilesY = static_cast<int>(std::ceil((visibleH / th) * 0.5f)) + 1;

        // 描画タイル数を増やすため半径を広げる（要望: およそ 2 倍）
        const int MAX_RADIUS = 8; // (2*8+1)^2 = 最大 289 タイル
        halfTilesX = std::min(halfTilesX, MAX_RADIUS);
        halfTilesY = std::min(halfTilesY, MAX_RADIUS);

        // 背景を暗くする（ここに置くことで全レイヤーに同じ効果が掛かる）
        // 0.45 から 0.25 に下げ、背景がより暗く見えるようにした
        SetColor(0.25f, 0.25f, 0.25f, 1.0f);

        // 視差補正: 背景はカメラより遅く動くべき
        float parCamX = camX * parallax;
        float parCamY = camY * parallax;

        for (int iy = centerY - halfTilesY; iy <= centerY + halfTilesY; ++iy) {
            for (int ix = centerX - halfTilesX; ix <= centerX + halfTilesX; ++ix) {
                float wx = ix * tw;
                float wy = iy * th;
                auto screen = std::pair<float,float>(wx - parCamX, wy - parCamY);
                RenderImage(screen.first, screen.second, tw, th, tex, 0, 1, 1);
            }
        }

        // 色を元へ戻す
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    };

    // 現在マップに応じて背景テクスチャを選ぶ
    if (g_mapManager.IsMapLoaded()) {
        const std::string& mapName = g_mapManager.GetCurrentMapName();
        if (!mapName.empty()) {
            if (mapName.rfind("World3", 0) == 0) {
                // world3: 遠景の parallax レイヤーだけ描画する（近景レイヤーは削除済み）
                DrawTiledBackground(g_platformWoodTexture, 0.15f, 0.15f, 0.6f, 1.4f);
            }
            else {
                // world1/2: 遠景 parallax レイヤーのみ描画する
                DrawTiledBackground(g_backgroundTexture1, 0.15f, 0.15f, 0.6f, 1.4f);
            }
        }
    }

    int currentWidth = g_camera.GetWidth();
    int currentHeight = g_camera.GetHeight();
    float aspectRatio = static_cast<float>(currentWidth) / static_cast<float>(currentHeight);

    float cameraX = g_camera.GetX();
    float cameraY = g_camera.GetY();

    auto worldToScreen = [cameraX, cameraY](float worldX, float worldY) -> std::pair<float, float> {
        return { worldX - cameraX, worldY - cameraY };
        };

    // HP アイコンアニメを進める
    g_healthAnim.Update(g_gameTimer.GetDeltaTime());

    // slash-count アイコンは地形に隠れないよう、後で描画する

    // プレイヤー残像を描画する（プレイヤーの背後）
    for (const auto& a : g_playerAfterImages) {
        if (!a.texture) continue;
        float t = (a.duration <= 0.0f) ? 1.0f : std::clamp(a.timer / a.duration, 0.0f, 1.0f);
        float alpha = (1.0f - t) * 0.85f;
        float gray = 0.25f + (1.0f - t) * 0.35f;
        auto p = worldToScreen(a.x, a.y);
        SetColor(gray, gray, gray, alpha);
        RenderImage(p.first, p.second, a.width, a.height,
            a.texture, a.frameIndex, a.splitY, a.splitX, true, 0.0f, a.flipHorizontal);
    }
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // ゲージトレイルパーティクルを描画する（プレイヤーの背後）
    for (const auto& pInst : g_gaugeTrailParticles) {
        if (!pInst.active || !pInst.texture) continue;

        float t = static_cast<float>(pInst.frame) / static_cast<float>(GaugeTrailParticleInstance::frameCount);
        t = std::clamp(t, 0.0f, 1.0f);
        // イーズアウトで減衰させ、線形より自然に見せる
        float alpha = (1.0f - t);
        alpha = alpha * alpha;
        alpha *= 0.9f;
        auto p = worldToScreen(pInst.x, pInst.y);

        // ゲージ演出に合わせて少し青白くする
        SetColor(0.85f, 0.95f, 1.0f, alpha);
        float w = PLAYER_WIDTH * 0.9f * pInst.scale;
        float h = PLAYER_HEIGHT * 0.9f * pInst.scale;
        RenderImage(p.first, p.second, w, h,
            pInst.texture,
            pInst.frame,
            GaugeTrailParticleInstance::rows,
            GaugeTrailParticleInstance::columns,
            false,
            pInst.rotation);
    }
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    for (const auto& e : g_weakPointHitEffects) {
        if (!e.active) continue;
        if (!e.texture) continue;
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        float size = 0.25f * 1.5f * e.scale;

        // ヒットエフェクトはワールド座標保持だが、`RenderImage` はスクリーン座標描画である。
        // そのため描画前にスクリーン座標へ変換しないとカメラ分だけずれて見える。
        // ここでは culling も無効化する。culling はワールド座標カメラ判定を使うため。
        auto p = worldToScreen(e.x - size * 0.5f, e.y - size * 0.5f);
        RenderImage(p.first, p.second,
            size, size, e.texture,
            e.frame, e.rows, e.columns,
            false);
    }

    // デバッグ視認性のため背景を一時的に無効化していた
    // （元は parallax 背景描画）

    // 新しいマップシステムでタイルを描画する
    if (g_mapManager.IsMapLoaded()) {
        Map* currentMap = g_mapManager.GetCurrentMap();

        // 背景レイヤータイルを描画する
        auto& bgTiles = currentMap->GetTiles(MapLayer::BACKGROUND);
        for (const auto& tile : bgTiles) {
            if (tile.tileInfo.code == "00") continue;
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
        }

        // 中景レイヤータイルを描画する（プレイヤー活動レイヤー）
        auto& mgTiles = currentMap->GetTiles(MapLayer::MIDGROUND);
        for (const auto& tile : mgTiles) {
            if (tile.tileInfo.code == "00") continue;
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);


            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            //RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
           
            // ゴール扉用
			if (tile.tileInfo.code == "DF" || tile.tileInfo.code == "DI" || tile.tileInfo.code == "D4" || tile.tileInfo.code == "D5" || 
                tile.tileInfo.code == "D6" || tile.tileInfo.code == "D7" || tile.tileInfo.code == "DB" || tile.tileInfo.code == "21" || tile.tileInfo.code == "22" || 
                tile.tileInfo.code == "23" || tile.tileInfo.code == "24" || tile.tileInfo.code == "25" || tile.tileInfo.code == "26" || tile.tileInfo.code == "27" ||
                tile.tileInfo.code == "31" || tile.tileInfo.code == "32" || tile.tileInfo.code == "33" || tile.tileInfo.code == "34" || tile.tileInfo.code == "35" ||
                tile.tileInfo.code == "36" || tile.tileInfo.code == "37") {
                // 実際のゲーム内ブロックサイズに合わせ、当たり判定と見た目を調整する
                float renderScale = 2.0f;  // 当たり判定サイズに合わせて調整する
                float renderWidth = tile.width; // 変更なし
                float renderHeight = tile.height * renderScale;

                // スプライトをタイル位置の中央へ合わせる
                float offsetX = (tile.width - renderWidth) * 0.5f;
                float offsetY = (tile.height - renderHeight) * 0.1f;

                RenderImage(screenPos.first + offsetX, screenPos.second + offsetY,
                    renderWidth, renderHeight, texture, 0, 1, 1);
            }

            //// 一方向足場用。キャラクター当たり判定に合わせて少し小さくしている。後で変える可能性あり
            //else if (tile.tileInfo.code == "OP") {
            //    // 実際のゲーム内ブロックサイズに合わせて当たり判定へ寄せる
            //    float renderScale = 0.67f;  // 当たり判定サイズに合わせて調整する
            //    float renderWidth = tile.width; // 変更なし
            //    float renderHeight = tile.height * renderScale;

            //    // スプライトをタイル位置の中央へ合わせる
            //    float offsetX = (tile.width - renderWidth) * 0.5f;
            //    float offsetY = (tile.height - renderHeight) * 0.5f;

            //    RenderImage(screenPos.first + offsetX, screenPos.second + offsetY,
            //        renderWidth, renderHeight, texture, 0, 1, 1);
            //}
            
            // 標識アニメ用
            else if (tile.tileInfo.code == "B1" || tile.tileInfo.code == "B2" || tile.tileInfo.code == "B3" || tile.tileInfo.code == "B4" || tile.tileInfo.code == "B5" ||
                     tile.tileInfo.code == "B6" || tile.tileInfo.code == "B7" || tile.tileInfo.code == "B8" || tile.tileInfo.code == "B9") {
               
                // 標識に応じた正しいテクスチャを取得する
                ID3D11ShaderResourceView* signTexture = signAnim.GetCurrentClipTexture();
                int numFrames = 4; // 4 フレームならこれが既定

                if (tile.tileInfo.code == "B1") signTexture = g_signWASDTexture;
                else if (tile.tileInfo.code == "B2") signTexture = g_signSTexture;
                else if (tile.tileInfo.code == "B3") {
                    signTexture = g_signRightTexture;
                    numFrames = 5; // この標識だけ 4 ではなく 5 フレーム
                }
                else if (tile.tileInfo.code == "B4") signTexture = g_signReleaseTexture;
                else if (tile.tileInfo.code == "B5") signTexture = g_signRedTexture;
                else if (tile.tileInfo.code == "B6") signTexture = g_signPinkTexture;
                else if (tile.tileInfo.code == "B7") signTexture = g_signLongClickTexture;
                else if (tile.tileInfo.code == "B8") signTexture = g_signClickTexture;
                else if (tile.tileInfo.code == "B9") signTexture = g_signESCTexture;

                /*if (signTexture) {
                    RenderImage(screenPos.first, screenPos.second, tile.width, tile.height * 2.0f,
                        signTexture,
                        signAnim.GetCurrentFrame(),
                        signAnim.GetSplitX(),
                        signAnim.GetSplitY());
                }*/
                if (signTexture) {
                    // フレームを計算する
                    int currentFrame = signAnim.GetCurrentFrame() % numFrames;

                    RenderImage(screenPos.first, screenPos.second, tile.width, tile.height * 2.0f, // 標識として正しいサイズにするため高さを 2 倍にする
                        signTexture,
                        currentFrame,
                        signAnim.GetSplitX(),
                        numFrames);
                }
            }
            else {
                // それ以外のタイルは通常描画する
                RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
            }
        }

        // 前景レイヤータイルを描画する
        auto& fgTiles = currentMap->GetTiles(MapLayer::FOREGROUND);
        for (const auto& tile : fgTiles) {
            if (tile.tileInfo.code == "00") continue;
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
        }
    }

    // チャージエフェクトを描画する
    if (g_player.isCharging && !g_player.isDead) {
        float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;
        float effectSize = PLAYER_WIDTH * (1.0f + chargeRatio * 1.0f);
        float alpha = 0.3f + chargeRatio * 0.7f;

        std::pair<float, float> effectPos = worldToScreen(
            g_player.posX - (effectSize - PLAYER_WIDTH) * 0.5f,
            g_player.posY - (effectSize - PLAYER_HEIGHT) * 0.5f
        );

        float r = 1.0f;
        float g = 1.0f - chargeRatio * 0.5f;
        float b = 0.0f;
        SetColor(r, g, b, alpha);
        RenderImage(effectPos.first, effectPos.second, effectSize, effectSize, g_chargeEffectTexture, 0, 1, 1);
    }

    // ダッシュエフェクトを描画する
    if (g_player.isDashing && !g_player.isDead) {
        float dashProgress = 1.0f - (g_player.dashTimer / DASH_DURATION);
        float effectSize = PLAYER_WIDTH * (1.2f + dashProgress * 0.3f);
        float alpha = 0.7f + dashProgress * 0.3f;

        std::pair<float, float> dashPos = worldToScreen(
            g_player.posX - (effectSize - PLAYER_WIDTH) * 0.5f,
            g_player.posY - (effectSize - PLAYER_HEIGHT) * 0.5f
        );

        SetColor(1.0f, 0.3f, 0.3f, alpha);
        RenderImage(dashPos.first, dashPos.second, effectSize, effectSize, g_dashEffectTexture, 0, 1, 1);
    }

    // ゲージ撃破用の赤パーティクル（バースト）を描画する
    for (const auto& p : g_gaugeKillParticlesRed) {
        if (!p.active || !p.texture) continue;

        auto pos = worldToScreen(p.x, p.y);
        const float baseW = 0.14f;
        const float baseH = 0.14f;
        const float w = baseW * p.scale;
        const float h = baseH * p.scale;

        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(pos.first - w * 0.5f, pos.second - h * 0.5f, w, h,
            p.texture, p.frame, GaugeKillParticleInstance::rows, GaugeKillParticleInstance::columns,
            false, p.rotation, false);
    }

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    RenderEnemies(g_camera);
    g_mouseIndicator.Render(g_camera.GetX(), g_camera.GetY());

    g_projectileManager.Render(g_camera);
    // slash-count アイコン（followers）はここで描画し、タイル後ろに隠れないようにする
    // ゲージ由来無敵中は非表示にする。
    if (!(g_player.isInvincible && g_player.isGaugeInvincible) && g_slashCountTexture) {
        const int count = std::clamp(g_player.dashPoints, 0, g_player.MAX_DASH_POINTS);
        if (count > 0) {
            struct Follower {
                float x = 0.0f;
                float y = 0.0f;
                bool init = false;
            };

            static Follower s_follow[3];

            // ワールド単位でのスプライトサイズ
            const float iconW = 0.065f;
            const float iconH = 0.065f;
            const float spacing = iconW * 0.60f;

            // 目標アンカー: プレイヤーの 1 グリッド後方（向きを考慮）
            const float behind = GRID_WIDTH;
            const float baseTargetX = g_player.posX + (g_player.facingRight ? -behind : behind);
            const float baseTargetY = g_player.posY + PLAYER_HEIGHT * 0.55f;

            // アイコンごとのオフセット（重なり・一直線を避ける）
            const float xDir = g_player.facingRight ? -1.0f : 1.0f; // さらに後方へ広げる
            const float xOffset[3] = { 0.0f, spacing * xDir, spacing * 2.0f * xDir };
            const float yOffset[3] = { -iconH * 0.18f, 0.0f, iconH * 0.18f };
            const float rotOffset[3] = { -0.08f, 0.0f, 0.08f };

            const float dt = std::max(0.0f, g_gameTimer.GetDeltaTime());

            if (g_slashCountSpawnPending) {
                int spawnIndex = std::clamp(g_player.dashPoints - 1, 0, g_player.MAX_DASH_POINTS - 1);
                s_follow[spawnIndex].x = g_slashCountSpawnX;
                s_follow[spawnIndex].y = g_slashCountSpawnY;
                s_follow[spawnIndex].init = true;
                g_slashCountSpawnPending = false;
            }

            int frame = g_slashCountAnim.GetCurrentFrame() % 3;

            // follower 強調表示が矢印プレビューと完全同期するよう、
            // プレビュー由来の予約消費を計算する。
            // 実際の累積予約消費と、現在 / 保存チャージ時間から導いたプレビュー値の最大を使う。
            int actualPending = std::clamp(g_player.chargePendingCost, 0, g_player.MAX_DASH_POINTS);
            int previewPending = 0;

            // プレビューに使う有効チャージ時間を決める（MouseIndicatorSystem と一致させる）
            float effectiveChargeTime = 0.0f;
            if (g_player.isCharging) {
                const bool willChainSavedCharge =
                    g_releaseDashChargeMode &&
                    g_player.hasSavedCharge &&
                    (g_player.chargeTime < g_player.CHARGE_THRESHOLD_LOW);

                effectiveChargeTime = willChainSavedCharge ? g_player.savedChargeTime : g_player.chargeTime;
            }
            else if (g_player.hasSavedCharge) {
                effectiveChargeTime = g_player.savedChargeTime;
            }

            // プレビュー予約消費: 実際にチャージ中はチャージ量に関係なく
            // 1 個分の予約消費（赤 follower 1 個）だけ表示する。
            if (g_player.isCharging) {
                previewPending = 1;
            }
            else {
                previewPending = g_player.GetChargeLevelFromTime(effectiveChargeTime);
            }

            int pendingCost = std::clamp(std::max(actualPending, previewPending), 0, g_player.MAX_DASH_POINTS);
            int highlightStart = std::max(0, count - pendingCost);

            for (int i = 0; i < count; ++i) {
                Follower& f = s_follow[i];

                const float targetX = baseTargetX + xOffset[i];
                const float targetY = baseTargetY + yOffset[i];

                if (!f.init) {
                    f.x = targetX;
                    f.y = targetY;
                    f.init = true;
                }

                const float dx = targetX - f.x;
                const float dy = targetY - f.y;
                const float dist = sqrtf(dx * dx + dy * dy);

                const float tauNear = 0.010f;
                const float tauFar = 0.002f;
                const float dist01 = std::clamp(dist / (GRID_WIDTH * 2.0f), 0.0f, 1.0f);
                const float tau = tauNear + (tauFar - tauNear) * dist01;
                const float safeTau = std::max(tau, 1e-4f);
                float a = 1.0f - expf(-dt / safeTau);
                a = std::clamp(a, 0.0f, 1.0f);

                f.x += dx * a;
                f.y += dy * a;

                if (dist < 0.0025f) {
                    f.x = targetX;
                    f.y = targetY;
                }

                auto p = worldToScreen(f.x, f.y);

                if (g_player.isCharging && g_player.isChargeCostHighlight && pendingCost > 0 && i >= highlightStart) {
                    SetColor(1.0f, 1.0f, 0.3f, 1.0f);
                }
                else {
                    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
                }

                bool followerFlip = (f.x < g_player.posX);
                RenderImage(p.first, p.second, iconW, iconH,
                    g_slashCountTexture,
                    frame,
                    1,
                    3,
                    false,
                    rotOffset[i],
                    followerFlip);
            }

            for (int i = count; i < 3; ++i) {
                s_follow[i].init = false;
            }

            SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    // プレイヤーを描画する
    if (!g_player.isDead) {
        // 生存中の通常描画
        std::pair<float, float> playerPos = worldToScreen(g_player.posX, g_player.posY);

        // キャラクターサイズ用
        float scale = 6.6f;
        float width = PLAYER_WIDTH * scale;
        float height = PLAYER_HEIGHT * scale;

        // 大きいスプライトを当たり判定の中心へ合わせる
        float offsetX = (width - PLAYER_WIDTH) * 0.5f;
        float offsetY = (height - PLAYER_HEIGHT) * 0.5f;

        // 現在のアニメーションクリップのテクスチャを取得する
        ID3D11ShaderResourceView* currentTexture = g_player.anim.GetCurrentClipTexture();
        int frameIndex = g_player.anim.GetCurrentFrame();
        int splitX = g_player.anim.GetSplitX();
        int splitY = g_player.anim.GetSplitY();

        // 向きに応じて水平反転を決める
        // facingRight が true（右向き）なら反転しない
        // facingRight が false（左向き）なら水平反転する
        bool flipHorizontal = !g_player.facingRight;


        //SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(playerPos.first - offsetX, playerPos.second - offsetY, width, height,
            currentTexture, frameIndex, splitY, splitX, true, 0.0f, flipHorizontal);

        SetColor(1.0f, 1.0f, 1.0f, 1.0f);  // 無敵描画後に色をリセットする




        // attack count 用
        
        //// プレイヤー左上へ配置する
        //float countWidth = 0.1f;   
        //float countHeight = 0.15f; 
        //float offsetCountX = -0.08f;  // 水平
        //float offsetCountY = PLAYER_HEIGHT /*+ 0.01f*/; // 垂直

        //float countXPos = g_player.posX + offsetCountX;
        //float countYPos = g_player.posY + offsetCountY;

        //auto countScreenPos = worldToScreen(countXPos, countYPos);

        //int frameIndexC = 0;
        //if (g_player.dashPoints == 3) {
        //    frameIndexC = 0;  // 三角 3 個を表示
        //}
        //else if (g_player.dashPoints == 2) {
        //    frameIndexC = 1;  // 三角 2 個を表示
        //}
        //else {
        //    frameIndexC = 2;  // 三角 1 個を表示（dashPoints 1 または 0 用）
        //}

        //SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        //RenderImage(countScreenPos.first, countScreenPos.second,
        //    countWidth, countHeight,
        //    g_attackCountTestTexture, 0, 1, 3);

        //RenderImage(countScreenPos.first + 0.2f, countScreenPos.second,
        //    countWidth, countHeight,
        //    g_attackCountTestTexture, 1, 1, 3);

        //RenderImage(countScreenPos.first + 0.4f, countScreenPos.second,
        //    countWidth, countHeight,
        //    g_attackCountTestTexture, 2, 1, 3);




    }
    else {
        // 死亡アニメーション
        std::pair<float, float> playerPos = worldToScreen(g_player.posX, g_player.posY);

        float scale = 6.6f;
        float width = PLAYER_WIDTH * scale;
        float height = PLAYER_HEIGHT * scale;

        float offsetX = (width - PLAYER_WIDTH) * 0.5f;
        float offsetY = (height - PLAYER_HEIGHT) * 0.5f;

        // 死亡アニメのテクスチャとフレームを取得する
        ID3D11ShaderResourceView* currentTexture = g_player.anim.GetCurrentClipTexture();
        int frameIndex = g_player.anim.GetCurrentFrame();
        int splitX = g_player.anim.GetSplitX();
        int splitY = g_player.anim.GetSplitY();

        bool flipHorizontal = !g_player.facingRight;

        // 死亡アニメを描画する
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(playerPos.first - offsetX, playerPos.second - offsetY, width, height,
            currentTexture, frameIndex, splitY, splitX, true, 0.0f, flipHorizontal);
    }

    DrawComboUI();
    DrawGaugeUI();
    DrawScoreUI();

    // ESC テクスチャ用
    if (g_escTexture) {
        // ESC ボタン UI
        InGameUI escUI;
        escUI.x = -0.95f;
        escUI.y = -0.95f;
        escUI.width = 0.4f;
        escUI.height = 0.5f;

        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(escUI.x, escUI.y, escUI.width, escUI.height, g_escTexture, 0, 1, 1);
    }
}

void HandleInput() {
    if (g_inputSystem.IsResetting()) {
        ResetGame();
    }

    // 死亡アニメ中（被弾で死亡）はすべてのゲーム入力を無効にする。
    // そうしないと下の移動入力で `g_player.facingRight` が変わってしまう。
    if (g_player.isDead) {
        g_player.velocityX = 0.0f;
        g_player.isMoving = false;
        if (g_player.isCharging) {
            CancelChargeDash();
        }
        return;
    }
   /* if (g_inputSystem.IsMouseRightDown()) {
        CancelChargeDash();
    }*/
    // 右クリック: チャージキャンセル専用として扱う（無敵発動は自動化済み）
    if (g_inputSystem.IsMouseRightDown()) {
        CancelChargeDash();
    }
    // ゲーム一時停止は P または Esc キーで行う
    if (g_inputSystem.IsTogglePressed(VK_P) || g_inputSystem.IsTogglePressed(VK_ESCAPE))
    {
        SCENE currentScene = sceneManager.GetCurrentSceneType();

        if (currentScene == GAMEPLAY || currentScene == CAKE) // エリアとケーキシーン
        {
            sceneManager.SaveBGMPath(Audio::GetCurrentBGMPath());
            sceneManager.SwitchScene(PAUSE);  // どのステージでも一時停止できる
            Audio::PauseBGM();
        }

        // P または Esc をもう一度押すとステージへ戻れる（マウスで continue を押してもよい）
        else if (currentScene == PAUSE)
        {
            SCENE previousScene = sceneManager.GetOriginalPausedScene();
            if (previousScene == GAMEPLAY || previousScene == CAKE) // エリアとケーキシーン
            {
                sceneManager.SwitchScene(previousScene);
                //Audio::ResumeBGM();
                std::string savedPath = sceneManager.GetSavedBGMPath();
                if (!Audio::IsBGMPlaying() && !savedPath.empty())
                {
                    Audio::PlayBGM(savedPath, true);
                }
                else
                {
                    Audio::ResumeBGM();
                }
                sceneManager.ClearSavedBGMPath();
            }
        }
    }

    // 現在のアクティブシーンが pause なら、これ以降のゲーム入力
    // （移動、ダッシュ、ジャンプ、マウスチャージ等）は処理しない。
    // ただし上の pause 切替処理は実行済みなので再開は可能。
    if (sceneManager.GetCurrentSceneType() == PAUSE) {
        return;
    }

    // マウス入力状態を取得する
    bool isMouseLeftPressed = g_inputSystem.IsMouseLeftPressed();
    bool isMouseLeftDown = g_inputSystem.IsMouseLeftDown();
    bool isMouseLeftReleased = g_inputSystem.IsMouseLeftReleased();

    // チャージダッシュ入力モード（VK_T）:
    // - true（既定）: 保存済みチャージがあっても、離した時に必ずダッシュする
    // - false: 保存済みチャージがあれば、マウス押下で即ダッシュする
    if (g_inputSystem.IsTogglePressed(VK_T)) {
        g_releaseDashChargeMode = !g_releaseDashChargeMode;
    }

    // ダッシュ後硬直の挙動切替（VK_G）:
    // - false（既定）: 後硬直は既存の重力 / 物理挙動を使う
    // - true: 後硬直中は重力を無視する。移動入力で後硬直は解除される（既存仕様）
    if (g_inputSystem.IsTogglePressed(VK_G)) {
        g_noGravityAftermathMode = !g_noGravityAftermathMode;
    }

    static bool wasMouseLeftDown = false;

    // マウスのみ操作: 押してチャージ開始
    if (isMouseLeftPressed) {
        if (!g_releaseDashChargeMode) {
            // 旧仕様: 保存済みチャージがあるなら、押した瞬間にダッシュする
            if (g_player.hasSavedCharge && !g_player.isCharging) {
                StartMouseChargeDash();
                g_player.chargeTime = 0.0f;
                ExecuteMouseChargeDash();
            }
            else {
                StartMouseChargeDash();
            }
        }
        else {
            // 新モード: 常にチャージ開始し、ダッシュは離した時に発生する
            StartMouseChargeDash();
        }
    }

    // マウスのみ操作: 離してダッシュ実行
    if (isMouseLeftReleased && wasMouseLeftDown && g_player.isCharging) {
        if (g_releaseDashChargeMode) {
            // 長押し < 0.2 秒: 以前の保存済みチャージを連携使用する（あれば）
            // 長押し >= 0.2 秒: 新しいチャージを使い、保存分を上書きする
            if (g_player.chargeTime < g_player.CHARGE_THRESHOLD_LOW) {
                if (g_player.hasSavedCharge) {
                    g_player.LoadSavedCharge();
                }
            }
            else {
                // Force_execute は saved へ戻らないようにして現在チャージを使わせる
                g_player.ClearSavedCharge();
            }
        }

        ExecuteMouseChargeDash();
    }

    // チャージをキャンセルする
    if (!isMouseLeftDown && g_player.isCharging) {
        CancelChargeDash();
    }

    wasMouseLeftDown = isMouseLeftDown;

    // 移動操作
    bool moving = false;
    if (g_inputSystem.IsMovingLeft()) {
        if (!g_player.isDashing) {
            g_player.velocityX = -MOVE_SPEED * g_player.GetMoveSpeedMultiplier();
        }
        g_player.isMoving = true;
        g_player.facingRight = false;
        moving = true;
    }
    if (g_inputSystem.IsMovingRight()) {
        if (!g_player.isDashing) {
            g_player.velocityX = MOVE_SPEED * g_player.GetMoveSpeedMultiplier();
        }
        g_player.isMoving = true;
        g_player.facingRight = true;
        moving = true;
    }

    // 要望対応: 移動入力でダッシュ終了後スローモーションを中断する。
    // 注: 移動入力はここで直接処理しているため、入力段階で状態を切る。
    if (moving && g_player.isInDashEndSlowMo) {
        g_player.isInDashEndSlowMo = false;
        g_player.dashEndSlowMoTimer = 0.0f;
    }

    if (!moving && !g_player.isDashing) {
        g_player.velocityX = 0.0f;
        g_player.isMoving = false;
    }

    // ジャンプ操作
    // Space と W の両方でジャンプ可能にする（壁滑り中も含む）。
    static bool wasSpacePressed = false;
    static bool wasWPressed = false;

    bool spaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    bool wDown = (GetAsyncKeyState('W') & 0x8000) != 0;

    // エッジ入力（このフレームで押され、前フレームでは押されていない）でジャンプする。
    // 両方同時押しでも Jump() は 1 回だけ呼ぶ。
    if ((spaceDown && !wasSpacePressed) || (wDown && !wasWPressed)) {
        Jump();
    }

    wasSpacePressed = spaceDown;
    wasWPressed = wDown;
}

// MouseIndicatorSystem の実装
void MouseIndicatorSystem::Initialize() {
    m_mouseIndicatorTexture = g_chargeEffectTexture;
    m_cursorTexture = g_cursorTexture;
    m_showMouseIndicator = true;
    m_arrowAngle = 0.0f;
}

void MouseIndicatorSystem::Update(float deltaTime) {
    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY);

    m_mouseWorldX = mouseX;
    m_mouseWorldY = mouseY;

    float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;

    float deltaX = m_mouseWorldX - playerCenterX;
    float deltaY = m_mouseWorldY - playerCenterY;

    m_arrowAngle = atan2(deltaY, deltaX);

    static int debugCounter = 0;
    if (debugCounter++ % 60 == 0) {
        printf("Mouse World: (%.2f, %.2f), Player: (%.2f, %.2f)\n",
            m_mouseWorldX, m_mouseWorldY, playerCenterX, playerCenterY);
    }
}

void MouseIndicatorSystem::Render(float cameraX, float cameraY) {
    //if (!m_showMouseIndicator) return;
    //// 主人公が死亡しているならマウスインジケータを表示しない
    //if (g_player.isDead) return;
    auto worldToScreen = [cameraX, cameraY](float worldX, float worldY) -> std::pair<float, float> {
        return { worldX - cameraX, worldY - cameraY };
        };

    // マウス位置インジケータを描画する
    float cursorWidth = 0.04f;
    float cursorHeight = 0.12f;
    auto mousePos = worldToScreen(m_mouseWorldX - cursorWidth / 2, m_mouseWorldY - cursorHeight / 2);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    RenderImage(mousePos.first, mousePos.second, cursorWidth, cursorHeight,
        m_cursorTexture, 0, 1, 1, false, 0);
    // マウスカーソル自体は `g_gameCursor` がグローバルに描画する

    // （固定数値の dash-points UI は削除済み。followers は引き続きプレイヤー上に描画される）

    // （T/G モード用のデバッグ数値 UI は削除済み）

    
    float uiX = -1.0f;
    float uiY = 0.25f;
    float uiWidth = 0.5f;
    float uiHeight = 1.0f;
    RenderImage(uiX, uiY, uiWidth, uiHeight, g_uiNumberTexture, 0, 1, 1);

    // タイマー表示用
    float timerX = -0.768f;  // X 座標
    float timerY = 0.78f;   // Y 座標
    float timerDigitWidth = 0.03f;  // 幅
    float timerDigitHeight = 0.07f; // 高さ

    // 分表示用
    int minuteTens = g_gameMinutes / 10;
    int minuteOnes = g_gameMinutes % 10;
    RenderNumber(minuteTens, timerX, timerY, timerDigitWidth, timerDigitHeight, pTextureNum);
    RenderNumber(minuteOnes, timerX + timerDigitWidth * 0.8f, timerY, timerDigitWidth, timerDigitHeight, pTextureNum);

    // 秒表示用
    int secondTens = g_gameSeconds / 10;
    int secondOnes = g_gameSeconds % 10;
    float secondsStartX = timerX + timerDigitWidth * 2.2f;
    RenderNumber(secondTens, secondsStartX, timerY, timerDigitWidth, timerDigitHeight, pTextureNum);
    RenderNumber(secondOnes, secondsStartX + timerDigitWidth * 0.8f, timerY, timerDigitWidth, timerDigitHeight, pTextureNum);

    //SetColor(1.0f, 1.0f, 1.0f, 1.0f); 

    if (!m_showMouseIndicator) return;
    // 主人公が死亡しているならマウスインジケータを表示しない
    if (g_player.isDead) return;

    // 方向矢印を描画する

    if (g_player.isCharging || g_player.hasSavedCharge)
    {
        if (!m_arrowShow) {

        }
        else {

            float centerOffsetX = 0.003f;
            float centerOffsetY = 0.0f;
            float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f + centerOffsetX;
            float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f + centerOffsetY;
            float arrowWidth = 0.15f;

            // 矢印プレビュー用の有効チャージ時間を取得する。
            // 連携（短押し）で saved charge がある場合、実際のダッシュはそれを使う。
            // そのため矢印も saved charge を反映しないと 2 回目のダッシュが短く見えてしまう。
            float chargeTime = 0.0f;
            if (g_player.isCharging) {
                const bool willChainSavedCharge =
                    g_releaseDashChargeMode &&
                    g_player.hasSavedCharge &&
                    (g_player.chargeTime < g_player.CHARGE_THRESHOLD_LOW);

                chargeTime = willChainSavedCharge ? g_player.savedChargeTime : g_player.chargeTime;
            }
            else if (g_player.hasSavedCharge) {
                chargeTime = g_player.savedChargeTime;
            }

            // チャージ段階を求め、ExecuteMouseChargeDash と同じ倍率を使う
            int chargeLevelPreview = g_player.GetChargeLevelFromTime(chargeTime);

            float speedMultiplier = 1.0f;
            float durationMultiplier = 1.0f;
            switch (chargeLevelPreview) {
            case 1:
                speedMultiplier = 1.3f;
                durationMultiplier = 1.0f;
                break;
            case 2:
                speedMultiplier = 1.6f;
                durationMultiplier = 1.0f;
                break;
            case 3:
                speedMultiplier = 2.0f;
                durationMultiplier = 1.0f;
                break;
            default:
                speedMultiplier = 1.0f;
                durationMultiplier = 1.0f;
                break;
            }

            // ExecuteMouseChargeDash と DashToMouse は短距離（無チャージ）ダッシュへ
            // 追加で 1.3 倍を掛けるため、プレビューもその挙動へ合わせる。
            // この追加倍率はプレビュー段階が 0 のときだけ適用する。
            const float SHORT_DASH_EXTRA = (chargeLevelPreview == 0) ? 1.3f : 1.0f;

            // プレイヤーが実際に移動するワールド距離を計算する
            // 距離 = 速度 × 時間 × 60.0（UpdatePlayerPhysics と一致）
            float dashSpeed = DASH_SPEED * speedMultiplier * SHORT_DASH_EXTRA;
            float dashDuration = DASH_DURATION * durationMultiplier;
            float arrowLength = dashSpeed * dashDuration * 60.0f;

            float tailX = playerCenterX;
            float tailY = playerCenterY;

            // 矢印の中心（尾と先端の中点）を計算する
            float arrowCenterX = tailX + cosf(m_arrowAngle) * (arrowLength * 0.5f);
            float arrowCenterY = tailY + sinf(m_arrowAngle) * (arrowLength * 0.5f);

            // 画面描画用の位置計算
            auto arrowScreenPos = worldToScreen(arrowCenterX - arrowLength * 0.5f, arrowCenterY - arrowWidth * 0.5f);

            // 色表示用にチャージ段階を取得する
            int chargeLevel = g_player.GetChargeLevelFromTime(chargeTime);

            // チャージ段階に応じて色を変える
            if (chargeLevel >= 3) {
                //SetColor(1.0f, 0.0f, 0.0f, 1.0f); // 赤
            }
            else if (chargeLevel >= 2) {
                //SetColor(0.0f, 0.0f, 1.0f, 1.0f); // 濃い青
            }
            else if (chargeLevel >= 1) {
                //SetColor(0.0f, 1.0f, 1.0f, 1.0f); // 青
            }

            SetColor(1.0f, 1.0f, 1.0f, 1.0f);
            // 矢印テクスチャは 1x3 のスプライトシートで、0 / 1 が胴体、2 が先端。
            // 画像全体を引き伸ばす代わりに、0 / 1 を繰り返して軸を作り、2 を先端として描画する。
            float dirX = cosf(m_arrowAngle);
            float dirY = sinf(m_arrowAngle);

            // ワールド単位で先端幅を決める。先端高さは arrowWidth と同じにする。
            float headWidth = arrowWidth; // 高さに対して横方向は極力等倍に近づける

            // 矢印が短すぎる場合は元の引き伸ばし描画へフォールバックする
            if (arrowLength <= headWidth * 1.1f) {
                RenderImage(arrowScreenPos.first, arrowScreenPos.second, arrowLength, arrowWidth,
                    g_arrowTexture, 0, 1, 1, false, m_arrowAngle);
            }
            else {
                float bodyLength = arrowLength - headWidth;

                // 胴体セグメント数を決める。headWidth を基準単位として見た目をそろえる。
                int segmentCount = (int)ceilf(bodyLength / headWidth);
                if (segmentCount < 1) segmentCount = 1;

                float segmentWidth = bodyLength / (float)segmentCount;

                // 各胴体セグメントを描画し、フレーム 0 / 1 を交互に使う
                for (int i = 0; i < segmentCount; ++i) {
                    float segCenterDist = (segmentWidth * (i + 0.5f));
                    float segCenterX = tailX + dirX * segCenterDist;
                    float segCenterY = tailY + dirY * segCenterDist;

                    auto segScreenPos = worldToScreen(segCenterX - segmentWidth * 0.5f, segCenterY - arrowWidth * 0.5f);

                    int frame = (i % 2 == 0) ? 0 : 1; // フレーム 0 と 1 を交互に使う
                    RenderImage(segScreenPos.first, segScreenPos.second, segmentWidth, arrowWidth,
                        g_arrowTexture, frame, 1, 3, false, m_arrowAngle);
                }

                // 先端に head を描画する
                float headCenterDist = bodyLength + headWidth * 0.5f;
                float headCenterX = tailX + dirX * headCenterDist;
                float headCenterY = tailY + dirY * headCenterDist;
                auto headScreenPos = worldToScreen(headCenterX - headWidth * 0.5f, headCenterY - arrowWidth * 0.5f);
                RenderImage(headScreenPos.first, headScreenPos.second, headWidth, arrowWidth,
                    g_arrowTexture, 2, 1, 3, false, m_arrowAngle);
            }

            SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}

void MouseIndicatorSystem::Cleanup() {
    m_mouseIndicatorTexture = nullptr;
}

void MouseIndicatorSystem::ShowMouseIndicator(bool i) {

    // 旧 API: カーソル表示は現在グローバルな in-game cursor が管理している。
    // 既存シーンコードが表示制御を意識しなくて済むよう、この API は残しておく。
    SetInGameCursorEnabled(i);
}


// 全統計をリセットする
void GameStatistics::Reset() {
    enemiesKilled = 0;
    weakPointKills = 0;
    totalDeaths = 0;
    totalTime = 0.0f;
    totalScore = 0;
    penalizableDeaths = 0;

    maxCombo = 0;
    currentAreaEnemyPoints = 0;
    totalEnemyPoints = 0;

    ResetCurrentStats();

    lifetimeEnemyPoints = 0;
    lifetimeKills = 0;
    lifetimeWeakKills = 0;
}

// キルカウンタを増やす
void GameStatistics::IncrementKills() {
    enemiesKilled++;
    currentKills++;
    lifetimeKills++;
    AddEnemyPoints(10);
}

// 弱点キルカウンタを増やす
void GameStatistics::IncrementWeakPointKills() {
    weakPointKills++;
    currentWeakKills++;
    lifetimeWeakKills++;
    AddEnemyPoints(30);
}

// 死亡カウンタを増やす
void GameStatistics::IncrementDeaths() {
    totalDeaths++;

    // 後で削除
    char debugMsg[256];
    sprintf_s(debugMsg, "\n====== PLAYER DIED ======\n");
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "Total Deaths: %d\n", totalDeaths);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "Lost Area Points: %d\n", currentAreaEnemyPoints);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "Lost Max Combo: %d\n", maxCombo);
    OutputDebugStringA(debugMsg);

    ResetAreaProgress();

    // 後で削除
    sprintf_s(debugMsg, "Remaining Total Points: %d\n", totalEnemyPoints);
    OutputDebugStringA(debugMsg);
    OutputDebugStringA("==========================\n\n");

}

// 最大コンボ更新用
void GameStatistics::UpdateMaxCombo(int combo) {
    if (combo > maxCombo) {
        maxCombo = combo;
    }
    if (combo > currentMaxCombo) {
        currentMaxCombo = combo; // 現在の最大コンボ
    }
}
// プレイヤー死亡時にエリア進行状況をリセットする
void GameStatistics::ResetAreaProgress() {
    //totalEnemyPoints -= currentAreaEnemyPoints; // 現在エリア分のポイントを総計から引く
    currentAreaEnemyPoints = 0; // 現在エリア進行をリセットする
    //maxCombo = 0;  // 死亡時に最大コンボをリセットする
    currentMaxCombo = 0;
}


void GameStatistics::AddEnemyPoints(int points) {
    currentAreaEnemyPoints += points;
    totalEnemyPoints += points;
    currentScore += points;
    lifetimeEnemyPoints += points;
}

// 総時間を更新する
void GameStatistics::UpdateTime(float time) {
    totalTime = time;
}

// キル数・時間・死亡数に基づいて最終スコアを計算する
void GameStatistics::CalculateFinalScore() {
    int comboMultiplier = std::max(1, maxCombo);  // 最低コンボ倍率は 1

    // 時間を 4 桁数値へ変換する（総秒数ベース）
    //int timeInSeconds = static_cast<int>(totalTime);
    int minutes = static_cast<int>(totalTime) / 60;
    int seconds = static_cast<int>(totalTime) % 60;
    int timeInMMSS = (minutes * 100) + seconds;  // MMSS 形式

    // ペナルティ計算用に死亡数上限を 50 にする
    int cappedDeaths = std::min(50, totalDeaths);
    int deathPenalty = cappedDeaths * 50;

    // 基本スコアを計算する
    int baseScore = lifetimeEnemyPoints * comboMultiplier;

    // ペナルティを計算する
    int penalty = timeInMMSS + deathPenalty;

    int penaltyMultiplied = static_cast<int>(penalty * 1.5f); // 整数値になるようにする
    totalScore = baseScore - penaltyMultiplied;

    // スコアが負にならないようにする
    totalScore = std::max(0, totalScore);

    // 後で削除
    char debugMsg[512];
    sprintf_s(debugMsg, "\n========== FINAL SCORE CALCULATION ==========\n");
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "Formula: (points × combo) - ((time + deaths×50) × 1.5)\n");
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "\nValues:\n");
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Total Enemy Points: %d\n", totalEnemyPoints);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Max Combo (lifetime): %d\n", maxCombo);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Time: %d:%02d (MMSS: %04d)\n", minutes, seconds, timeInMMSS);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Deaths: %d (capped: %d)\n", totalDeaths, cappedDeaths);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "\nCalculation:\n");
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Base Score: %d × %d = %d\n",
        totalEnemyPoints, comboMultiplier, baseScore);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Death Penalty: %d × 50 = %d\n",
        cappedDeaths, deathPenalty);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Total Penalty Before ×1.5: %d + %d = %d\n",
        timeInMMSS, deathPenalty, penalty);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Penalty After ×1.5: %d\n", penaltyMultiplied);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Final: %d - %d = %d\n",
        baseScore, penaltyMultiplied, totalScore);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "\n>>> FINAL SCORE: %d <<<\n", totalScore);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "\nStatistics Summary:\n");
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Enemies Killed: %d\n", enemiesKilled);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Weak Point Kills: %d\n", weakPointKills);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Total Deaths: %d\n", totalDeaths);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, " Time: %d:%02d (%.1f seconds)\n", minutes, seconds, totalTime);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "=============================================\n\n");
    OutputDebugStringA(debugMsg);
}

void GameStatistics::AddScore(int points) {
    totalScore += points;
    if (totalScore < 0) {
        totalScore = 0; // 負のスコアにならないようにする
    }
}

void GameStatistics::ResetCurrentStats() {
    currentScore = 0;
    currentKills = 0;
    currentWeakKills = 0;
    currentAreaEnemyPoints = 0;
    //totalEnemyPoints = 0;
}
