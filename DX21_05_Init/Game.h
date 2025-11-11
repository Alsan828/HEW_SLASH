#pragma once

#include <cstdint>
#include "InputSystem.h"

// 移除原来的 IsKeyDown 函数声明
// 添加输入系统实例声明
extern InputSystem g_inputSystem;

class GameTimer {
private:
    __int64 m_prevTime = 0;
    __int64 m_currTime = 0;
    double m_secondsPerCount = 0.0;
    float m_deltaTime = 0.0f;

public:
    GameTimer();
    void Tick();
    float GetDeltaTime() const;
};

// 游戏初始化
void InitGameWorld();

// 游戏循环
void GameLoop();

// 绘制函数
void Draw();

// 输入处理
void HandleInput();

// 重置游戏
void ResetGame();