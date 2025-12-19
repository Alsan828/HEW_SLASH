#pragma once
#define NOMINMAX

#include <d3d11.h>

// 全局常量定义
const float GRID_WIDTH = 0.0625f;
const float GRID_HEIGHT = 0.085f;
const float PLAYER_WIDTH = 0.08f;
const float PLAYER_HEIGHT = 0.12f;
const float GRAVITY = -0.004f;
const float JUMP_FORCE = 0.075f;
const float MOVE_SPEED = 0.01f;
const float DASH_SPEED = 0.15f;
const float DASH_DURATION = 0.05f;
const float DASH_COOLDOWN = 0.2f;

// 枚举定义
enum DashType {
    DASH_INSTANT,
    DASH_CHARGE
};

enum GameState {
    STATE_PLAYING,
    STATE_GAME_OVER
};