#pragma once
#include "Player.h"
#include "GameDefines.h"

class DashSystem {
private:
    DashType m_currentDashType = DASH_INSTANT;

public:
    void UpdateDash(Player& player, float deltaTime);
    void DashInstant(Player& player);
    void StartChargeDash(Player& player);
    void ExecuteChargeDash(Player& player);
    void CancelChargeDash(Player& player);
    void ToggleDashType();
    DashType GetCurrentDashType() const;

private:
    bool IsKeyDown(int key);
};