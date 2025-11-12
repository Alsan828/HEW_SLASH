#include "InputSystem.h"

InputSystem::InputSystem() {
    // Initialize all key states to false
    int keys[] = {
        VK_LEFT, 'A', VK_RIGHT, 'D', VK_UP, 'W',
        VK_DOWN, 'S', VK_SPACE, VK_SHIFT, 'R', 'T'
    };

    for (int key : keys) {
        m_currentKeyStates[key] = false;
        m_previousKeyStates[key] = false;
    }
}

void InputSystem::Update() {
    // Save previous frame's states
    m_previousKeyStates = m_currentKeyStates;

    // Update current frame's states
    for (auto& pair : m_currentKeyStates) {
        pair.second = (GetAsyncKeyState(pair.first) & 0x8000) != 0;
    }
}

bool InputSystem::IsKeyDown(int key) const {
    auto it = m_currentKeyStates.find(key);
    if (it != m_currentKeyStates.end()) {
        return it->second;
    }
    // If key is not in the map, query directly
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

bool InputSystem::IsKeyPressed(int key) const {
    return IsKeyDown(key) && !(m_previousKeyStates.count(key) ? m_previousKeyStates.at(key) : false);
}

bool InputSystem::IsKeyReleased(int key) const {
    return !IsKeyDown(key) && (m_previousKeyStates.count(key) ? m_previousKeyStates.at(key) : false);
}

bool InputSystem::IsMovingLeft() const {
    return IsKeyDown(m_moveLeftKey) || IsKeyDown(m_moveLeftAltKey);
}

bool InputSystem::IsMovingRight() const {
    return IsKeyDown(m_moveRightKey) || IsKeyDown(m_moveRightAltKey);
}

bool InputSystem::IsJumping() const {
    return IsKeyDown(m_jumpKey) || IsKeyDown(m_jumpAltKey);
}

bool InputSystem::IsDashing() const {
    return IsKeyDown(m_dashKey) || IsKeyDown(m_dashAltKey);
}

bool InputSystem::IsResetting() const {
    return IsKeyDown(m_resetKey);
}

bool InputSystem::IsToggling() const {
    return IsKeyDown(m_toggleKey);
}

void InputSystem::GetMoveDirection(float& dirX, float& dirY) const {
    dirX = 0.0f;
    dirY = 0.0f;

    if (IsMovingLeft()) dirX -= 1.0f;
    if (IsMovingRight()) dirX += 1.0f;

    // Up/down movement (if needed)
    if (IsKeyDown(VK_UP) || IsKeyDown('W')) dirY += 1.0f;
    if (IsKeyDown(VK_DOWN) || IsKeyDown('S')) dirY -= 1.0f;

    // Normalize direction vector
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }
}

void InputSystem::RebindKey(int action, int newKey) {
    // Key rebinding functionality can be implemented here
    // Simplified implementation, can be expanded as needed
}