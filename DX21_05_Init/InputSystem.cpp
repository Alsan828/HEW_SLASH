#include "InputSystem.h"
#include "Game.h"

InputSystem::InputSystem() {
    m_mousePos = { 0, 0 };
    // すべてのキー状態を false で初期化する
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
    // マップにないキーは直接問い合わせる
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

    // 上下移動（必要な場合）
    if (IsKeyDown(VK_UP) || IsKeyDown('W')) dirY += 1.0f;
    if (IsKeyDown(VK_DOWN) || IsKeyDown('S')) dirY -= 1.0f;

    // 方向ベクトルを正規化する
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }
}

void InputSystem::RebindKey(int action, int newKey) {
    // キー再割り当て機能はここで実装できる
    // 簡易実装のため、必要に応じて拡張可能
}

void InputSystem::Update() {
    // キーボード状態を更新する：前フレームの状態を保存してから現在の入力を読む
    m_previousKeyStates = m_currentKeyStates;
    for (auto& pair : m_currentKeyStates) {
        pair.second = (GetAsyncKeyState(pair.first) & 0x8000) != 0;
    }

    // マウス状態を更新する
    UpdateMouseState();
}

// InputSystem.cpp でマウス状態更新を実装する
void InputSystem::UpdateMouseState() {
    // 前フレームの状態を保存する
    m_prevMouseLeftDown = m_mouseLeftDown;

    // 現在のマウス状態を取得する
    m_mouseLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    m_mouseLeftPressed = m_mouseLeftDown && !m_prevMouseLeftDown;
    m_mouseLeftReleased = !m_mouseLeftDown && m_prevMouseLeftDown;

    m_mouseRightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

    // マウス位置を取得する
    GetCursorPos(&m_mousePos);
}

void InputSystem::GetMousePosition(float& worldX, float& worldY) const {
    // スクリーン座標をワールド座標へ変換する
    // 変換ロジックはレンダリングシステムに合わせて調整が必要
    extern HWND g_gameHwnd;
    HWND hwnd = g_gameHwnd ? g_gameHwnd : GetForegroundWindow();
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    POINT clientPos = m_mousePos;
    ScreenToClient(hwnd, &clientPos);

    // マウス座標を [-1, 1] 範囲へ正規化する
    float screenX = (2.0f * clientPos.x / clientRect.right) - 1.0f;
    float screenY = 1.0f - (2.0f * clientPos.y / clientRect.bottom);

    // カメラオフセットを考慮してワールド座標へ変換する
    worldX = screenX + g_camera.GetX();
    worldY = screenY + g_camera.GetY();
}

bool InputSystem::IsTogglePressed(int key) {
    if (IsKeyDown(key)) {
        if (!m_toggleConsumed[key]) {
            m_toggleConsumed[key] = true;
            return true; // 1 回だけ発火する
        }
    }
    else {
        m_toggleConsumed[key] = false; // 離したときにリセットする
    }
    return false;
}