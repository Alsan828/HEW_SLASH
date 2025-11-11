#pragma once
#include "GameDefines.h"

class Player {
public:
    float posX = 0.0f;
    float posY = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    bool isOnGround = false;
    bool isMoving = false;
    bool facingRight = true;
    // 冲刺相关
    bool isDashing = false;
    float dashTimer = 0.0f;
    float dashCooldown = 0.0f;
    float dashDirectionX = 0.0f;
    float dashDirectionY = 0.0f;

    // 蓄力冲刺
    bool isCharging = false;
    float chargeTime = 0.0f;
    const float MAX_CHARGE_TIME = 1.5f;
    const float MIN_CHARGE_TIME = 0.2f;

    // 方法声明
    void Reset();
};