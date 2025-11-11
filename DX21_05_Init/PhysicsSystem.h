#pragma once
#include "Player.h"
#include "Game.h"
#include <vector>

class PhysicsSystem {
private:
    std::vector<GameBlock> m_mapBlocks;

public:
    bool CheckCollision(float x1, float y1, float w1, float h1,
        float x2, float y2, float w2, float h2);
    void UpdatePlayerPhysics(Player& player, float deltaTime);
    void SetMapBlocks(const std::vector<GameBlock>& blocks);
    const std::vector<GameBlock>& GetMapBlocks() const;
    void CreateTestMap();
};