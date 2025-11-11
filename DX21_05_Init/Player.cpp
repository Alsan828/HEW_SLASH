#include "Player.h"

void Player::Reset() {
    posX = 0.0f;
    posY = -0.7f;
    velocityX = 0.0f;
    velocityY = 0.0f;
    isOnGround = false;
    isMoving = false;
    facingRight = true;
    isDashing = false;
    dashTimer = 0.0f;
    dashCooldown = 0.0f;
    dashDirectionX = 0.0f;
    dashDirectionY = 0.0f;
    isCharging = false;
    chargeTime = 0.0f;
}