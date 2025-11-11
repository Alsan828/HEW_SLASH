#pragma once
#include <windows.h>
#include <map>

// 输入系统类
class InputSystem {
private:
    // 存储按键状态
    std::map<int, bool> m_currentKeyStates;
    std::map<int, bool> m_previousKeyStates;

    // 按键映射
    int m_moveLeftKey = VK_LEFT;
    int m_moveLeftAltKey = 'A';
    int m_moveRightKey = VK_RIGHT;
    int m_moveRightAltKey = 'D';
    int m_jumpKey = VK_UP;
    int m_jumpAltKey = 'W';
    int m_dashKey = VK_SPACE;
    int m_dashAltKey = VK_SHIFT;
    int m_resetKey = 'R';
    int m_toggleKey = 'T';

public:
    InputSystem();

    // 更新输入状态（每帧调用）
    void Update();

    // 按键状态查询
    bool IsKeyDown(int key) const;
    bool IsKeyPressed(int key) const;  // 当前帧按下
    bool IsKeyReleased(int key) const; // 当前帧释放

    // 特定动作的查询
    bool IsMovingLeft() const;
    bool IsMovingRight() const;
    bool IsJumping() const;
    bool IsDashing() const;
    bool IsResetting() const;
    bool IsToggling() const;

    // 获取移动方向向量
    void GetMoveDirection(float& dirX, float& dirY) const;

    // 修改按键绑定（可选功能）
    void RebindKey(int action, int newKey);
};