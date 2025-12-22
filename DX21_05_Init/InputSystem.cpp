#include "InputSystem.h"
#include "Game.h"

InputSystem::InputSystem() {
    m_mousePos = { 0, 0 };
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

void InputSystem::Update() {
    // 更新键盘状态（现有代码）
    //m_previousKeyStates = m_currentKeyStates;
    for (auto& pair : m_currentKeyStates) {
        pair.second = (GetAsyncKeyState(pair.first) & 0x8000) != 0;
    }

    m_previousKeyStates = m_currentKeyStates;

    // 更新鼠标状态
    UpdateMouseState();
}

// 在 InputSystem.cpp 中实现鼠标状态更新
void InputSystem::UpdateMouseState() {
    // 保存上一帧状态
    m_prevMouseLeftDown = m_mouseLeftDown;

    // 获取当前鼠标状态
    m_mouseLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    m_mouseLeftPressed = m_mouseLeftDown && !m_prevMouseLeftDown;
    m_mouseLeftReleased = !m_mouseLeftDown && m_prevMouseLeftDown;

    // 获取鼠标位置
    GetCursorPos(&m_mousePos);
}

void InputSystem::GetMousePosition(float& worldX, float& worldY) const {
    // 将屏幕坐标转换为世界坐标
    // 这里需要根据你的渲染系统调整转换逻辑
    HWND hwnd = GetForegroundWindow();
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    // 将鼠标坐标归一化到 [-1, 1] 范围
    float screenX = (2.0f * m_mousePos.x / clientRect.right) - 1.0f;
    float screenY = 1.0f - (2.0f * m_mousePos.y / clientRect.bottom);

    // 转换为世界坐标（考虑相机偏移）
    worldX = screenX + g_camera.GetX();
    worldY = screenY + g_camera.GetY();
}

bool InputSystem::IsTogglePressed(int key) {
    if (IsKeyDown(key)) {
        if (!m_toggleConsumed[key]) {
            m_toggleConsumed[key] = true;
            return true; // fire once
        }
    }
    else {
        m_toggleConsumed[key] = false; // reset when released
    }
    return false;
}