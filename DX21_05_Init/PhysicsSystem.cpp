#include "PhysicsSystem.h"
#include <algorithm>
#include <cmath>

bool PhysicsSystem::CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
        y1 < y2 + h2 && y1 + h1 > y2);
}

void PhysicsSystem::UpdatePlayerPhysics(Player& player, float deltaTime) {
    // 应用重力（如果不是在冲刺状态）
    if (!player.isDashing) {
        float fixedDeltaTime = std::min(deltaTime, 0.033f);
        player.velocityY += GRAVITY * fixedDeltaTime * 60.0f;

        // 限制最大下落速度
        if (player.velocityY < -0.3f) {
            player.velocityY = -0.3f; 
        }
    }

    // 先保存当前位置用于碰撞检测
    float originalX = player.posX;
    float originalY = player.posY;

    // 先应用水平移动
    player.posX += player.velocityX * deltaTime * 60.0f;

    // 水平碰撞检测
    for (const auto& block : m_mapBlocks) {
        if (!block.isSolid) continue;

        if (CheckCollision(player.posX, player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            block.posX, block.posY, block.width, block.height)) {
            player.velocityX = 0.0f;
        }
    }

    // 再应用垂直移动
    player.posY += player.velocityY * deltaTime * 60.0f;

    player.isOnGround = false;

    for (const auto& block : m_mapBlocks) {
        
        if (!block.isSolid) continue;

        if (CheckCollision(player.posX, player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            block.posX, block.posY, block.width, block.height)) {

            // 计算玩家和块的中心点
            float playerCenterX = player.posX + PLAYER_WIDTH / 2;
            float playerCenterY = player.posY + PLAYER_HEIGHT / 2;
            float blockCenterX = block.posX + block.width / 2;
            float blockCenterY = block.posY + block.height / 2;

            // 计算重叠量
            float overlapX = (PLAYER_WIDTH / 2 + block.width / 2) - fabs(playerCenterX - blockCenterX);
            float overlapY = (PLAYER_HEIGHT / 2 + block.height / 2) - fabs(playerCenterY - blockCenterY);

            // 根据重叠量最小的方向解决碰撞
            if (overlapX < overlapY) {
                // X轴碰撞
                if (playerCenterX < blockCenterX) {
                    player.posX = block.posX - PLAYER_WIDTH;
                }
                else {
                    player.posX = block.posX + block.width;
                }
                player.velocityX = 0.0f;
            }
            else {
                // Y轴碰撞
                if (playerCenterY < blockCenterY) {
                    player.posY = block.posY - PLAYER_HEIGHT;
                    player.velocityY = 0.0f;
                }
                else {
                    // 从上方碰撞（落地）
                    player.posY = block.posY + block.height;
                    player.velocityY = 0.0f;
                    player.isOnGround = true;
                }
            }
        }
    }

    // 边界检查 - 掉落检测
    if (player.posY < -2.0f) {
        player.Reset();
    }
}

void PhysicsSystem::SetMapBlocks(const std::vector<GameBlock>& blocks) {
    m_mapBlocks = blocks;
}

const std::vector<GameBlock>& PhysicsSystem::GetMapBlocks() const {
    return m_mapBlocks;
}

void PhysicsSystem::CreateTestMap() {
    m_mapBlocks.clear();

    // 创建连续的地面
    for (int i = 0; i < 20; i++) {
        GameBlock ground;
        ground.posX = -1.0f + i * GRID_WIDTH * 8;
        ground.posY = -0.9f;
        ground.width = GRID_WIDTH * 8;
        ground.height = GRID_HEIGHT;
        ground.isSolid = true;
        m_mapBlocks.push_back(ground);
    }

    // 调整平台位置，给玩家更多跳跃空间
    GameBlock platform1;
    platform1.posX = -0.5f;
    platform1.posY = -0.6f;
    platform1.width = 0.3f;
    platform1.height = 0.05f;
    platform1.isSolid = true;
    m_mapBlocks.push_back(platform1);

    GameBlock platform2;
    platform2.posX = 0.2f;
    platform2.posY = -0.4f;
    platform2.width = 0.3f;
    platform2.height = 0.05f;
    platform2.isSolid = true;
    m_mapBlocks.push_back(platform2);

    GameBlock platform3;
    platform3.posX = -0.2f;
    platform3.posY = -0.2f;
    platform3.width = 0.3f;
    platform3.height = 0.05f;
    platform3.isSolid = true;
    m_mapBlocks.push_back(platform3);

    // 添加一些墙和障碍物来测试碰撞
    GameBlock leftWall;
    leftWall.posX = -1.0f;
    leftWall.posY = -0.9f;
    leftWall.width = 0.05f;
    leftWall.height = 1.8f;
    leftWall.isSolid = true;
    m_mapBlocks.push_back(leftWall);

    GameBlock rightWall;
    rightWall.posX = 0.95f;
    rightWall.posY = -0.9f;
    rightWall.width = 0.05f;
    rightWall.height = 1.8f;
    rightWall.isSolid = true;
    m_mapBlocks.push_back(rightWall);
}