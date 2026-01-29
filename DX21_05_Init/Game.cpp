#include "Game.h"
// Add to global variable definition section in Game.cpp
float g_slowMoTimer = 0.0f;
float g_slowMoFactor = 1.0f;
bool g_isSlowMotion = false;

// added december 4th
// for the timer of the game
float g_gameElapsedTime = 0.0f;
int g_gameMinutes = 0;
int g_gameSeconds = 0;


// Sound effect instance ID storage
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
// Clear gauge state and particles (callable from other modules)
void ClearGaugeOnDeath()
{
    // Reset player gauge values
    g_player.gaugePoints = 0;
    g_player.isInvincible = false;
    g_player.isGaugeInvincible = false;
    g_player.g_gaugeEffectActive = false;
    g_player.g_gaugeEffectTimer = 0.0f;

    // Clear particles and spawn timer
    g_gaugeTrailParticles.clear();
    g_gaugeKillParticlesRed.clear();
    g_gaugeTrailSpawnTimer = 0.0f;
}

void SpawnGaugeKillParticlesRed(float worldX, float worldY) {
    if (!g_gaugeKillParticleRedTexture) return;

    const int count = 4 + (rand() % 4); // 4..7
    for (int i = 0; i < count; ++i) {
        GaugeKillParticleInstance p;
        p.x = worldX;
        p.y = worldY;
        p.texture = g_gaugeKillParticleRedTexture;
        p.active = true;

        // Random radial burst
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
        // Slow down animation to ~0.6x of current speed (increase frame time)
        p.frameTime = 0.05f / 0.6f;

        g_gaugeKillParticlesRed.push_back(p);
    }
}

// Spawn a larger, more dramatic burst when the gauge is filled (player becomes invincible).
// This creates more particles with higher speed and wider spread than the regular red kill burst.
void SpawnGaugeFullBurst(float worldX, float worldY) {
    // Use the trail particle sheet (particle_sheet.png) for a more varied look
    if (!g_gaugeTrailParticleTexture) return;

    const int count = 20 + (rand() % 11); // 20..30 particles for a very large burst
    for (int i = 0; i < count; ++i) {
        GaugeKillParticleInstance p;
        p.x = worldX;
        p.y = worldY;
        p.texture = g_gaugeTrailParticleTexture; // particle_sheet.png
        p.active = true;

        // Stronger radial burst with wider variation
        const float angle = Rand01() * 6.2831853f;
        const float speed = (0.15f + Rand01() * 0.35f) * 10.0f; // faster
        p.vx = cosf(angle) * speed;
        p.vy = sinf(angle) * speed;

        // Larger, more varied scale
        p.scale = 1.2f * (0.9f + Rand01() * 0.6f);
        p.rotation = (Rand01() * 2.0f - 1.0f) * 3.14159f;
        p.angularVelocity = (Rand01() * 2.0f - 1.0f) * 12.0f; // stronger spin
        p.frame = 0;
        p.timer = 0.0f;
        p.frameTimer = 0.0f;
        // Faster animation for a snappier look
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

    // Natural dispersion:
    // - random drift direction
    // - slight bias backwards relative to facing
    // - random scale + rotation
    const float r01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    const float r02 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    const float r03 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    const float r04 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    const float angle = (r01 * 2.0f - 1.0f) * 1.1f; // ~[-1.1, 1.1] rad
    const float speed = 0.02f + r02 * 0.03f;        // 0.02..0.05

    float backBias = g_player.facingRight ? -0.015f : 0.015f;
    p.vx = cosf(angle) * speed + backBias;
    p.vy = sinf(angle) * speed + 0.01f;

    // Size: half of previous (base 1.0 -> 0.5), with small random variation.
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
    // Prefer new slash flash textures; fallback to old single texture if needed.
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
    // Weak-point slash flash should be more visible than a normal hit.
    e.scale = (chosen == g_hitEffectTexture) ? 1.0f : 2.0f;
    e.timer = 0.0f;
    e.frameTime = 0.08f;
    e.frame = 0;
    e.active = true;
    e.texture = chosen;

    // Some textures are sprite-sheets (slash flash), others are single images (legacy hit).
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

// Spawn a larger visual effect for weak-point kills
void SpawnWeakPointKillEffect(float worldX, float worldY) {
    // Prefer new slash flash textures; fallback to old single texture if needed.
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
    // Make the kill effect larger than a normal weak-point hit
    e.scale = (chosen == g_hitEffectTexture) ? 1.5f : 3.0f;
    e.timer = 0.0f;
    // Slightly slower frame time for a more dramatic kill flash
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

// Slash-count follower spawn request (set on enemy kill)
float g_slashCountSpawnX = 0.0f;
float g_slashCountSpawnY = 0.0f;
bool g_slashCountSpawnPending = false;

void SetGameWindowHandle(HWND hwnd) {
    g_gameHwnd = hwnd;
}

GameCursor g_gameCursor;

void GameCursor::Initialize(ID3D11ShaderResourceView* texture) {
    m_texture = texture;
    // Match the historical cursor placement used by MouseIndicatorSystem.
    // The cursor texture is drawn with its top-left at the mouse world position.
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

    // Keep ShowCursor counter stable: call until the desired visibility is reached.
    const bool showOsCursor = !enabled;
    if (showOsCursor) {
        while (ShowCursor(TRUE) < 0) {}
    }
    else {
        while (ShowCursor(FALSE) >= 0) {}
    }
}

// ...existing code...

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

    // Use the same orientation source as the player render logic.
    // During wall slide, facingRight can be momentarily out-of-sync with the visual orientation.
    bool facingRightForAfterImage = g_player.facingRight;
    if (g_player.isWallSliding && g_player.wallSlideDirection != 0) {
        // In Player.cpp wall-slide detection:
        //   left wall  => wallSlideDirection = -1, facingRight = true
        //   right wall => wallSlideDirection =  1, facingRight = false
        facingRightForAfterImage = (g_player.wallSlideDirection == -1);
    }

    a.flipHorizontal = !facingRightForAfterImage;
    g_playerAfterImages.push_back(a);
}

// Game timer implementation
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

// Trigger slow motion effect
void TriggerSlowMotion(float duration = 1.0f, float factor = 0.3f) {
    g_isSlowMotion = true;
    g_slowMoTimer = duration;
    g_slowMoFactor = factor;
}

void ResetGame() {
    g_projectileManager.ClearAll();  // New: clear all projectiles
    CleanupEnemies();
    g_weakPointHitEffects.clear();
    g_playerAfterImages.clear();
    // Preserve gauge-related particles and timers so the player's gauge
    // progress and visual effects are not lost when entering a new level.
    // g_gaugeTrailParticles.clear();
    // g_gaugeKillParticlesRed.clear();
    // g_gaugeTrailSpawnTimer = 0.0f;
    if (g_mapManager.IsMapLoaded()) {
        g_mapManager.ReloadCurrentMap();
    }

    g_player.comboCount = 0;
    g_player.comboTimer = 0.0f;

    // Reset charge (charging / saved charge) state
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
    g_player.hitStopTriggered = 0;
    g_player.hitStopTimer = 0.0f;
    g_player.savedChargeTime = 0.0f;
    g_player.hasSavedCharge = false;
    g_player.chargeDecayTimer = 0.0f;

    // Preserve gauge points and invincibility state across level transitions.
    // Do not reset g_player.gaugePoints, g_player.isInvincible, g_player.isGaugeInvincible,
    // or g_player.invincibleTimer here.

    g_gameState = STATE_PLAYING;
}

// added december 11th
void CleanUpGameWorld()
{
    g_projectileManager.ClearAll();
    CleanupEnemies();
    g_mouseIndicator.Cleanup();
    g_weakPointHitEffects.clear();
    g_playerAfterImages.clear();
    // NOTE: preserve gauge-related particles/state when switching maps so
    // the player's gauge (points and active effects) is not unexpectedly
    // reset when entering a new level. Do not clear the gauge particle
    // lists or reset the spawn timer here.

    // 释放所有纹理 - 只保留右边纹理
    ReleaseTexture(g_playerTexture);
    // 只保留右边的纹理
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

    // FOR THE PLAYER WHEN INVINCIBLE
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

    ReleaseTexture(g_groundTexture);
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

    // 解放击中特效纹理
    ReleaseTexture(g_hitEffectTexture);
    for (auto& t : g_slashFlashTextures) {
        ReleaseTexture(t);
    }
    ReleaseTexture(g_numberTexture);
    ReleaseTexture(g_uiNumberTexture);
    ReleaseTexture(g_arrowTexture);
    ReleaseTexture(g_cursorTexture);

    ReleaseTexture(g_escTexture);

    // for the combo texture
    ReleaseTexture(g_comboNumberTexture);
    ReleaseTexture(g_comboXTexture);
    ReleaseTexture(g_comboRemainingTimeTexture);

    // for the gauge bar when there is one
    ReleaseTexture(g_gaugeBarTexture);
    ReleaseTexture(g_gaugeBarFilledTexture);
    ReleaseTexture(g_gaugeFullEffectTexture);
    ReleaseTexture(g_gaugeTrailParticleTexture);
    ReleaseTexture(g_gaugeKillParticleRedTexture);

    ReleaseTexture(g_attackCountTestTexture);

    ReleaseTexture(g_bossHealthBarTexture);
    ReleaseTexture(g_bossInnerHPTexture);
}

// Improved collision detection function
bool CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
        y1 < y2 + h2 && y1 + h1 > y2);
}

// for the combo UI of the player when hitting enemies
void DrawComboUI(void)
{
    if (g_player.comboCount < 1)
    {
        return;
    }

    // Combo X symbol UI
    InGameUI comboXUI;
    comboXUI.x = 0.3f;
    comboXUI.y = 0.45f;
    comboXUI.width = 0.45f;
    comboXUI.height = 0.5f;

    // Combo digit UI
    InGameUI comboDigitUI;
    comboDigitUI.width = 0.15f;
    comboDigitUI.height = 0.35f;
    comboDigitUI.y = 0.55f;

    // Spacing values
    float spaceBetweenDigits = 0.12f;
    float spaceBetweenXandDigit = 0.08f;

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Draw the "X" symbol
    RenderImage(comboXUI.x, comboXUI.y, comboXUI.width, comboXUI.height, g_comboXTexture, 0, 1, 1);

    char buffer[32];
    sprintf_s(buffer, "%d", g_player.comboCount); // it converts the number into digits

    float digitXaxis = comboXUI.x + (comboXUI.width * 0.5f) + spaceBetweenXandDigit; // for the first number x axis position 

    // Draw combo number
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        int digit = buffer[i] - '0';  // 1 for frame 1, 2 for frame 2, 3 for frame 3, etc etc
        RenderImage(digitXaxis, comboDigitUI.y, comboDigitUI.width, comboDigitUI.height,
            g_comboNumberTexture, digit, 1, 10);

        digitXaxis += spaceBetweenDigits; // Move to next digit position
    }

    // Combo remaining-time bar (under the combo UI)
    // comboTimer counts down from COMBO_RESET_TIME to 0.
    float ratio = 0.0f;
    if (g_player.COMBO_RESET_TIME > 0.0f) {
        ratio = g_player.comboTimer / g_player.COMBO_RESET_TIME;
    }
    ratio = std::clamp(ratio, 0.0f, 1.0f);

    // Combo timer bar UI
    InGameUI comboTimerBarUI;
    comboTimerBarUI.width = 0.28f;
    comboTimerBarUI.height = 0.025f;
    comboTimerBarUI.x = comboXUI.x + 0.2f;
    comboTimerBarUI.y = 0.5f - 0.02f;

    // background
    SetColor(0.05f, 0.05f, 0.05f, 0.75f);
    RenderImage(comboTimerBarUI.x, comboTimerBarUI.y, comboTimerBarUI.width, comboTimerBarUI.height, g_comboRemainingTimeTexture, 0, 1, 1);

    // fill
    SetColor(1.0f, 0.0f, 0.0f, 0.95f);
    RenderImage(comboTimerBarUI.x - (comboTimerBarUI.width * (1.0f - ratio) * 0.5f), comboTimerBarUI.y,
        comboTimerBarUI.width * ratio, comboTimerBarUI.height, g_comboRemainingTimeTexture, 0, 1, 1);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

// for the gauge bar UI
void DrawGaugeUI(void)
{
    // for the surrounded of the gauge bar. 
    InGameUI gaugeFrameUI;
    gaugeFrameUI.x = -1.0f;
    gaugeFrameUI.y = -0.5f;
    gaugeFrameUI.width = 0.35f;
    gaugeFrameUI.height = 1.15f;

    // for the inner part of the gauge bar
    float barOffsetX = -0.0015f;  // if positive move right, if negative move left
    float barOffsetY = 0.32f;    // if positive move up, if negative move down

    // Gauge bar UI struct (inner filled part)
    InGameUI gaugeBarUI;
    gaugeBarUI.width = gaugeFrameUI.width * 0.5f;
    gaugeBarUI.height = gaugeFrameUI.height * 0.49f;
    // Center horizontally, bottom aligned
    gaugeBarUI.x = gaugeFrameUI.x + (gaugeFrameUI.width - gaugeBarUI.width) * 0.5f + barOffsetX;
    gaugeBarUI.y = gaugeFrameUI.y + (gaugeFrameUI.height - gaugeBarUI.height) * 0.0f + barOffsetY;

    // for the surrounding frame of the gauge bar
    SetColor(1, 1, 1, 1);
    if (g_gaugeBarTexture)
        RenderImage(gaugeFrameUI.x, gaugeFrameUI.y, gaugeFrameUI.width, gaugeFrameUI.height,
            g_gaugeBarTexture, 0, 1, 1);

    // draw the gauge full effect animation when the gauge is full
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

    // for the filled bar calculation
    float fillRatio = 0.0f;

    // Normal gameplay: calculate from gauge points
    if (g_player.MAX_GAUGE_POINTS > 0) {
        fillRatio = (float)g_player.gaugePoints / (float)g_player.MAX_GAUGE_POINTS;
    }

    // If invincible the drain progress will go from top to bottom
    if (g_player.isInvincible && g_player.isGaugeInvincible)
    {
        float drainProgress = g_player.invincibleTimer / g_player.INVINCIBLE_DURATION;
        fillRatio = drainProgress;  // it goes down from top to bottom
        if (fillRatio < 0.0f) {
            fillRatio = 0.0f;
        }
    }

    // draw the filled part of the gauge bar
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

// for the score UI
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

    // Draw the enemy points
    DrawNumber(enemyPoints, scoreUI.x, scoreUI.y, scoreUI.width, scoreUI.height, g_numberTexture);
}


// Game initialization
void InitGameWorld() {
    g_projectileManager.LoadTextures(g_pDevice);

    // FOR THE PLAYER
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
    // 为动画剪辑添加通用名称（不再区分左右）
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


    // FOR THE PLAYER WHEN INVINCIBLE
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
    // 为动画剪辑添加通用名称（不再区分左右）
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


    LoadTexture(g_pDevice, "asset/platform/platformrenga3.png", &g_groundTexture);
    LoadTexture(g_pDevice, "asset/goal.png", &g_goalTexture);
    LoadTexture(g_pDevice, "asset/platform/platform_pass4.png", &g_oneWayPlatformTexture);
    LoadTexture(g_pDevice, "asset/platform/platformtest.png", &g_comboRemainingTimeTexture);
    LoadTexture(g_pDevice, "asset/UI/boss_HP/boss_HP_background.png", &g_bossHealthBarTexture);
    LoadTexture(g_pDevice, "asset/UI/boss_HP/boss_HP.png", &g_bossInnerHPTexture);
    LoadTexture(g_pDevice, "asset/background/1-6background.png", &g_backgroundTexture1);

    // for the signs
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

    // Dash/slash-count UI (follows player): attack_count.png is a 1x3 sheet
    LoadTexture(g_pDevice, "asset/UI/attack_count.png", &g_slashCountTexture);
    g_slashCountAnim.AddClip("SlashCount", 0, 2, 1, 3, 0.12f, true, g_slashCountTexture);
    g_slashCountAnim.SetClip("SlashCount");

    // Health follower spritesheet (1x3)
    LoadTexture(g_pDevice, "asset/UI/Health.png", &g_healthTexture);
    g_healthAnim.AddClip("Health", 0, 2, 1, 3, 0.12f, true, g_healthTexture);
    g_healthAnim.SetClip("Health");

    LoadTexture(g_pDevice, "asset/effect/effect_hit.png", &g_hitEffectTexture);

    LoadTexture(g_pDevice, "asset/effect/slash_flash1.png", &g_slashFlashTextures[0]);
    LoadTexture(g_pDevice, "asset/effect/slash_flash2.png", &g_slashFlashTextures[1]);
    LoadTexture(g_pDevice, "asset/effect/slash_flash3.png", &g_slashFlashTextures[2]);
    LoadTexture(g_pDevice, "asset/effect/slash_flash4.png", &g_slashFlashTextures[3]);

	// for the gauge bar 
    LoadTexture(g_pDevice, "asset/UI/gauge/gauge_frame.png", &g_gaugeBarTexture);
    LoadTexture(g_pDevice, "asset/UI/gauge/gauge_filled.png", &g_gaugeBarFilledTexture);
    LoadTexture(g_pDevice, "asset/UI/gauge/gauge_effect.png", &g_gaugeFullEffectTexture);
    g_gaugeEffectAnim.AddClip("GaugeFull", 0, 9, 10, 1, 0.08f, true, g_gaugeFullEffectTexture);

    // Gauge mode trailing particle (1x5)
    LoadTexture(g_pDevice, "asset/effect/particle_sheet.png", &g_gaugeTrailParticleTexture);

    // Gauge kill burst particle (1x5)
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

// Modified game update function
void UpdateGame(float deltaTime) {
    if (g_gameState != STATE_PLAYING) {
        return;
    }
    // `animLockDuration` is a constant duration; `animLockTimer` is the running countdown.
    // Decrementing the duration itself can break the lock/unlock logic and freeze animation transitions.
    if (g_player.animLockTimer > 0.0f) {
        g_player.animLockTimer -= deltaTime;
        if (g_player.animLockTimer < 0.0f) {
            g_player.animLockTimer = 0.0f;
        }
    }

    // Update audio manager
    g_gameTimer.Tick(); // added december 3rd

    signAnim.Update(deltaTime);
    g_slashCountAnim.Update(deltaTime);

    // added december 4th
    g_gameElapsedTime += deltaTime;
    g_gameMinutes = static_cast<int>(g_gameElapsedTime) / 60;
    g_gameSeconds = static_cast<int>(g_gameElapsedTime) % 60;

    g_gameStats.UpdateTime(g_gameElapsedTime); // track total time

    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY);

    // Shoot fireball from player position towards mouse position
    /*g_projectileManager.CreateFireball(
        g_player.posX + PLAYER_WIDTH / 2,  // Shoot from player center
        g_player.posY + PLAYER_HEIGHT / 2,
        mouseX,
        mouseY,
        true  // From player
    );*/

    // Update slow motion effect
    if (g_isSlowMotion) {
        g_slowMoTimer -= deltaTime;
        if (g_slowMoTimer <= 0.0f) {
            g_isSlowMotion = false;
            g_slowMoFactor = 1.0f; // Restore normal time
        }
    }

    // Apply time scaling effect (priority: global slow motion > dash-end slow motion > charge effect)
    float timeScale = 1.0f;
    if (g_isSlowMotion) {
        timeScale = g_slowMoFactor; // Use slow motion factor

    }
    else if (g_player.isInDashEndSlowMo) {
        timeScale = g_player.DASH_END_SLOWMO_FACTOR;
    }
    else if (g_player.isCharging) {
        float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;
        chargeRatio = std::min(chargeRatio * 8, 1.0f);
        timeScale = 1.0f - chargeRatio * 0.8f;
    }


    // for updating the combo timer
    if (g_player.comboCount > 0) {
        g_player.comboTimer -= deltaTime;
        if (g_player.comboTimer <= 0.0f) {
            g_player.comboCount = 0;
            g_player.comboTimer = 0.0f;
        }
        g_gameStats.UpdateMaxCombo(g_player.comboCount);
    }

    // Acceleration state:
    // - active when comboCount > threshold
    // - also active during gauge-based invincibility (gauge/full-gauge reward during kill streak)
    g_player.isAccelerated = (g_player.comboCount > ACCEL_COMBO_THRESHOLD) || (g_player.isInvincible && g_player.isGaugeInvincible);

    // for updating the invincibility timer
    if (g_player.isInvincible) {
        const float prevInvTime = g_player.invincibleTimer;
        g_player.invincibleTimer -= deltaTime;

        // Gauge invincibility: 1-second warning (play once)
        if (g_player.isGaugeInvincible && prevInvTime > 1.0f && g_player.invincibleTimer <= 1.0f) {
            Audio::PlaySE(SoundEffect::INVINCIBLE_WARNING);
        }

        if (g_player.invincibleTimer <= 0.0f) {
            g_player.isInvincible = false;
            g_player.isGaugeInvincible = false;
            g_player.invincibleTimer = 0.0f;
        }
    }

    // Auto-activate invincibility when gauge is full
    if (!g_player.isInvincible && g_player.gaugePoints >= g_player.MAX_GAUGE_POINTS) {
        g_player.isInvincible = true;
        g_player.isGaugeInvincible = true;
        g_player.invincibleTimer = g_player.INVINCIBLE_DURATION;
        g_player.g_gaugeEffectActive = true; 
        g_player.g_gaugeEffectTimer = g_player.INVINCIBLE_DURATION;
        g_player.gaugePoints = 0;

        Audio::PlaySE(SoundEffect::LIMITBREAK, 2.0f);
        // Spawn a dramatic particle burst at the player's center when gauge activates
        {
            float centerX = g_player.posX + PLAYER_WIDTH * 0.5f;
            float centerY = g_player.posY + PLAYER_HEIGHT * 0.5f;
            SpawnGaugeFullBurst(centerX, centerY);
        }
    }

	// for the gauge effect timer
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

    // Gauge trail particles: active only during gauge-based invincibility.
    if (g_player.isInvincible && g_player.isGaugeInvincible && !g_player.isDead) {
        g_gaugeTrailSpawnTimer -= scaledDeltaTime;
        if (g_gaugeTrailSpawnTimer <= 0.0f) {
            // Spawn around player's body (random within an ellipse around the player).
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

            // 1.5x spawn rate => interval gets smaller.
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

    // Update / spawn player afterimages
    for (auto it = g_playerAfterImages.begin(); it != g_playerAfterImages.end();) {
        it->timer += scaledDeltaTime;
        if (it->timer >= it->duration) {
            it = g_playerAfterImages.erase(it);
        }
        else {
            ++it;
        }
    }

    // Use displacement-based speed so afterimages still spawn even if velocity is
    // temporarily overridden/reset during input/physics updates.
    static float s_prevPlayerX = g_player.posX;
    static float s_prevPlayerY = g_player.posY;
    float dtForSpeed = (scaledDeltaTime > 1e-6f) ? scaledDeltaTime : 1e-6f;
    float dx = g_player.posX - s_prevPlayerX;
    float dy = g_player.posY - s_prevPlayerY;
    float speed = sqrtf(dx * dx + dy * dy) / dtForSpeed;
    s_prevPlayerX = g_player.posX;
    s_prevPlayerY = g_player.posY;

    // Afterimages:
    // - Always during dash.
    // - Also during accelerated state (combo) while moving/running.
    // - When not accelerated, dash afterimages spawn at half rate (interval doubled).
    bool shouldSpawnAfterImage = !g_player.isDead && (g_player.isDashing || g_player.isAccelerated);
    if (shouldSpawnAfterImage) {
        float interval = g_player.afterImageSpawnInterval;
        // Only slow down afterimage rate for non-accelerated dash.
        if (g_player.isDashing && !g_player.isAccelerated) {
            interval *= 2.0f;
        }

        // If we're only accelerated (not dashing), require actual movement.
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
        // Update game logic using adjusted time
        UpdateDash(deltaTime);
        UpdatePlayerPhysics(scaledDeltaTime);

        UpdateEnemies(scaledDeltaTime, &g_mapManager);
        // Update all projectiles
        g_projectileManager.Update(scaledDeltaTime, &g_mapManager, g_enemies);
        // 在UpdateGame函数中修改动画设置部分
        if (g_player.animLockTimer <= 0.0f)
        {
            // Only use invincible animation set during gauge-based invincibility.
            // Dash/slash post-invincibility should not override current animation.
            if (g_player.isInvincible && g_player.isGaugeInvincible)
            {
                // INVINCIBLE ANIMATIONS
                if (g_player.isDead)
                {
                    // Death animation stays normal (no invincible death texture)
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
                    // Determine slash direction
                    float dx = g_player.dashDirectionX;
                    float dy = g_player.dashDirectionY;

                    const float DIAG_RATIO = 0.70710678f;
                    const float adx = fabsf(dx);
                    const float ady = fabsf(dy);

                    const char* clip;
                    if (dy > 0.0f && ady >= adx * DIAG_RATIO) {
                        clip = "InvincibleSlash2";  // Up
                    }
                    else if (dy < 0.0f && ady >= adx * DIAG_RATIO) {
                        clip = "InvincibleSlash4";  // Down
                    }
                    else if (adx >= ady * DIAG_RATIO) {
                        clip = "InvincibleSlash3";  // Side
                    }
                    else {
                        clip = "InvincibleSlash1";  // Diagonal
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
                    if (g_player.velocityY < 0.0f) // Falling
                    {
                        if (g_player.anim.GetCurrentClipName() != "InvincibleFalling") {
                            g_player.anim.SetClip("InvincibleFalling");
                        }
                    }
                    else // Jumping
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
                else // Idle
                {
                    if (g_player.anim.GetCurrentClipName() != "InvincibleIdle") {
                        g_player.anim.SetClip("InvincibleIdle");
                    }
                }
            }
            else
            {
                if (g_player.isDead) // for when dying
                {
                    if (g_player.anim.GetCurrentClipName() != "Death") {
                        g_player.anim.SetClip("Death");
                    }
                }

                else if (g_player.isCharging) // 如果玩家正在蓄力
                {
                    if (!g_player.isOnGround) // 如果玩家在空中蓄力
                    {
                        if (g_player.anim.GetCurrentClipName() != "AirCharge") {
                            g_player.anim.SetClip("AirCharge");
                        }
                    }
                    else // 如果玩家在地面蓄力
                    {
                        if (g_player.anim.GetCurrentClipName() != "GroundCharge") {
                            g_player.anim.SetClip("GroundCharge");
                        }
                    }
                }
                else if (g_player.isDashing) // 如果玩家正在冲刺
                {
                    // Slash clip depends on dash direction.
                    // Classify into 4 sectors with 45° boundaries:
                    //   Up:    |dy| dominates and dy > 0  -> Slash2
                    //   Down:  |dy| dominates and dy < 0  -> Slash4
                    //   Side:  otherwise, |dx| dominates -> Slash3
                    //   DiagDown-ish fallback            -> Slash1
                    // Left/right is handled by facing/flip.
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
                        // diagonal-ish: choose a dedicated diagonal-down slash
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
                else if (!g_player.isOnGround) // 玩家不在地面上
                {
                    if (g_player.velocityY < 0.0f) // 下落
                    {
                        if (g_player.anim.GetCurrentClipName() != "Falling") {
                            g_player.anim.SetClip("Falling");
                        }
                    }
                    else // 跳跃
                    {
                        if (g_player.anim.GetCurrentClipName() != "Jump") {
                            g_player.anim.SetClip("Jump");
                        }
                    }
                }
                else if (g_player.isMoving) // 玩家在移动
                {
                    if (g_player.anim.GetCurrentClipName() != "Run") {
                        g_player.anim.SetClip("Run");
                    }
                }
                else // 玩家站立
                {
                    if (g_player.anim.GetCurrentClipName() != "Idle") {
                        g_player.anim.SetClip("Idle");
                    }
                }
            }
        }

        // Keep animation speed in sync with accelerated state
        g_player.anim.Update(scaledDeltaTime * g_player.GetAnimSpeedMultiplier());
        UpdatePlayerDeath(scaledDeltaTime);
    }
    g_mouseIndicator.Update(scaledDeltaTime);
}

// Helper function: Get texture based on tile code
ID3D11ShaderResourceView* GetTextureForTile(const std::string& tileCode) {
    if (tileCode == "G1" || tileCode == "G2" || tileCode == "G3") {
        return g_groundTexture;
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
    else {
        return g_groundTexture;
    }
}

// Helper function: Set color based on tile code
void SetTileColor(const std::string& tileCode) {
    if (tileCode == "G1") {
        //SetColor(0.4f, 0.8f, 0.3f, 1.0f);
        SetColor(0.7f, 0.7f, 0.7f, 1.0f);
    }
    else if (tileCode == "G2") {
        SetColor(0.6f, 0.4f, 0.2f, 1.0f);
    }
    else if (tileCode == "G3") {
        SetColor(0.5f, 0.5f, 0.5f, 1.0f);
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
    int currentWidth = g_camera.GetWidth();
    int currentHeight = g_camera.GetHeight();
    float aspectRatio = static_cast<float>(currentWidth) / static_cast<float>(currentHeight);

    float cameraX = g_camera.GetX();
    float cameraY = g_camera.GetY();

    auto worldToScreen = [cameraX, cameraY](float worldX, float worldY) -> std::pair<float, float> {
        return { worldX - cameraX, worldY - cameraY };
        };

    // Advance health icon animation
    g_healthAnim.Update(g_gameTimer.GetDeltaTime());

    // Draw slash-count icons will be rendered later (moved down) so they are not occluded by terrain

    // Draw player afterimages (behind player)
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

    // Draw gauge trail particles (behind player)
    for (const auto& pInst : g_gaugeTrailParticles) {
        if (!pInst.active || !pInst.texture) continue;

        float t = static_cast<float>(pInst.frame) / static_cast<float>(GaugeTrailParticleInstance::frameCount);
        t = std::clamp(t, 0.0f, 1.0f);
        // Ease-out fade, looks more "natural" than linear.
        float alpha = (1.0f - t);
        alpha = alpha * alpha;
        alpha *= 0.9f;
        auto p = worldToScreen(pInst.x, pInst.y);

        // Slightly bluish-white to match gauge vibe.
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

        // Hit effects are stored in WORLD coordinates, but `RenderImage` draws in SCREEN coordinates.
        // Convert to screen space before rendering, otherwise the effect appears offset by camera.
        // Disable culling here because culling uses world-space camera checks.
        auto p = worldToScreen(e.x - size * 0.5f, e.y - size * 0.5f);
        RenderImage(p.first, p.second,
            size, size, e.texture,
            e.frame, e.rows, e.columns,
            false);
    }

    // Background disabled temporarily for debugging visibility
    // (was: parallax background draw)

    // Use new map system to draw tiles
    if (g_mapManager.IsMapLoaded()) {
        Map* currentMap = g_mapManager.GetCurrentMap();

        // Draw background layer tiles
        auto& bgTiles = currentMap->GetTiles(MapLayer::BACKGROUND);
        for (const auto& tile : bgTiles) {
            if (tile.tileInfo.code == "00") continue;
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
        }

        // Draw midground layer tiles (player activity layer)
        auto& mgTiles = currentMap->GetTiles(MapLayer::MIDGROUND);
        for (const auto& tile : mgTiles) {
            if (tile.tileInfo.code == "00") continue;
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);


            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            //RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
           
            // for the goal
			if (tile.tileInfo.code == "DF" || tile.tileInfo.code == "DI" || tile.tileInfo.code == "D4" || tile.tileInfo.code == "D5" || 
                tile.tileInfo.code == "D6" || tile.tileInfo.code == "D7" || tile.tileInfo.code == "DB" || tile.tileInfo.code == "21" || tile.tileInfo.code == "22" || 
                tile.tileInfo.code == "23" || tile.tileInfo.code == "24" || tile.tileInfo.code == "25" || tile.tileInfo.code == "26" || tile.tileInfo.code == "27" ||
                tile.tileInfo.code == "31" || tile.tileInfo.code == "32" || tile.tileInfo.code == "33" || tile.tileInfo.code == "34" || tile.tileInfo.code == "35" ||
                tile.tileInfo.code == "36" || tile.tileInfo.code == "37") {
                // scale the texture to match the collision (0.1f / 0.15f = 0.67) bc thats the size of the actual block in the game
                float renderScale = 2.0f;  // Adjust this to match your collision size
                float renderWidth = tile.width; // no change
                float renderHeight = tile.height * renderScale;

                // center the sprite on the tile position
                float offsetX = (tile.width - renderWidth) * 0.5f;
                float offsetY = (tile.height - renderHeight) * 0.1f;

                RenderImage(screenPos.first + offsetX, screenPos.second + offsetY,
                    renderWidth, renderHeight, texture, 0, 1, 1);
            }

            //// for one way platforms. they will be a bit smaller to match collision with character. might change later
            //else if (tile.tileInfo.code == "OP") {
            //    // scale the texture to match the collision (0.1f / 0.15f = 0.67) bc thats the size of the actual block in the game
            //    float renderScale = 0.67f;  // Adjust this to match your collision size
            //    float renderWidth = tile.width; // no change
            //    float renderHeight = tile.height * renderScale;

            //    // center the sprite on the tile position
            //    float offsetX = (tile.width - renderWidth) * 0.5f;
            //    float offsetY = (tile.height - renderHeight) * 0.5f;

            //    RenderImage(screenPos.first + offsetX, screenPos.second + offsetY,
            //        renderWidth, renderHeight, texture, 0, 1, 1);
            //}
            
            // for the animation of the signs
            else if (tile.tileInfo.code == "B1" || tile.tileInfo.code == "B2" || tile.tileInfo.code == "B3" || tile.tileInfo.code == "B4" || tile.tileInfo.code == "B5" ||
                     tile.tileInfo.code == "B6" || tile.tileInfo.code == "B7" || tile.tileInfo.code == "B8" || tile.tileInfo.code == "B9") {
               
                // for getting the right texture depending on the sign
                ID3D11ShaderResourceView* signTexture = signAnim.GetCurrentClipTexture();
                int numFrames = 4; // default if 4 frames

                if (tile.tileInfo.code == "B1") signTexture = g_signWASDTexture;
                else if (tile.tileInfo.code == "B2") signTexture = g_signSTexture;
                else if (tile.tileInfo.code == "B3") {
                    signTexture = g_signRightTexture;
                    numFrames = 5; // only this sign is 5 frames instead of 4
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
                    // calculate the frames
                    int currentFrame = signAnim.GetCurrentFrame() % numFrames;

                    RenderImage(screenPos.first, screenPos.second, tile.width, tile.height * 2.0f, // *2.0f so it will be the correct size for the sign (twice as a big ad the actual tile)
                        signTexture,
                        currentFrame,
                        signAnim.GetSplitX(),
                        numFrames);
                }
            }
            else {
                // Normal rendering for all other tiles
                RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
            }
        }

        // Draw foreground layer tiles
        auto& fgTiles = currentMap->GetTiles(MapLayer::FOREGROUND);
        for (const auto& tile : fgTiles) {
            if (tile.tileInfo.code == "00") continue;
            std::pair<float, float> screenPos = worldToScreen(tile.posX, tile.posY);
            ID3D11ShaderResourceView* texture = GetTextureForTile(tile.tileInfo.code);
            SetTileColor(tile.tileInfo.code);
            RenderImage(screenPos.first, screenPos.second, tile.width, tile.height, texture, 0, 1, 1);
        }
    }

    // Draw charge effect
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

    // Draw dash effect
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

    // Draw gauge-kill red particles (burst)
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
    // Draw slash-count icons (followers) here so they render after tiles and are not occluded by terrain
    // Hide during gauge-based invincibility.
    if (!(g_player.isInvincible && g_player.isGaugeInvincible) && g_slashCountTexture) {
        const int count = std::clamp(g_player.dashPoints, 0, g_player.MAX_DASH_POINTS);
        if (count > 0) {
            struct Follower {
                float x = 0.0f;
                float y = 0.0f;
                bool init = false;
            };

            static Follower s_follow[3];

            // Sprite size in world units
            const float iconW = 0.065f;
            const float iconH = 0.065f;
            const float spacing = iconW * 0.60f;

            // Target anchor: one grid tile behind player (respect facing)
            const float behind = GRID_WIDTH;
            const float baseTargetX = g_player.posX + (g_player.facingRight ? -behind : behind);
            const float baseTargetY = g_player.posY + PLAYER_HEIGHT * 0.55f;

            // Per-icon offsets (so they don't overlap and are not aligned)
            const float xDir = g_player.facingRight ? -1.0f : 1.0f; // extend further behind
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
            int pendingCost = std::clamp(g_player.chargePendingCost, 0, g_player.MAX_DASH_POINTS);
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

    // Draw player
    if (!g_player.isDead) {
        // Normal drawing when alive
        std::pair<float, float> playerPos = worldToScreen(g_player.posX, g_player.posY);

        // for the size of the character
        float scale = 6.6f;
        float width = PLAYER_WIDTH * scale;
        float height = PLAYER_HEIGHT * scale;

        // center the bigger sprite on collision box
        float offsetX = (width - PLAYER_WIDTH) * 0.5f;
        float offsetY = (height - PLAYER_HEIGHT) * 0.5f;

        // 获取当前动画剪辑的纹理
        ID3D11ShaderResourceView* currentTexture = g_player.anim.GetCurrentClipTexture();
        int frameIndex = g_player.anim.GetCurrentFrame();
        int splitX = g_player.anim.GetSplitX();
        int splitY = g_player.anim.GetSplitY();

        // 根据朝向决定是否水平翻转
        // 如果facingRight为true（面向右），不翻转
        // 如果facingRight为false（面向左），水平翻转
        bool flipHorizontal = !g_player.facingRight;


        //SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(playerPos.first - offsetX, playerPos.second - offsetY, width, height,
            currentTexture, frameIndex, splitY, splitX, true, 0.0f, flipHorizontal);

        SetColor(1.0f, 1.0f, 1.0f, 1.0f);  // Reset color after being invincible




        // for the attack count 
        
        //// Position top left of the player
        //float countWidth = 0.1f;   
        //float countHeight = 0.15f; 
        //float offsetCountX = -0.08f;  // hozirontal
        //float offsetCountY = PLAYER_HEIGHT /*+ 0.01f*/; // vertical

        //float countXPos = g_player.posX + offsetCountX;
        //float countYPos = g_player.posY + offsetCountY;

        //auto countScreenPos = worldToScreen(countXPos, countYPos);

        //int frameIndexC = 0;
        //if (g_player.dashPoints == 3) {
        //    frameIndexC = 0;  // Show 3 triangles
        //}
        //else if (g_player.dashPoints == 2) {
        //    frameIndexC = 1;  // Show 2 triangles
        //}
        //else {
        //    frameIndexC = 2;  // Show 1 triangle (for dashPoints 1 or 0)
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
        // Death animation
        std::pair<float, float> playerPos = worldToScreen(g_player.posX, g_player.posY);

        float scale = 6.6f;
        float width = PLAYER_WIDTH * scale;
        float height = PLAYER_HEIGHT * scale;

        float offsetX = (width - PLAYER_WIDTH) * 0.5f;
        float offsetY = (height - PLAYER_HEIGHT) * 0.5f;

        // Get death animation texture and frame
        ID3D11ShaderResourceView* currentTexture = g_player.anim.GetCurrentClipTexture();
        int frameIndex = g_player.anim.GetCurrentFrame();
        int splitX = g_player.anim.GetSplitX();
        int splitY = g_player.anim.GetSplitY();

        bool flipHorizontal = !g_player.facingRight;

        // Render death animation
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(playerPos.first - offsetX, playerPos.second - offsetY, width, height,
            currentTexture, frameIndex, splitY, splitX, true, 0.0f, flipHorizontal);
    }

    DrawComboUI();
    DrawGaugeUI();
    DrawScoreUI();

    // for the esc texture
    if (g_escTexture) {
        // ESC button UI
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

    // During death animation (death happens on hit), disable all gameplay inputs.
    // Otherwise movement input below would still flip `g_player.facingRight`.
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
    // Right click: keep as cancel-charge only (invincibility is now auto-triggered)
    if (g_inputSystem.IsMouseRightDown()) {
        CancelChargeDash();
    }
    // for pausing the game press P or Esc key
    if (g_inputSystem.IsTogglePressed(VK_P) || g_inputSystem.IsTogglePressed(VK_ESCAPE))
    {
        SCENE currentScene = sceneManager.GetCurrentSceneType();

        if (currentScene == GAMEPLAY || currentScene == CAKE) // the areas and the cake scene
        {
            sceneManager.SaveBGMPath(Audio::GetCurrentBGMPath());
            sceneManager.SwitchScene(PAUSE);  // you can pause the game at any stage
            Audio::PauseBGM();
        }

        // if you press P or Esc key again you can go back to the stage (you can use the mouse and click the continue button
        else if (currentScene == PAUSE)
        {
            SCENE previousScene = sceneManager.GetOriginalPausedScene();
            if (previousScene == GAMEPLAY || previousScene == CAKE) // the areas and the cake scene
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

    // If the current active scene is the pause scene, do not process any
    // further gameplay inputs (movement, dash, jump, mouse charge, etc.).
    // Pause toggle handling above still executes so player can resume.
    if (sceneManager.GetCurrentSceneType() == PAUSE) {
        return;
    }

    // Get mouse input state
    bool isMouseLeftPressed = g_inputSystem.IsMouseLeftPressed();
    bool isMouseLeftDown = g_inputSystem.IsMouseLeftDown();
    bool isMouseLeftReleased = g_inputSystem.IsMouseLeftReleased();

    // Charge-dash input mode (VK_T):
    // - true (default): always dash on release even if a saved charge exists.
    // - false: if a saved charge exists, mouse press dashes immediately.
    if (g_inputSystem.IsTogglePressed(VK_T)) {
        g_releaseDashChargeMode = !g_releaseDashChargeMode;
    }

    // Dash aftermath behavior toggle (VK_G):
    // - false (default): aftermath uses existing gravity/physics behavior.
    // - true: aftermath ignores gravity; movement input breaks aftermath (already).
    if (g_inputSystem.IsTogglePressed(VK_G)) {
        g_noGravityAftermathMode = !g_noGravityAftermathMode;
    }

    static bool wasMouseLeftDown = false;

    // Pure mouse control: press to start charging
    if (isMouseLeftPressed) {
        if (!g_releaseDashChargeMode) {
            // Legacy: if we already have a saved charge, pressing dashes immediately.
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
            // New mode: always start charging, dash will happen on release.
            StartMouseChargeDash();
        }
    }

    // Pure mouse control: release to execute dash
    if (isMouseLeftReleased && wasMouseLeftDown && g_player.isCharging) {
        if (g_releaseDashChargeMode) {
            // Hold < 0.2s: chain previous saved charge (if any).
            // Hold >= 0.2s: use the new charge (override saved).
            if (g_player.chargeTime < g_player.CHARGE_THRESHOLD_LOW) {
                if (g_player.hasSavedCharge) {
                    g_player.LoadSavedCharge();
                }
            }
            else {
                // Force_execute uses current charge by making sure we don't fall back to saved.
                g_player.ClearSavedCharge();
            }
        }

        ExecuteMouseChargeDash();
    }

    // Cancel charging
    if (!isMouseLeftDown && g_player.isCharging) {
        CancelChargeDash();
    }

    wasMouseLeftDown = isMouseLeftDown;

    // Movement control
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

    // Requested: movement should interrupt dash-end slow motion.
    // Note: movement input is handled here directly (not always via MovePlayerLeft/Right),
    // so break the state at the input level.
    if (moving && g_player.isInDashEndSlowMo) {
        g_player.isInDashEndSlowMo = false;
        g_player.dashEndSlowMoTimer = 0.0f;
    }

    if (!moving && !g_player.isDashing) {
        g_player.velocityX = 0.0f;
        g_player.isMoving = false;
    }

    // Jump control
    // Allow both Space and W to trigger a jump (including while wall-sliding).
    static bool wasSpacePressed = false;
    static bool wasWPressed = false;

    bool spaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    bool wDown = (GetAsyncKeyState('W') & 0x8000) != 0;

    // Trigger jump on edge (pressed this frame but not previous). If both are
    // pressed simultaneously, only call Jump() once.
    if ((spaceDown && !wasSpacePressed) || (wDown && !wasWPressed)) {
        Jump();
    }

    wasSpacePressed = spaceDown;
    wasWPressed = wDown;
}

// MouseIndicatorSystem implementation
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
    //// Do not show mouse indicator if protagonist is dead
    //if (g_player.isDead) return;
    auto worldToScreen = [cameraX, cameraY](float worldX, float worldY) -> std::pair<float, float> {
        return { worldX - cameraX, worldY - cameraY };
        };

    // Draw mouse position indicator
    float cursorWidth = 0.04f;
    float cursorHeight = 0.12f;
    auto mousePos = worldToScreen(m_mouseWorldX - cursorWidth / 2, m_mouseWorldY - cursorHeight / 2);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    RenderImage(mousePos.first, mousePos.second, cursorWidth, cursorHeight,
        m_cursorTexture, 0, 1, 1, false, 0);
    // Mouse cursor is rendered by `g_gameCursor` globally.

    // (Removed fixed numeric dash-points UI — followers still render above player.)

    // (Removed debug numeric toggle UI for T/G modes)

    
    float uiX = -1.0f;
    float uiY = 0.25f;
    float uiWidth = 0.5f;
    float uiHeight = 1.0f;
    RenderImage(uiX, uiY, uiWidth, uiHeight, g_uiNumberTexture, 0, 1, 1);

    // for the timer counting
    float timerX = -0.768f;  // position x axis
    float timerY = 0.78f;   // position y axis
    float timerDigitWidth = 0.03f;  // width
    float timerDigitHeight = 0.07f; // height

    // for the minutes
    int minuteTens = g_gameMinutes / 10;
    int minuteOnes = g_gameMinutes % 10;
    RenderNumber(minuteTens, timerX, timerY, timerDigitWidth, timerDigitHeight, pTextureNum);
    RenderNumber(minuteOnes, timerX + timerDigitWidth * 0.8f, timerY, timerDigitWidth, timerDigitHeight, pTextureNum);

    // for the seconds
    int secondTens = g_gameSeconds / 10;
    int secondOnes = g_gameSeconds % 10;
    float secondsStartX = timerX + timerDigitWidth * 2.2f;
    RenderNumber(secondTens, secondsStartX, timerY, timerDigitWidth, timerDigitHeight, pTextureNum);
    RenderNumber(secondOnes, secondsStartX + timerDigitWidth * 0.8f, timerY, timerDigitWidth, timerDigitHeight, pTextureNum);

    //SetColor(1.0f, 1.0f, 1.0f, 1.0f); 

    if (!m_showMouseIndicator) return;
    // Do not show mouse indicator if protagonist is dead
    if (g_player.isDead) return;

    // Draw direction arrow

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

            // Get effective charge time for arrow preview.
            // When chaining (short hold) and a saved charge exists, the dash will actually use the saved charge.
            // So the arrow must preview that saved charge, otherwise it looks too short on the second dash.
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

            // Determine charge level and use the same multipliers as in ExecuteMouseChargeDash
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

            // ExecuteMouseChargeDash and DashToMouse apply an extra 1.3x multiplier
            // for short (1-point) dashes; the preview should match that behavior.
            const float SHORT_DASH_EXTRA = 1.3f;

            // Calculate the ACTUAL world distance the player will travel
            // Distance = velocity × time × 60.0 (matching UpdatePlayerPhysics)
            float dashSpeed = DASH_SPEED * speedMultiplier * SHORT_DASH_EXTRA;
            float dashDuration = DASH_DURATION * durationMultiplier;
            float arrowLength = dashSpeed * dashDuration * 60.0f;

            float tailX = playerCenterX;
            float tailY = playerCenterY;

            // Calculate the center of the arrow (midpoint between tail and head)
            float arrowCenterX = tailX + cosf(m_arrowAngle) * (arrowLength * 0.5f);
            float arrowCenterY = tailY + sinf(m_arrowAngle) * (arrowLength * 0.5f);

            // Position calculation for screen rendering
            auto arrowScreenPos = worldToScreen(arrowCenterX - arrowLength * 0.5f, arrowCenterY - arrowWidth * 0.5f);

            // Get charge level for color display
            int chargeLevel = g_player.GetChargeLevelFromTime(chargeTime);

            // Display different colors based on charge level
            if (chargeLevel >= 3) {
                //SetColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
            }
            else if (chargeLevel >= 2) {
                //SetColor(0.0f, 0.0f, 1.0f, 1.0f); // Dark blue
            }
            else if (chargeLevel >= 1) {
                //SetColor(0.0f, 1.0f, 1.0f, 1.0f); // Blue
            }

            SetColor(1.0f, 1.0f, 1.0f, 1.0f);
            // The arrow texture is a 1x3 spritesheet: frames 0 and 1 are body segments, frame 2 is the head.
            // Instead of stretching the whole image, repeat frames 0/1 to form the shaft, then draw frame 2 as the head.
            float dirX = cosf(m_arrowAngle);
            float dirY = sinf(m_arrowAngle);

            // Choose head width in world units. Keep head height equal to arrowWidth.
            float headWidth = arrowWidth; // try to keep head unscaled horizontally relative to its height

            // If arrow is too short, fall back to original stretched rendering
            if (arrowLength <= headWidth * 1.1f) {
                RenderImage(arrowScreenPos.first, arrowScreenPos.second, arrowLength, arrowWidth,
                    g_arrowTexture, 0, 1, 1, false, m_arrowAngle);
            }
            else {
                float bodyLength = arrowLength - headWidth;

                // Determine number of body segments. Use headWidth as a nominal segment unit to keep visuals consistent.
                int segmentCount = (int)ceilf(bodyLength / headWidth);
                if (segmentCount < 1) segmentCount = 1;

                float segmentWidth = bodyLength / (float)segmentCount;

                // Draw each body segment, alternating frames 0 and 1
                for (int i = 0; i < segmentCount; ++i) {
                    float segCenterDist = (segmentWidth * (i + 0.5f));
                    float segCenterX = tailX + dirX * segCenterDist;
                    float segCenterY = tailY + dirY * segCenterDist;

                    auto segScreenPos = worldToScreen(segCenterX - segmentWidth * 0.5f, segCenterY - arrowWidth * 0.5f);

                    int frame = (i % 2 == 0) ? 0 : 1; // alternate frame 0 and 1
                    RenderImage(segScreenPos.first, segScreenPos.second, segmentWidth, arrowWidth,
                        g_arrowTexture, frame, 1, 3, false, m_arrowAngle);
                }

                // Draw head at the tip
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

    // Legacy API: cursor visibility is now controlled by the global in-game cursor.
    // Keep this so existing scene code doesn't need to manage cursor visibility.
    SetInGameCursorEnabled(i);
}


// Reset all statistics
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

// Increment kill counter
void GameStatistics::IncrementKills() {
    enemiesKilled++;
    currentKills++;
    lifetimeKills++;
    AddEnemyPoints(10);
}

// Increment weak point kill counter
void GameStatistics::IncrementWeakPointKills() {
    weakPointKills++;
    currentWeakKills++;
    lifetimeWeakKills++;
    AddEnemyPoints(30);
}

// Increment death counter
void GameStatistics::IncrementDeaths() {
    totalDeaths++;

    //erase later
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

    // erase later
    sprintf_s(debugMsg, "Remaining Total Points: %d\n", totalEnemyPoints);
    OutputDebugStringA(debugMsg);
    OutputDebugStringA("==========================\n\n");

}

// for updating the maximum combo
void GameStatistics::UpdateMaxCombo(int combo) {
    if (combo > maxCombo) {
        maxCombo = combo;
    }
    if (combo > currentMaxCombo) {
        currentMaxCombo = combo; // current max combo
    }
}
//  when player dies, resets the area progress
void GameStatistics::ResetAreaProgress() {
    //totalEnemyPoints -= currentAreaEnemyPoints; // Subtract current area points from total
    currentAreaEnemyPoints = 0; // Reset current area progress
    //maxCombo = 0;  // Reset max combo on death
    currentMaxCombo = 0;
}


void GameStatistics::AddEnemyPoints(int points) {
    currentAreaEnemyPoints += points;
    totalEnemyPoints += points;
    currentScore += points;
    lifetimeEnemyPoints += points;
}

// Update total time
void GameStatistics::UpdateTime(float time) {
    totalTime = time;
}

// Calculate the final score based on kills, time, and deaths
void GameStatistics::CalculateFinalScore() {
    int comboMultiplier = std::max(1, maxCombo);  // Minimum combo is 1

    // Convert time to 4-digit number (total seconds)
    //int timeInSeconds = static_cast<int>(totalTime);
    int minutes = static_cast<int>(totalTime) / 60;
    int seconds = static_cast<int>(totalTime) % 60;
    int timeInMMSS = (minutes * 100) + seconds;  // MMSS format

    // Cap deaths at 50 for penalty calculation
    int cappedDeaths = std::min(50, totalDeaths);
    int deathPenalty = cappedDeaths * 50;

    // Calculate base score
    int baseScore = lifetimeEnemyPoints * comboMultiplier;

    // Calculate penalty
    int penalty = timeInMMSS + deathPenalty;

    int penaltyMultiplied = static_cast<int>(penalty * 1.5f); // so it gives me an integrer number
    totalScore = baseScore - penaltyMultiplied;

    // Ensure score doesn't go negative
    totalScore = std::max(0, totalScore);

    // erase later
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
        totalScore = 0; // so there will not be negative score
    }
}

void GameStatistics::ResetCurrentStats() {
    currentScore = 0;
    currentKills = 0;
    currentWeakKills = 0;
    currentAreaEnemyPoints = 0;
    //totalEnemyPoints = 0;
}
