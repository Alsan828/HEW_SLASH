#pragma once
#include <windows.h>
#include <map>

// Input system class
class InputSystem {
private:
    // Store key states
    std::map<int, bool> m_currentKeyStates;
    std::map<int, bool> m_previousKeyStates;

    // Key mappings
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

    // Update input state (call every frame)
    void Update();

    // Key state queries
    bool IsKeyDown(int key) const;
    bool IsKeyPressed(int key) const;  // Pressed this frame
    bool IsKeyReleased(int key) const; // Released this frame

    // Specific action queries
    bool IsMovingLeft() const;
    bool IsMovingRight() const;
    bool IsJumping() const;
    bool IsDashing() const;
    bool IsResetting() const;
    bool IsToggling() const;

    // Get movement direction vector
    void GetMoveDirection(float& dirX, float& dirY) const;

    // Modify key bindings (optional functionality)
    void RebindKey(int action, int newKey);
};