#include "Game.h"
#include "Enemy.h"

// Update player physics
void UpdatePlayerPhysics(float deltaTime) {
    // Skip physics update if player is dead
    if (g_player.isDead) {
        return;
    }

    // Ignore gravity and movement during stun state
    if (g_player.isInDashAftermath) {
        // Only handle basic collision to prevent falling through the ground
        auto& solidTiles = g_mapManager.GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                tile.posX, tile.posY, tile.width, tile.height)) {
                // Simple vertical collision handling
                g_player.posY = tile.posY + tile.height; // Stand on ground
                g_player.isOnGround = true;
            }
        }
        return; // Skip normal physics update during stun state
    }

    // Apply gravity
    if (!g_player.isDashing) {
        float fixedDeltaTime = std::min(deltaTime, 0.033f);
        g_player.velocityY += GRAVITY * fixedDeltaTime * 60.0f;
        if (g_player.velocityY < -0.2f) {
            g_player.velocityY = -0.2f;
        }
    }

    // Save original position for collision detection
    float oldX = g_player.posX;
    float oldY = g_player.posY;

    // Calculate movement distance
    float moveX = g_player.velocityX * deltaTime * 60.0f;
    float moveY = g_player.velocityY * deltaTime * 60.0f;

    // Get current map's spatial grid
    SpatialGrid* spatialGrid = g_mapManager.GetCurrentMap()->GetSpatialGrid();
    if (!spatialGrid) {
        // Fallback to original method if spatial grid not built
        auto& solidTiles = g_mapManager.GetCurrentMap()->GetSolidTiles();

        // === FIX: Use continuous collision detection when speed exceeds threshold, regardless of dashing ===
        float speedSquared = g_player.velocityX * g_player.velocityX + g_player.velocityY * g_player.velocityY;
        float speedThreshold = 0.5f; // Speed threshold for continuous collision detection

        if (g_player.isDashing || speedSquared > speedThreshold * speedThreshold) {
            int steps = 4; // Divide movement into 4 steps for detection
            float stepX = moveX / steps;
            float stepY = moveY / steps;

            for (int i = 0; i < steps; i++) {
                g_player.posX += stepX;

                // Horizontal collision detection
                for (const auto& tile : solidTiles) {
                    if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                        tile.posX, tile.posY, tile.width, tile.height)) {
                        // Revert to pre-collision position
                        g_player.posX -= stepX;
                        g_player.velocityX = 0.0f;

                        // Calculate collision normal and bounce
                        if (moveX > 0) {
                            // Collision when moving right
                            g_player.posX = tile.posX - PLAYER_WIDTH;
                        }
                        else if (moveX < 0) {
                            // Collision when moving left
                            g_player.posX = tile.posX + tile.width;
                        }
                        break;
                    }
                }

                g_player.posY += stepY;

                // Vertical collision detection
                for (const auto& tile : solidTiles) {
                    if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                        tile.posX, tile.posY, tile.width, tile.height)) {
                        // Revert to pre-collision position
                        g_player.posY -= stepY;

                        if (moveY > 0) {
                            // Collision when moving upward
                            g_player.posY = tile.posY - PLAYER_HEIGHT;
                            g_player.velocityY = 0.0f;
                        }
                        else if (moveY < 0) {
                            // Collision when moving downward
                            g_player.posY = tile.posY + tile.height;
                            g_player.velocityY = 0.0f;
                            g_player.isOnGround = true;
                        }
                        break;
                    }
                }
            }
        }
        else {
            // Normal movement uses separate axis collision handling
            g_player.posX += moveX;
            g_player.posY += moveY;

            // Reset ground state
            g_player.isOnGround = false;

            for (const auto& tile : solidTiles) {
                if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                    tile.posX, tile.posY, tile.width, tile.height)) {

                    float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
                    float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;
                    float tileCenterX = tile.posX + tile.width / 2;
                    float tileCenterY = tile.posY + tile.height / 2;

                    float overlapX = (PLAYER_WIDTH / 2 + tile.width / 2) - fabs(playerCenterX - tileCenterX);
                    float overlapY = (PLAYER_HEIGHT / 2 + tile.height / 2) - fabs(playerCenterY - tileCenterY);

                    // Separate axis handling: choose direction of minimum overlap
                    if (overlapX < overlapY) {
                        // Horizontal collision
                        if (playerCenterX < tileCenterX) {
                            g_player.posX = tile.posX - PLAYER_WIDTH;
                        }
                        else {
                            g_player.posX = tile.posX + tile.width;
                        }
                        g_player.velocityX = 0.0f;
                    }
                    else {
                        // Vertical collision
                        if (playerCenterY < tileCenterY) {
                            g_player.posY = tile.posY - PLAYER_HEIGHT;
                            g_player.velocityY = 0.0f;
                        }
                        else {
                            g_player.posY = tile.posY + tile.height;
                            g_player.velocityY = 0.0f;
                            g_player.isOnGround = true;
                        }
                    }
                }
            }
        }
    }
    else {
        // Use spatial grid optimized collision detection
        std::vector<MapTile*> nearbyTiles;

        // Get tiles around player
        float padding = 1.0f;  // Extend range slightly
        spatialGrid->GetTilesInArea(
            g_player.posX - padding,
            g_player.posY - padding,
            PLAYER_WIDTH + padding * 2,
            PLAYER_HEIGHT + padding * 2,
            nearbyTiles
        );

        // === FIX: Use continuous collision detection when speed exceeds threshold, regardless of dashing ===
        float speedSquared = g_player.velocityX * g_player.velocityX + g_player.velocityY * g_player.velocityY;
        float speedThreshold = 0.1f; // Speed threshold for continuous collision detection

        if (g_player.isDashing || speedSquared > speedThreshold * speedThreshold) {
            int steps = 4;
            float stepX = moveX / steps;
            float stepY = moveY / steps;

            for (int i = 0; i < steps; i++) {
                g_player.posX += stepX;

                // Horizontal collision detection
                for (const auto& tile : nearbyTiles) {
                    if (tile->tileInfo.isSolid &&
                        CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                            tile->posX, tile->posY, tile->width, tile->height)) {
                        // Revert to pre-collision position
                        g_player.posX -= stepX;
                        g_player.velocityX = 0.0f;

                        // Calculate collision normal
                        if (moveX > 0) {
                            // Collision when moving right
                            g_player.posX = tile->posX - PLAYER_WIDTH;
                        }
                        else if (moveX < 0) {
                            // Collision when moving left
                            g_player.posX = tile->posX + tile->width;
                        }
                        break;
                    }
                }

                g_player.posY += stepY;

                // Vertical collision detection
                for (const auto& tile : nearbyTiles) {
                    if (tile->tileInfo.isSolid &&
                        CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                            tile->posX, tile->posY, tile->width, tile->height)) {
                        // Revert to pre-collision position
                        g_player.posY -= stepY;

                        if (moveY > 0) {
                            // Collision when moving upward
                            g_player.posY = tile->posY - PLAYER_HEIGHT;
                            g_player.velocityY = 0.0f;
                        }
                        else if (moveY < 0) {
                            // Collision when moving downward
                            g_player.posY = tile->posY + tile->height;
                            g_player.velocityY = 0.0f;
                            g_player.isOnGround = true;
                        }
                        break;
                    }
                }
            }
        }
        else {
            // Normal movement uses separate axis collision handling
            g_player.posX += moveX;
            g_player.posY += moveY;

            g_player.isOnGround = false;

            for (const auto& tile : nearbyTiles) {
                if (tile->tileInfo.isSolid &&
                    CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                        tile->posX, tile->posY, tile->width, tile->height)) {

                    float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
                    float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;
                    float tileCenterX = tile->posX + tile->width / 2;
                    float tileCenterY = tile->posY + tile->height / 2;

                    float overlapX = (PLAYER_WIDTH / 2 + tile->width / 2) - fabs(playerCenterX - tileCenterX);
                    float overlapY = (PLAYER_HEIGHT / 2 + tile->height / 2) - fabs(playerCenterY - tileCenterY);

                    // Separate axis handling: choose direction of minimum overlap
                    if (overlapX < overlapY) {
                        // Horizontal collision
                        if (playerCenterX < tileCenterX) {
                            g_player.posX = tile->posX - PLAYER_WIDTH;
                        }
                        else {
                            g_player.posX = tile->posX + tile->width;
                        }
                        g_player.velocityX = 0.0f;
                    }
                    else {
                        // Vertical collision
                        if (playerCenterY < tileCenterY) {
                            g_player.posY = tile->posY - PLAYER_HEIGHT;
                            g_player.velocityY = 0.0f;
                        }
                        else {
                            g_player.posY = tile->posY + tile->height;
                            g_player.velocityY = 0.0f;
                            g_player.isOnGround = true;
                        }
                    }
                }
            }
        }
    }

    // Portal handling
    static float portalCooldown = 0.0f;
    if (portalCooldown > 0.0f) {
        portalCooldown -= deltaTime;
    }

    if (portalCooldown <= 0.0f) {
        std::string targetMap;
        int portalId, linkedSpawnId;
        if (g_mapManager.GetCurrentMap()->CheckPortalCollision(
            g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            targetMap, portalId, linkedSpawnId)) {

            g_mapManager.SwitchMap(targetMap, portalId, linkedSpawnId);
            portalCooldown = 1.0f;
        }
    }
    // If velocityY absolute value > 0.05f, consider player not on ground
    if (fabs(g_player.velocityY) > 0.05f) {
        g_player.isOnGround = false;
    }

    // Boundary check
    if (g_player.posY < -2.0f) {
        ResetGame();
    }

    CheckDashAttack();
}

// New: Update player death state
void UpdatePlayerDeath(float deltaTime) {
    if (!g_player.isDead) {
        return;
    }

    g_player.deathTimer -= deltaTime;

    if (g_player.deathTimer <= 0.0f) {
        ResetGame();
    }
}

// New: Player death handler
void OnPlayerDeath() {
    g_player.isDead = true;
    g_player.deathTimer = g_player.DEATH_RESPAWN_TIME;
    g_player.deathCount++;

    // Stop all player actions
    g_player.isMoving = false;
    g_player.isDashing = false;
    g_player.isCharging = false;
    g_player.isInDashAftermath = false;
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;

    // Death sound can be added here
    // g_audioManager.PlaySFX("death_sound.wav");
}


// New: Check if player should die
void CheckPlayerDeath() {
    if (g_player.isDead) {
        return;
    }

    // Check collision with all alive enemies
    for (auto& enemy : g_enemies) {
        if (!enemy->IsAlive()) continue;

        if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight())) {

            // If player is dashing, they won't die but will attack the enemy instead
            if (g_player.isDashing) {
                // Attack logic handled in CheckDashAttack
                continue;
            }
            else {
                // Otherwise, player dies
                OnPlayerDeath();
            }
            return;
        }
    }

}

// Modified UpdateDash function: add charge decay update
void UpdateDash(float deltaTime) {
    // Prioritize dashing state update
    if (g_player.isDashing) {
        g_player.dashTimer -= deltaTime;

        if (g_player.dashTimer <= 0.0f) {
            g_player.isDashing = false;
            g_player.hasMouseTarget = false;
            EnterDashAftermath(); // Enter aftermath when dash ends
        }
    }

    // Then update aftermath state
    UpdateDashAftermath(deltaTime);
    // Update dash point recovery system
    UpdateDashPoints(deltaTime);

    // Update charge decay timer
    g_player.UpdateChargeDecay(deltaTime);

    // Charge logic should be independent of aftermath state
    if (g_player.isCharging) {
        g_player.chargeTime += deltaTime;

        // Allow charging during stun, but charge time cannot be too long
        if (g_player.chargeTime >= g_player.MAX_CHARGE_TIME) {
            // When charge time is max, clear aftermath first if in aftermath
            if (g_player.isInDashAftermath) {
                g_player.isInDashAftermath = false;
            }
            ExecuteMouseChargeDash();
        }
    }


    CheckPlayerDeath();
}

void CancelChargeDash() {
    if (g_player.isCharging) {
        g_player.isCharging = false;
        g_player.chargeTime = 0.0f;
    }
}

void MovePlayerLeft() {
    // Check if charging and movement is not allowed during charge
    if (g_player.isCharging && !g_player.allowMoveWhileCharging) {
        return;  // Do not allow movement during charge
    }

    // If in aftermath, movement will interrupt it
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }

    g_player.velocityX = -MOVE_SPEED;
    g_player.isMoving = true;
    g_player.facingRight = false;
}

void MovePlayerRight() {
    // Check if charging and movement is not allowed during charge
    if (g_player.isCharging && !g_player.allowMoveWhileCharging) {
        return;
    }

    // If in aftermath, movement will interrupt it
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }

    g_player.velocityX = MOVE_SPEED;
    g_player.isMoving = true;
    g_player.facingRight = true;
}

void StopPlayer() {
    if (!g_player.isDashing) {
        g_player.velocityX = 0.0f;
    }
    g_player.isMoving = false;
}

// Improved jump function
void Jump() {
    if (g_player.isOnGround && !g_player.isDashing && !g_player.isCharging) {
        g_player.velocityY = JUMP_FORCE;
        g_player.isOnGround = false;
    }
}

// Method 3: Mouse direction dash
void DashToMouse() {
    // Check if points are sufficient
    if (g_player.dashPoints <= 0) {
        return;
    }

    // Consume dash point
    if (!ConsumeDashPoint()) {
        return;
    }

    // Get mouse world coordinates
    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY);

    // Calculate direction vector from player to mouse
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dirX = mouseX - playerCenterX;
    float dirY = mouseY - playerCenterY;

    // Normalize direction vector
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }
    else {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
        dirY = 0.0f;
    }

    // Set dash state
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION;
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // Set dash speed
    g_player.velocityX = dirX * DASH_SPEED;
    g_player.velocityY = dirY * DASH_SPEED;

    // Store mouse target position
    g_player.mouseTargetX = mouseX;
    g_player.mouseTargetY = mouseY;
    g_player.hasMouseTarget = true;
}

// Modified StartMouseChargeDash function: adds charge inheritance logic
void StartMouseChargeDash() {
    // Conditions: not dashing, not charging, points sufficient, in actionable state
    if (g_player.isDashing || g_player.isCharging || g_player.dashPoints <= 0) {
        return;
    }

    g_player.isCharging = true;

    // Check for saved charge; if exists, inherit it
    if (g_player.hasSavedCharge) {
        g_player.LoadSavedCharge(); // Load saved charge time
        // Do not clear saved charge, allowing continued inheritance (until decay time ends)
    }
    else {
        g_player.chargeTime = 0.0f; // No saved charge, start from beginning
    }

    // Record initial mouse position
    g_inputSystem.GetMousePosition(g_player.mouseTargetX, g_player.mouseTargetY);
    g_player.hasMouseTarget = true;
}

// Modified ExecuteMouseChargeDash function: save charge when dash ends
void ExecuteMouseChargeDash() {
    if (!g_player.isCharging) return;

    // Allow charged dash even in aftermath state
    if (g_player.dashPoints <= 0) return;

    // Clear aftermath state to allow new dash
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }

    g_player.hitEnemies.clear();
    if (!ConsumeDashPoint()) return;

    // Get current mouse position
    float currentMouseX, currentMouseY;
    g_inputSystem.GetMousePosition(currentMouseX, currentMouseY);

    // Calculate direction from player to mouse
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dirX = currentMouseX - playerCenterX;
    float dirY = currentMouseY - playerCenterY;

    // Normalize
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }
    else {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
        dirY = 0.0f;
    }

    // Get current charge level
    int chargeLevel = g_player.GetChargeLevel();

    // Three-stage charge determination
    float speedMultiplier = 1.0f;
    float durationMultiplier = 1.0f;
    float cooldownMultiplier = 1.0f;

    // Set attribute multipliers based on charge level
    switch (chargeLevel) {
    case 1:
        speedMultiplier = 1.3f;
        durationMultiplier = 1.0f;
        cooldownMultiplier = 0.8f;
        break;
    case 2:
        speedMultiplier = 1.6f;
        durationMultiplier = 1.0f;
        cooldownMultiplier = 0.6f;
        break;
    case 3:
        speedMultiplier = 2.0f;
        durationMultiplier = 1.0f;
        cooldownMultiplier = 0.5f;
        break;
    default:
        speedMultiplier = 1.0f;
        durationMultiplier = 1.0f;
        cooldownMultiplier = 1.0f;
        break;
    }

    // Set dash state
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION * durationMultiplier;
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // Apply dash speed
    g_player.velocityX = dirX * DASH_SPEED * speedMultiplier;
    g_player.velocityY = dirY * DASH_SPEED * speedMultiplier;

    // Store new mouse target position
    g_player.mouseTargetX = currentMouseX;
    g_player.mouseTargetY = currentMouseY;

    // === Key modification: Save current charge time when dash ends ===
    // Only save if charge time is at a valid value (to avoid saving invalid charges)
    if (g_player.chargeTime >= g_player.MIN_CHARGE_TIME) {
        g_player.SaveCharge(); // Save current charge time
    }

    // End charging state
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
}


// Enter dash aftermath state
void EnterDashAftermath() {
    // Clear all velocity to keep player stationary
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;

    // Do not enter aftermath if no points left
    if (g_player.dashPoints <= 0) {
        g_player.ClearSavedCharge();
        return;
    }

    g_player.isInDashAftermath = true;
    g_player.dashAftermathTimer = g_player.DASH_AFTERMATH_DURATION;
}

// Update aftermath state
void UpdateDashAftermath(float deltaTime) {
    if (!g_player.isInDashAftermath) return;

    g_player.dashAftermathTimer -= deltaTime;

    // Check for movement input to interrupt
    if (g_inputSystem.IsMovingLeft() || g_inputSystem.IsMovingRight()) {
        g_player.isInDashAftermath = false;
        g_player.velocityY = 0.0f;
        return;
    }

    // Aftermath state ends
    if (g_player.dashAftermathTimer <= 0.0f) {
        g_player.isInDashAftermath = false;
        g_player.velocityY = 0.0f;
    }
}

// Update dash point recovery
void UpdateDashPoints(float deltaTime) {
    // Ground recovery of points
    if (g_player.isOnGround && g_player.dashPoints < g_player.MAX_DASH_POINTS) {
        g_player.dashPointRecoverTimer += deltaTime;

        if (g_player.dashPointRecoverTimer >= g_player.DASH_POINT_RECOVER_TIME) {
            g_player.dashPoints++;
            g_player.dashPointRecoverTimer = 0.0f;
        }
    }
    else {
        g_player.dashPointRecoverTimer = 0.0f;
    }
}

// Consume dash point
bool ConsumeDashPoint() {
    if (g_player.dashPoints > 0) {
        g_player.dashPoints--;
        return true;
    }
    return false;
}

// Restore point on enemy defeat (reserved interface)
void OnEnemyDefeated() {
    if (g_player.dashPoints < g_player.MAX_DASH_POINTS) {
        g_player.dashPoints++;
    }
}


void CheckDashAttack() {
    if (!g_player.isDashing) {
        g_player.hitEnemies.clear();
        return;
    }

    // Calculate player dash angle
    float dashAngle = atan2(g_player.dashDirectionY, g_player.dashDirectionX);

    for (auto& enemy : g_enemies) {
        if (!enemy->IsAlive()) continue;

        // Check if already hit this enemy
        if (std::find(g_player.hitEnemies.begin(), g_player.hitEnemies.end(), enemy) != g_player.hitEnemies.end()) {
            continue;
        }

        // Check collision
        if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight())) {

            // Directly pass player dash angle, enemy calculates relative direction
            int actualDamage = enemy->CalculateDamageFromPlayer((int)g_player.attackDamage, dashAngle);

            // Deal damage to enemy
            enemy->TakeDamage(actualDamage, dashAngle);

            // Mark as hit
            g_player.hitEnemies.push_back(enemy);
        }
    }
}