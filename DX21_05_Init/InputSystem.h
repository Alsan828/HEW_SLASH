#pragma once
#include <windows.h>
#include <map>
#include <Xinput.h> //XInputを使うためのヘッダーファイル
#pragma comment (lib, "xinput.lib") //XInputを使うために必要


// for controller
#define XINPUT_A              0x1000
#define XINPUT_B              0x2000
#define XINPUT_X              0x4000
#define XINPUT_Y              0x8000
#define XINPUT_UP             0x0001
#define XINPUT_DOWN           0x0002
#define XINPUT_LEFT           0x0004
#define XINPUT_RIGHT          0x0008
#define XINPUT_START          0x0010
#define XINPUT_BACK           0x0020
#define XINPUT_LEFT_THUMB     0x0040 //right stick
#define XINPUT_RIGHT_THUMB    0x0080 //left stick
#define XINPUT_LEFT_SHOULDER  0x0100 //L
#define XINPUT_RIGHT_SHOULDER 0x0200 //R

// for keyboard
#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x45
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A

// Input system class
class InputSystem {
private:
    //for keyboard
    BYTE keyState[256] = {};
    BYTE keyState_old[256] = {};

    //for ocntroller
    XINPUT_STATE controllerState = {};
    XINPUT_STATE controllerState_old = {};

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