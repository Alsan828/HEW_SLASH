#pragma once
#include "GameTimer.h"
#include "Player.h"
#include "PhysicsSystem.h"
#include "InputSystem.h"
#include "DashSystem.h"

struct GameBlock {
    float posX, posY;
    float width, height;
    bool isSolid;
};
// 纹理声明
extern ID3D11ShaderResourceView* g_playerTexture;   // 纹理
extern ID3D11ShaderResourceView* g_blockTexture;   // 方块纹理
extern ID3D11ShaderResourceView* g_backgroundTexture;  // 背景方块纹理
extern ID3D11ShaderResourceView* m_dashEffectTexture;  // 背景方块纹理
extern ID3D11ShaderResourceView* m_chargeEffectTexture;  // 背景方块纹理

extern Player g_player;
extern std::vector<GameBlock> g_blocks;          // 所有方块

class Game {
private:
    GameTimer m_timer;
    Player m_player;
    PhysicsSystem m_physicsSystem;
    InputSystem m_inputSystem;
    DashSystem m_dashSystem;
    GameState m_gameState = STATE_PLAYING;

    // 纹理资源
    ID3D11ShaderResourceView* m_playerTexture = nullptr;
    ID3D11ShaderResourceView* m_groundTexture = nullptr;
    ID3D11ShaderResourceView* m_backgroundTexture = nullptr;
    ID3D11ShaderResourceView* m_dashEffectTexture = nullptr;
    ID3D11ShaderResourceView* m_chargeEffectTexture = nullptr;

public:
    void Init();
    void Update();
    void Draw();
    void Reset();
    void GameLoop();
};