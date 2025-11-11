#define NOMINMAX
#include "Game.h"
#include "Render.h"
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

#pragma once

// 全局常量定义
const float GRID_WIDTH = 0.0625f;
const float GRID_HEIGHT = 0.085f;
const float PLAYER_WIDTH = 0.08f;
const float PLAYER_HEIGHT = 0.12f;
const float GRAVITY = -0.004f;
const float JUMP_FORCE = 0.075f;
const float MOVE_SPEED = 0.01f;
const float DASH_SPEED = 0.15f;  // 冲刺基础速度
const float DASH_DURATION = 0.05f; // 冲刺基础持续时间
const float DASH_COOLDOWN = 0.2f; // 冲刺冷却时间

// 冲刺类型枚举
enum DashType {
    DASH_INSTANT,    // 按下直接冲刺
    DASH_CHARGE      // 长按蓄力冲刺
};

DashType g_currentDashType = DASH_INSTANT; // 当前使用的冲刺类型

GameTimer g_gameTimer;

// 游戏状态枚举
enum GameState {
    STATE_PLAYING,
    STATE_GAME_OVER
};

GameState g_gameState = STATE_PLAYING;

// 玩家结构
struct Player {
    float posX = 0.0f;
    float posY = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    bool isOnGround = false;
    bool isMoving = false;
    bool facingRight = true; // 朝向

    // 冲刺相关变量
    bool isDashing = false;
    float dashTimer = 0.0f;
    float dashCooldown = 0.0f;
    float dashDirectionX = 0.0f;
    float dashDirectionY = 0.0f;

    // 蓄力冲刺专用变量
    bool isCharging = false;
    float chargeTime = 0.0f;
    const float MAX_CHARGE_TIME = 1.5f; // 最大蓄力时间
    const float MIN_CHARGE_TIME = 0.01f; // 最小有效蓄力时间
};

Player g_player;

// 地图块结构
struct MapBlock {
    float posX, posY;
    float width, height;
    bool isSolid;
};

std::vector<MapBlock> g_mapBlocks;
ID3D11ShaderResourceView* g_playerTexture = nullptr;
ID3D11ShaderResourceView* g_groundTexture = nullptr;
ID3D11ShaderResourceView* g_backgroundTexture = nullptr;
ID3D11ShaderResourceView* g_dashEffectTexture = nullptr;
ID3D11ShaderResourceView* g_chargeEffectTexture = nullptr; // 蓄力特效纹理

// 游戏计时器实现
GameTimer::GameTimer() {
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
}

float GameTimer::GetDeltaTime() const {
    return m_deltaTime;
}

void ResetGame() {
    // 重置玩家状态 - 将玩家放在地面上
    g_player.posX = 0.0f;
    g_player.posY = -0.7f;  // 放在地面上方
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;
    g_player.isOnGround = false;
    g_player.isMoving = false;
    g_player.facingRight = true;

    // 重置冲刺状态
    g_player.isDashing = false;
    g_player.dashTimer = 0.0f;
    g_player.dashCooldown = 0.0f;
    g_player.dashDirectionX = 0.0f;
    g_player.dashDirectionY = 0.0f;

    // 重置蓄力状态
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;

    g_gameState = STATE_PLAYING;
}

// 改进的碰撞检测函数
bool CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
        y1 < y2 + h2 && y1 + h1 > y2);
}

// 创建简单的地图
void CreateTestMap() {
    g_mapBlocks.clear();

    // 创建连续的地面
    for (int i = 0; i < 20; i++) {
        MapBlock ground;
        ground.posX = -1.0f + i * GRID_WIDTH * 8; // 确保无缝隙
        ground.posY = -0.9f;
        ground.width = GRID_WIDTH * 8;
        ground.height = GRID_HEIGHT;
        ground.isSolid = true;
        g_mapBlocks.push_back(ground);
    }

    // 调整平台位置，给玩家更多跳跃空间
    MapBlock platform1;
    platform1.posX = -0.5f;
    platform1.posY = -0.6f;  // 提高平台位置
    platform1.width = 0.3f;
    platform1.height = 0.05f;
    platform1.isSolid = true;
    g_mapBlocks.push_back(platform1);

    MapBlock platform2;
    platform2.posX = 0.2f;
    platform2.posY = -0.4f;  // 提高平台位置
    platform2.width = 0.3f;
    platform2.height = 0.05f;
    platform2.isSolid = true;
    g_mapBlocks.push_back(platform2);

    MapBlock platform3;
    platform3.posX = -0.2f;
    platform3.posY = -0.2f;  // 提高平台位置
    platform3.width = 0.3f;
    platform3.height = 0.05f;
    platform3.isSolid = true;
    g_mapBlocks.push_back(platform3);

    // 添加一些墙和障碍物来测试碰撞
    MapBlock leftWall;
    leftWall.posX = -1.0f;
    leftWall.posY = -0.9f;
    leftWall.width = 0.05f;
    leftWall.height = 1.8f;  // 从地面到顶部
    leftWall.isSolid = true;
    g_mapBlocks.push_back(leftWall);

    MapBlock rightWall;
    rightWall.posX = 0.95f;  // 屏幕右侧
    rightWall.posY = -0.9f;
    rightWall.width = 0.05f;
    rightWall.height = 1.8f;
    rightWall.isSolid = true;
    g_mapBlocks.push_back(rightWall);
}

// 游戏初始化
void InitGameWorld() {
    // 加载纹理（暂时用现有纹理替代）
    LoadTexture(g_pDevice, "asset/block.png", &g_playerTexture);      // 玩家纹理
    LoadTexture(g_pDevice, "asset/blockB.png", &g_groundTexture);     // 地面纹理
    LoadTexture(g_pDevice, "asset/Space.png", &g_backgroundTexture);  // 背景纹理
    LoadTexture(g_pDevice, "asset/block.png", &g_dashEffectTexture);   // 冲刺特效纹理
    LoadTexture(g_pDevice, "asset/blockB.png", &g_chargeEffectTexture); // 蓄力特效纹理

    CreateTestMap();
    ResetGame();
}

void UpdatePlayerPhysics(float deltaTime) {
    // 应用重力（如果不是在冲刺状态）
    if (!g_player.isDashing) {
        float fixedDeltaTime = std::min(deltaTime, 0.033f);
        g_player.velocityY += GRAVITY * fixedDeltaTime * 60.0f;

        // 限制最大下落速度
        if (g_player.velocityY < -0.3f) {
            g_player.velocityY = -0.3f;
        }
    }

    // 先保存当前位置用于碰撞检测
    float originalX = g_player.posX;
    float originalY = g_player.posY;

    // 先应用水平移动
    g_player.posX += g_player.velocityX * deltaTime * 60.0f;

    // 水平碰撞检测
    for (const auto& block : g_mapBlocks) {
        if (!block.isSolid) continue;

        if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            block.posX, block.posY, block.width, block.height)) {
            g_player.velocityX = 0.0f;
        }
    }

    // 再应用垂直移动
    g_player.posY += g_player.velocityY * deltaTime * 60.0f;

    g_player.isOnGround = false;

    for (const auto& block : g_mapBlocks) {
        if (!block.isSolid) continue;

        if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            block.posX, block.posY, block.width, block.height)) {

            // 计算玩家和块的中心点
            float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
            float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;
            float blockCenterX = block.posX + block.width / 2;
            float blockCenterY = block.posY + block.height / 2;

            // 计算重叠量
            float overlapX = (PLAYER_WIDTH / 2 + block.width / 2) - fabs(playerCenterX - blockCenterX);
            float overlapY = (PLAYER_HEIGHT / 2 + block.height / 2) - fabs(playerCenterY - blockCenterY);

            // 根据重叠量最小的方向解决碰撞
            if (overlapX < overlapY) {
                // X轴碰撞
                if (playerCenterX < blockCenterX) {
                    g_player.posX = block.posX - PLAYER_WIDTH;
                }
                else {
                    g_player.posX = block.posX + block.width;
                }
                g_player.velocityX = 0.0f;
            }
            else {
                // Y轴碰撞
                if (playerCenterY < blockCenterY) {
                    g_player.posY = block.posY - PLAYER_HEIGHT;
                    g_player.velocityY = 0.0f;
                }
                else {
                    // 从上方碰撞（落地）
                    g_player.posY = block.posY + block.height;
                    g_player.velocityY = 0.0f;
                    g_player.isOnGround = true;
                }
            }
        }
    }

    // 边界检查 - 掉落检测
    if (g_player.posY < -2.0f) {
        ResetGame();
    }

}
// 方式1：按下直接冲刺
void Dash1() {
    if (g_player.dashCooldown > 0.0f || g_player.isDashing) {
        return;
    }

    // 使用输入系统获取方向
    float dirX = 0.0f, dirY = 0.0f;
    g_inputSystem.GetMoveDirection(dirX, dirY);

    // 如果没有方向输入，使用玩家朝向
    if (dirX == 0.0f && dirY == 0.0f) {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
    }

    // 设置冲刺状态
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION;
    g_player.dashCooldown = DASH_COOLDOWN;
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // 设置冲刺速度
    g_player.velocityX = dirX * DASH_SPEED;
    g_player.velocityY = dirY * DASH_SPEED;
}

// 方式2：长按蓄力冲刺
void StartChargeDash() {
    if (g_player.dashCooldown > 0.0f || g_player.isDashing || g_player.isCharging) {
        return;
    }

    g_player.isCharging = true;
    g_player.chargeTime = 0.0f;
}

void ExecuteChargeDash() {
    if (!g_player.isCharging || g_player.chargeTime < g_player.MIN_CHARGE_TIME) {
        return;
    }

    // 使用输入系统获取方向
    float dirX = 0.0f, dirY = 0.0f;
    g_inputSystem.GetMoveDirection(dirX, dirY);

    // 如果没有方向输入，使用玩家朝向
    if (dirX == 0.0f && dirY == 0.0f) {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
    }

    // 标准化方向向量
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }

    // 根据蓄力时间计算冲刺参数
    float chargeRatio = g_player.chargeTime / g_player.MAX_CHARGE_TIME;
    chargeRatio = std::min(chargeRatio, 1.0f); // 限制在1.0以内

    // 速度和持续时间随蓄力时间增加（非线性，前期增长快，后期增长慢）
    float speedMultiplier = 1.0f + chargeRatio * 2.0f; // 1.0x 到 3.0x
    float durationMultiplier = 1.0f + chargeRatio * 1.5f; // 1.0x 到 2.5x

    // 设置冲刺状态
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION * durationMultiplier;
    g_player.dashCooldown = DASH_COOLDOWN * (0.5f + chargeRatio * 0.5f); // 冷却时间也随蓄力增加
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // 设置冲刺速度
    g_player.velocityX = dirX * DASH_SPEED * speedMultiplier;
    g_player.velocityY = dirY * DASH_SPEED * speedMultiplier;

    // 结束蓄力状态
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
}

void CancelChargeDash() {
    if (g_player.isCharging) {
        g_player.isCharging = false;
        g_player.chargeTime = 0.0f;
    }
}

// 更新冲刺状态
void UpdateDash(float deltaTime) {
    // 更新蓄力状态
    if (g_player.isCharging) {
        g_player.chargeTime += deltaTime;
        if (g_player.chargeTime >= g_player.MAX_CHARGE_TIME) {
            // 达到最大蓄力时间，自动触发冲刺
            ExecuteChargeDash();
        }
    }

    if (g_player.dashCooldown > 0.0f) {
        g_player.dashCooldown -= deltaTime;
    }

    if (g_player.isDashing) {
        g_player.dashTimer -= deltaTime;

        if (g_player.dashTimer <= 0.0f) {
            // 冲刺结束
            g_player.isDashing = false;

            // 如果不是在地面上，保持Y轴速度，否则停止Y轴移动
            if (!g_player.isOnGround) {
                g_player.velocityY *= 0.5f; // 保留一部分速度
            }
            else {
                g_player.velocityY = 0.0f;
            }

            // 停止水平移动，除非有输入
            if (!g_player.isMoving) {
                g_player.velocityX *= 0.3f; // 保留一些惯性
            }
        }
    }
}

// 切换冲刺类型（用于测试）
void ToggleDashType() {
    if (g_currentDashType == DASH_INSTANT) {
        g_currentDashType = DASH_CHARGE;
    }
    else {
        g_currentDashType = DASH_INSTANT;
    }

    // 取消当前的蓄力状态
    if (g_player.isCharging) {
        CancelChargeDash();
    }
}

// 玩家移动控制
void MovePlayerLeft() {
    g_player.velocityX = -MOVE_SPEED;
    g_player.isMoving = true;
    g_player.facingRight = false;
}

void MovePlayerRight() {
    g_player.velocityX = MOVE_SPEED;
    g_player.isMoving = true;
    g_player.facingRight = true;
}

void StopPlayer() {
    if (!g_player.isDashing) {
        g_player.velocityX = 0.0f;
    }
    g_player.isMoving = false;
}

// 改进的跳跃函数
void Jump() {
    if (g_player.isOnGround && !g_player.isDashing && !g_player.isCharging) {
        g_player.velocityY = JUMP_FORCE;
        g_player.isOnGround = false;
    }
}

// 修改后的游戏更新函数
void UpdateGame(float deltaTime) {
    if (g_gameState != STATE_PLAYING) {
        return;
    }

    // 更新冲刺状态
    UpdateDash(deltaTime);

    // 更新物理
    UpdatePlayerPhysics(deltaTime);
}

// 修改后的渲染函数
void Draw() {
    RendererDrawF();

    // 绘制背景
    RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, g_backgroundTexture, 0, 1, 1);

    // 绘制地图块
    for (const auto& block : g_mapBlocks) {
        ID3D11ShaderResourceView* texture = g_groundTexture;
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

// 键盘检测
bool IsKeyDown(int key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}
void HandleInput() {
    // 更新输入系统
    g_inputSystem.Update();

    if (g_inputSystem.IsResetting()) {
        ResetGame();
    }

    // 切换冲刺类型
    static bool wasToggleKeyPressed = false;
    bool isToggleKeyPressed = g_inputSystem.IsToggling();
    if (isToggleKeyPressed && !wasToggleKeyPressed) {
        ToggleDashType();
    }
    wasToggleKeyPressed = isToggleKeyPressed;

    // 水平移动
    bool moving = false;
    if (g_inputSystem.IsMovingLeft()) {
        if (!g_player.isDashing && !g_player.isCharging) {
            g_player.velocityX = -MOVE_SPEED;
        }
        g_player.isMoving = true;
        g_player.facingRight = false;
        moving = true;
    }
    if (g_inputSystem.IsMovingRight()) {
        if (!g_player.isDashing && !g_player.isCharging) {
            g_player.velocityX = MOVE_SPEED;
        }
        g_player.isMoving = true;
        g_player.facingRight = true;
        moving = true;
    }

    if (!moving && !g_player.isDashing && !g_player.isCharging) {
        g_player.velocityX = 0.0f;
        g_player.isMoving = false;
    }

    // 跳跃输入
    static bool wasJumpKeyPressed = false;
    bool isJumpKeyPressed = g_inputSystem.IsJumping();

    if (isJumpKeyPressed && !wasJumpKeyPressed) {
        Jump();
    }
    wasJumpKeyPressed = isJumpKeyPressed;

    // 冲刺输入处理
    static bool wasDashKeyPressed = false;
    bool isDashKeyPressed = g_inputSystem.IsDashing();

    if (g_currentDashType == DASH_INSTANT) {
        // 方式1：按下直接冲刺
        if (isDashKeyPressed && !wasDashKeyPressed) {
            Dash1();
        }
    }
    else {
        // 方式2：长按蓄力冲刺
        if (isDashKeyPressed && !wasDashKeyPressed) {
            // 按下开始蓄力
            StartChargeDash();
        }
        else if (!isDashKeyPressed && wasDashKeyPressed) {
            // 释放触发冲刺
            ExecuteChargeDash();
        }
        else if (!isDashKeyPressed && g_player.isCharging) {
            // 防止异常情况
            CancelChargeDash();
        }
    }

    wasDashKeyPressed = isDashKeyPressed;
}

// 修改后的游戏循环
void GameLoop() {
    g_gameTimer.Tick();
    float delta = g_gameTimer.GetDeltaTime();

    HandleInput();  // 处理键盘输入
    // 更新游戏状态
    UpdateGame(delta);
    Draw();
}