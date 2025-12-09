#include "Game.h"
#include "Enemy.h"

// Íæ¼ÒÎEúÔEÂ
void UpdatePlayerPhysics(float deltaTime) {
    // ÔÚÓ²Ö±×´Ì¬ÏÂºöÂÔÖØÁ¦ºÍÒÆ¶¯
    if (g_player.isInDashAftermath) {
        // Ö»´¦ÀúĞ¹Ö±Åö×²¼Eâ£¨·ÀÖ¹µô³öµØÍ¼£©
        auto& solidTiles = g_mapManager.GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                tile.posX, tile.posY, tile.width, tile.height)) {
                // ¼òµ¥µÄ´¹Ö±Åö×²´¦ÀE
                g_player.posY = tile.posY + tile.height; // Õ¾ÔÚµØÃæÉÏ
                g_player.isOnGround = true;
            }
        }
        return; // Ó²Ö±×´Ì¬ÏÂÌø¹ıÕı³£ÎEúÔEÂ
    }
    // Ó¦ÓÃÖØÁ¦...
    if (!g_player.isDashing) {
        float fixedDeltaTime = std::min(deltaTime, 0.033f);
        g_player.velocityY += GRAVITY * fixedDeltaTime * 60.0f;
        if (g_player.velocityY < -0.3f) {
            g_player.velocityY = -0.3f;
        }
    }

    float originalX = g_player.posX;
    float originalY = g_player.posY;

    // Ë®Æ½ÒÆ¶¯
    g_player.posX += g_player.velocityX * deltaTime * 60.0f;

    // Ê¹ÓÃĞÂµÄµØÍ¼ÏµÍ³½øĞĞÅö×²¼EE
    auto& solidTiles = g_mapManager.GetCurrentMap()->GetSolidTiles();
    for (const auto& tile : solidTiles) {
        if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            tile.posX, tile.posY, tile.width, tile.height)) {
            g_player.velocityX = 0.0f;
        }
    }

    // ´¹Ö±ÒÆ¶¯
    g_player.posY += g_player.velocityY * deltaTime * 60.0f;
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

            if (overlapX < overlapY) {
                if (playerCenterX < tileCenterX) {
                    g_player.posX = tile.posX - PLAYER_WIDTH;
                }
                else {
                    g_player.posX = tile.posX + tile.width;
                }
                g_player.velocityX = 0.0f;
            }
            else {
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

    // ´«ËÍÃÅ¼EE
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

    // ±ß½ç¼EE
    if (g_player.posY < -2.0f) {
        ResetGame();
    }

    CheckDashAttack();
}

void UpdateDash(float deltaTime) {
    // ÓÅÏÈ¸EÂ³å´Ì×´Ì¬
    if (g_player.isDashing) {
        g_player.dashTimer -= deltaTime;

        if (g_player.dashTimer <= 0.0f) {
            g_player.isDashing = false;
            g_player.hasMouseTarget = false;
            EnterDashAftermath(); // ³å´Ì½áÊø½øÈE²Ö±
        }
    }

    // È»ºó¸EÂÓ²Ö±×´Ì¬
    UpdateDashAftermath(deltaTime);
    // ×ûÖó¸EÂµãÊı»Ö¸´ÏµÍ³
    UpdateDashPoints(deltaTime);

    // Ğûİ¦Âß¼­Ó¦¸Ã¶ÀÁ¢ÓÚÓ²Ö±×´Ì¬
    if (g_player.isCharging) {
        g_player.chargeTime += deltaTime;

        // Ó²Ö±×´Ì¬ÏÂÔÊĞúìûİ¦£¬µ«Ğûİ¦ÍEÉÊ±¼EéÌõ¼ş
        if (g_player.chargeTime >= g_player.MAX_CHARGE_TIME) {
            // Ğûİ¦ÍEÉÊ±£¬Èç¹û´¦ÓÚÓ²Ö±×´Ì¬£¬ÏÈÇå³ıÓ²Ö±
            if (g_player.isInDashAftermath) {
                g_player.isInDashAftermath = false;
            }
            ExecuteMouseChargeDash();
        }
    }
}

void CancelChargeDash() {
    if (g_player.isCharging) {
        g_player.isCharging = false;
        g_player.chargeTime = 0.0f;
    }
}

void MovePlayerLeft() {
    // ¼EéÊÇ·ñ´¦ÓÚĞûİ¦×´Ì¬ÇÒ²»ÔÊĞúîÆ¶¯
    if (g_player.isCharging && !g_player.allowMoveWhileCharging) {
        return;  // Ğûİ¦ÖĞ²»ÔÊĞúîÆ¶¯£¬Ö±½Ó·µ»Ø
    }

    // Èç¹û´¦ÓÚÓ²Ö±×´Ì¬£¬ÒÆ¶¯»á´ò¶ÏÓ²Ö±
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }

    g_player.velocityX = -MOVE_SPEED;
    g_player.isMoving = true;
    g_player.facingRight = false;
}

void MovePlayerRight() {
    // ¼EéÊÇ·ñ´¦ÓÚĞûİ¦×´Ì¬ÇÒ²»ÔÊĞúîÆ¶¯
    if (g_player.isCharging && !g_player.allowMoveWhileCharging) {
        return;
    }

    // Èç¹û´¦ÓÚÓ²Ö±×´Ì¬£¬ÒÆ¶¯»á´ò¶ÏÓ²Ö±
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

// ·½·¨3: Êó±E½Ïò³å´Ì
void DashToMouse() {
    // ¼EéµãÊıÊÇ·ñ×ã¹»
    if (g_player.dashPoints <= 0) {
        return;
    }

    // ÏûºÄ³å´ÌµãÊı
    if (!ConsumeDashPoint()) {
        return;
    }

    // »ñÈ¡Êó±EÀ½ç×ø±E
    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY);

    // ¼ÆËã´ÓÍæ¼ÒÖ¸ÏòÊó±EÄ·½ÏòÏòÁ¿
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dirX = mouseX - playerCenterX;
    float dirY = mouseY - playerCenterY;

    // ¹éÒ»»¯·½ÏòÏòÁ¿
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }
    else {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
        dirY = 0.0f;
    }

    // ÉèÖÃ³å´Ì×´Ì¬
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION;
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // ÉèÖÃ³å´ÌËÙ¶È
    g_player.velocityX = dirX * DASH_SPEED;
    g_player.velocityY = dirY * DASH_SPEED;

    // ´æ´¢Êó±E¿±E»ÖÃ
    g_player.mouseTargetX = mouseX;
    g_player.mouseTargetY = mouseY;
    g_player.hasMouseTarget = true;
}

// ·½·¨4: Êó±Eûİ¦³å´Ì
void StartMouseChargeDash() {
    // ¼EéÌõ¼ş£ºÊÇ·ñÕıÔÚ³å´Ì¡¢ÊÇ·ñÕıÔÚĞûİ¦¡¢µãÊıÊÇ·ñ×ã¹»¡¢ÊÇ·ñ´¦ÓÚ¿ÉĞĞ¶¯×´Ì¬
    if (g_player.isDashing || g_player.isCharging || g_player.dashPoints <= 0) {
        return;
    }

    g_player.isCharging = true;
    g_player.chargeTime = 0.0f;

    // ¼ÇÂ¼³õÊ¼Êó±E»ÖÃ
    g_inputSystem.GetMousePosition(g_player.mouseTargetX, g_player.mouseTargetY);
    g_player.hasMouseTarget = true;
}

void ExecuteMouseChargeDash() {
    if (!g_player.isCharging) return;

    // ÔÊĞúğÚÓ²Ö±×´Ì¬ÏÂ½øĞĞĞûİ¦³å´Ì
    if (g_player.dashPoints <= 0) return;

    // Çå³ıÓ²Ö±×´Ì¬£¬ÔÊĞúìÂµÄ³å´Ì
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }

    g_player.hitEnemies.clear();
    if (!ConsumeDashPoint()) return;
    // »ñÈ¡µ±Ç°Êó±E»ÖÃ
    float currentMouseX, currentMouseY;
    g_inputSystem.GetMousePosition(currentMouseX, currentMouseY);

    // ¼ÆËã´ÓÍæ¼ÒÖ¸ÏòÊó±EÄ·½ÏE
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dirX = currentMouseX - playerCenterX;
    float dirY = currentMouseY - playerCenterY;

    // ¹éÒ»»¯
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }
    else {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
        dirY = 0.0f;
    }

    // Èı¶ÎĞûİ¦ÅĞ¶¨
    float speedMultiplier = 1.0f;
    float durationMultiplier = 1.0f;
    float cooldownMultiplier = 1.0f;

    // ¸EÂÊôĞÔ±¶ÂÊ´úÂE
    if (g_player.chargeTime >= g_player.CHARGE_THRESHOLD_LOW && g_player.chargeTime < g_player.CHARGE_THRESHOLD_MID) {
        speedMultiplier = 1.3f;
        durationMultiplier = 1.2f;
        cooldownMultiplier = 0.8f;
    }
    else if (g_player.chargeTime >= g_player.CHARGE_THRESHOLD_MID && g_player.chargeTime < g_player.CHARGE_THRESHOLD_HIGH) {
        speedMultiplier = 1.6f;
        durationMultiplier = 1.4f;
        cooldownMultiplier = 0.6f;
    }
    else if (g_player.chargeTime >= g_player.CHARGE_THRESHOLD_HIGH) {
        speedMultiplier = 2.0f;
        durationMultiplier = 1.8f;
        cooldownMultiplier = 0.5f;
    }

    // ÉèÖÃ³å´Ì×´Ì¬
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION * durationMultiplier;
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // Ó¦ÓÃ³å´ÌËÙ¶È
    g_player.velocityX = dirX * DASH_SPEED * speedMultiplier;
    g_player.velocityY = dirY * DASH_SPEED * speedMultiplier;

    // ´æ´¢×ûòÕÊó±E»ÖÃ
    g_player.mouseTargetX = currentMouseX;
    g_player.mouseTargetY = currentMouseY;

    // ½áÊøĞûİ¦×´Ì¬
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
}

// ½øÈEå´ÌºóÓ²Ö±×´Ì¬
void EnterDashAftermath() {
    // Çå³ıËùÓĞËÙ¶È£¬Ê¹Íæ¼ÒÍE«Í£Ö¹
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;

    // Èç¹ûÃ»ÓĞµãÊıÔò²»½øÈE²Ö±×´Ì¬
    if (g_player.dashPoints <= 0) {
        return;
    }

    g_player.isInDashAftermath = true;
    g_player.dashAftermathTimer = g_player.DASH_AFTERMATH_DURATION;
}

// ¸EÂÓ²Ö±×´Ì¬
void UpdateDashAftermath(float deltaTime) {
    if (!g_player.isInDashAftermath) return;

    g_player.dashAftermathTimer -= deltaTime;

    // ¼EéÒÆ¶¯ÊäÈEò¶Ï
    if (g_inputSystem.IsMovingLeft() || g_inputSystem.IsMovingRight()) {
        g_player.isInDashAftermath = false;
        g_player.velocityY = 0.0f;
        return;
    }

    // Ó²Ö±×´Ì¬½áÊE
    if (g_player.dashAftermathTimer <= 0.0f) {
        g_player.isInDashAftermath = false;
        g_player.velocityY = 0.0f;
    }
}

// ¸EÂ³å´ÌµãÊı»Ö¸´
void UpdateDashPoints(float deltaTime) {
    // µØÃæ»Ö¸´µãÊı
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

// ÏûºÄ³å´ÌµãÊı
bool ConsumeDashPoint() {
    if (g_player.dashPoints > 0) {
        g_player.dashPoints--;
        return true;
    }
    return false;
}

// »÷°ÜµĞÈËÊ±»Ö¸´µãÊı£¨Ô¤Áô½Ó¿Ú£©
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

    // ¼ÆËãÍæ¼Ò³å´Ì½Ç¶È
    float dashAngle = atan2(g_player.dashDirectionY, g_player.dashDirectionX);

    for (auto& enemy : g_enemies) {
        if (!enemy->IsAlive()) continue;

        // ¼EéÊÇ·ñÒÑ¾­»÷ÖĞ¹ıÕâ¸öµĞÈË
        if (std::find(g_player.hitEnemies.begin(), g_player.hitEnemies.end(), enemy) != g_player.hitEnemies.end()) {
            continue;
        }

        // ¼EâÅö×²
        if (CheckCollision(g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT,
            enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight())) {

            // Ö±½Ó´«ÈEæ¼Ò³å´Ì½Ç¶È£¬µĞÈË×Ô¼º¼ÆËãÏà¶Ô·½ÏE
            int actualDamage = enemy->CalculateDamageFromPlayer((int)g_player.attackDamage, dashAngle);

            // ¶ÔµĞÈËÔEÉÉËº¦
            enemy->TakeDamage(actualDamage, dashAngle);

            // ±EÇÎªÒÑ»÷ÖĞ
            g_player.hitEnemies.push_back(enemy);
        }
    }
}