#include "Game.h"
#include "Render.h"

// 全局设备指针（需要从外部设置）
extern ID3D11Device* g_pDevice;

std::vector<GameBlock> g_mapBlocks;
ID3D11ShaderResourceView* g_playerTexture = nullptr;
ID3D11ShaderResourceView* g_groundTexture = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture = nullptr;
ID3D11ShaderResourceView* g_dashEffectTexture = nullptr;
ID3D11ShaderResourceView* g_chargeEffectTexture = nullptr; // 蓄力特效纹理

void Game::Init() {
    // 加载纹理
    LoadTexture(g_pDevice, "asset/block.png", &m_playerTexture);
    LoadTexture(g_pDevice, "asset/blockB.png", &m_groundTexture);
    LoadTexture(g_pDevice, "asset/Space.png", &m_backgroundTexture);
    LoadTexture(g_pDevice, "asset/block.png", &m_dashEffectTexture);
    LoadTexture(g_pDevice, "asset/blockB.png", &m_chargeEffectTexture);

    m_physicsSystem.CreateTestMap();
    Reset();
}

void Game::Update() {
    if (m_gameState != STATE_PLAYING) {
        return;
    }

    // 更新冲刺状态
    m_dashSystem.UpdateDash(m_player, m_timer.GetDeltaTime());

    // 更新物理
    m_physicsSystem.UpdatePlayerPhysics(m_player, m_timer.GetDeltaTime());
}


// 修改后的渲染函数
void  Game::Draw() {
    RendererDrawF();

    // 绘制背景
    RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, g_backgroundTexture, 0, 1, 1);

    // 绘制地图块
    for (const auto& block : g_blocks) {
        ID3D11ShaderResourceView* texture = g_blockTexture;
        RenderImage(block.posX, block.posY, block.width, block.height, texture, 0, 1, 1);
    }

    // 绘制蓄力特效（如果有）
    if (g_player.isCharging) {
        float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;
        float effectSize = PLAYER_WIDTH * (1.0f + chargeRatio * 1.0f);
        float alpha = 0.3f + chargeRatio * 0.7f;

        RenderImage(g_player.posX - (effectSize - PLAYER_WIDTH) * 0.5f,
            g_player.posY - (effectSize - PLAYER_HEIGHT) * 0.5f,
            effectSize, effectSize, g_chargeEffectTexture, 0, alpha, 1);
    }

    // 绘制冲刺特效（如果有）
    if (g_player.isDashing) {
        // 在玩家位置绘制冲刺特效
        RenderImage(g_player.posX, g_player.posY, PLAYER_WIDTH * 1.5f, PLAYER_HEIGHT * 1.5f,
            g_dashEffectTexture, 0, 1, 1);
    }

    // 绘制玩家（根据状态选择不同颜色/纹理）
    ID3D11ShaderResourceView* playerTexture = g_playerTexture;

    // 可以根据玩家状态选择不同的帧或颜色
    int frameIndex = 0;
    if (g_player.isCharging) {
        frameIndex = 4; // 蓄力状态
    }
    else if (g_player.isDashing) {
        frameIndex = 3; // 冲刺状态
    }
    else if (!g_player.isOnGround) {
        frameIndex = 2; // 浮空状态
    }
    else if (g_player.isMoving) {
        frameIndex = 1; // 移动状态
    }

    RenderImage(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
        playerTexture, frameIndex, 1, 5); // 5帧：站立、移动、跳跃、冲刺、蓄力

    // 绘制UI信息
    // 这里可以添加显示当前冲刺类型的文本

    RendererDrawB();
}

void Game::Reset() {
    m_player.Reset();
    m_gameState = STATE_PLAYING;
}

void Game::GameLoop() {
    m_timer.Tick();
    float delta = m_timer.GetDeltaTime();

    m_inputSystem.HandleInput(m_player, m_dashSystem);
    Update();
    Draw();
}