#include "Enemy.h"
#include "Map.h"

// Initialize damage number manager
std::vector<DamageNumber> DamageNumberManager::damageNumbers;

// Enemy class implementation
Enemy::Enemy(float x, float y, float hp)
    : posX(x), posY(y), health(hp), maxHealth(hp), isAlive(true),
    currentState(PATROL), patrolMinX(-1.0f), patrolMaxX(1.0f), attackRange(0.08f) {

    width = PLAYER_WIDTH * 1.2f;
    height = PLAYER_HEIGHT * 1.2f;
    moveSpeed = MOVE_SPEED * 0.65f;

    // Initialize damage multipliers
    for (int i = 0; i < 8; i++) {
        damageMultipliers[i] = 1.0f;
    }

    facingRight = true;  // Default facing right
    velocityX = 0.0f;
    velocityY = 0.0f;

    // Set base 8-direction damage multipliers
    SetDamageMultiplier(DIR_FRONT, 1.0f);
    SetDamageMultiplier(DIR_FRONT_UP, 1.0f);
    SetDamageMultiplier(DIR_UP, 1.0f);
    SetDamageMultiplier(DIR_BACK_UP, 1.0f);
    SetDamageMultiplier(DIR_BACK, 1.0f);
    SetDamageMultiplier(DIR_BACK_DOWN, 1.0f);
    SetDamageMultiplier(DIR_DOWN, 1.0f);
    SetDamageMultiplier(DIR_FRONT_DOWN, 1.0f);
}

void Enemy::SetDamageMultiplier(Direction dir, float multiplier) {
    if (dir >= DIR_FRONT && dir <= DIR_FRONT_DOWN) {
        damageMultipliers[static_cast<int>(dir)] = multiplier;
    }
}

float Enemy::GetDamageMultiplier(float attackAngle) {
    float relativeAngle = GetRelativeAngle(attackAngle);
    int directionIndex = AngleToDirectionIndex(relativeAngle);
    return damageMultipliers[directionIndex];
}

// Calculate relative angle (based on enemy facing direction)
float Enemy::GetRelativeAngle(float attackAngle) const {
    // When facing right, 0 degrees is front; when facing left, 180 degrees is front
    float enemyFrontAngle = facingRight ? 0.0f : 3.14159f;
    float relativeAngle = attackAngle - enemyFrontAngle;

    // Normalize to [-π, π]
    while (relativeAngle > 3.14159f) relativeAngle -= 2 * 3.14159f;
    while (relativeAngle < -3.14159f) relativeAngle += 2 * 3.14159f;

    return relativeAngle;
}

// Convert angle to direction index (8 directions)
int Enemy::AngleToDirectionIndex(float relativeAngle) {
    // Normalize relative angle to [0, 2π]
    float angle = relativeAngle;
    if (angle < 0) angle += 2 * 3.14159f;

    // 8 directions, each 45 degrees
    float sector = 3.14159f / 4.0f;

    // Calculate direction index
    int index = static_cast<int>((angle + sector / 2) / sector) % 8;
    return index;
}

float Enemy::NormalizeAngle(float angle) {
    while (angle < 0) angle += 2 * 3.14159f;
    while (angle >= 2 * 3.14159f) angle -= 2 * 3.14159f;
    return angle;
}

// Calculate damage based on attack angle
int Enemy::CalculateDamageFromPlayer(int baseDamage, float playerDashAngle) {
    float multiplier = GetDamageMultiplier(playerDashAngle);
    return (int)(baseDamage * multiplier);
}

// Use DamageNumberManager in TakeDamage method
void Enemy::TakeDamage(int damage, float attackAngle) {
    if (!isAlive) return;

    float multiplier = GetDamageMultiplier(attackAngle);
    int actualDamage = (int)(damage * multiplier);

    // Use independent damage number manager
    bool isCritical = (multiplier > 1.5f);
    DamageNumberManager::AddDamageNumber(
        posX + width * 0.5f,  // Enemy center X
        posY + height,        // Top of enemy
        actualDamage,
        isCritical
    );

    health -= actualDamage;
    isHit = true;
    hitTimer = HIT_DURATION;
    OnHit(actualDamage);

    if (health <= 0) {
        health = 0;
        isAlive = false;
        OnDeath();
    }
}


void Enemy::OnHit(int damage) {
    // Base enemy has no special behavior when hit
}

void Enemy::OnDeath() {
    // Base enemy death handling
    isAlive = false;
}

void Enemy::Update(float deltaTime, MapManager* mapManager) {
    if (!isAlive) return;

    // Update hit state
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    // Apply gravity
    velocityY += GRAVITY * deltaTime * 60.0f;

    // Save old position for collision detection
    float oldX = posX;
    float oldY = posY;

    // Move
    posX += velocityX * deltaTime * 60.0f;

    // Horizontal collision detection - use map manager to get solid tiles
    bool horizontalCollision = false;
    if (mapManager && mapManager->GetCurrentMap()) {
        auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (CheckCollisionWithTile(tile)) {
                posX = oldX;
                horizontalCollision = true;
                break;
            }
        }
    }

    posY += velocityY * deltaTime * 60.0f;

    // Vertical collision detection
    bool verticalCollision = false;
    if (mapManager && mapManager->GetCurrentMap()) {
        auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (CheckCollisionWithTile(tile)) {
                posY = oldY;
                verticalCollision = true;
                velocityY = 0;
                break;
            }
        }
    }

    // Reset horizontal velocity only on horizontal collision
    if (horizontalCollision) {
        velocityX = 0;
    }

    // Boundary check - use map boundaries
    if (mapManager && mapManager->GetCurrentMap()) {
        // Can add map-based boundary checks here
        if (posY < -5.0f) { // Fall death height
            isAlive = false;
            return;
        }
    }
    else {
        // Fallback boundary check
        if (posX < -1.0f) posX = -1.0f;
        if (posX > 1.0f - width) posX = 1.0f - width;
        if (posY < -2.0f) {
            isAlive = false;
            return;
        }
    }

    UpdateAI(deltaTime);
}

void Enemy::UpdateAI(float deltaTime) {
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    // Update facing direction
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // State machine logic
    switch (currentState) {
    case PATROL:
        PatrolBehavior(deltaTime);
        if (distance < 3.0f) currentState = CHASE;
        break;
    case CHASE:
        ChaseBehavior(deltaTime);
        if (distance > 8.0f) currentState = PATROL;
        if (distance < attackRange) currentState = ATTACK;
        break;
    case ATTACK:
        AttackBehavior(deltaTime);
        if (distance > attackRange + 0.2f) currentState = CHASE;
        if (health < maxHealth * 0.3f) currentState = FLEE;
        break;
    case FLEE:
        FleeBehavior(deltaTime);
        if (health > maxHealth * 0.5f) currentState = CHASE;
        break;
    }
}

void Enemy::PatrolBehavior(float deltaTime) {
    static float patrolDirection = 1.0f;
    static float patrolTimer = 0.0f;

    patrolTimer += deltaTime;

    // Check if need to change direction every 2 seconds
    if (patrolTimer >= 2.0f) {
        if (posX <= patrolMinX || posX >= patrolMaxX) {
            patrolDirection *= -1.0f;
        }
        patrolTimer = 0.0f;
    }

    velocityX = patrolDirection * moveSpeed * 0.5f;

    // Add small vertical velocity variation to avoid complete stillness
    if (velocityY == 0) {
        velocityY = 0.01f;
    }
}

void Enemy::ChaseBehavior(float deltaTime) {
    if (g_player.posX > posX) {
        velocityX = moveSpeed;
    }
    else {
        velocityX = -moveSpeed;
    }

    // Simple jump attempt
    if (abs(g_player.posX - posX) < 0.3f && g_player.posY > posY + 0.2f) {
        velocityY = JUMP_FORCE * 0.8f;
    }
}

void Enemy::AttackBehavior(float deltaTime) {
    // Stop moving to attack
    velocityX = 0;
    velocityY = 0;
}

void Enemy::FleeBehavior(float deltaTime) {
    // Move away from player
    if (g_player.posX > posX) {
        velocityX = -moveSpeed;
    }
    else {
        velocityX = moveSpeed;
    }
}


void Enemy::WorldToScreenPosition(float worldX, float worldY, float& screenX, float& screenY, const Camera& camera) {
    // Get camera position (camera center coordinates)
    float cameraX = camera.GetX();
    float cameraY = camera.GetY();

    // Convert world coordinates to screen coordinates (relative coordinates)
    // Assume rendering system uses screen center as origin (0,0)
    screenX = worldX - cameraX;
    screenY = worldY - cameraY;
}

void Enemy::Render(ID3D11ShaderResourceView* texture, const Camera& camera) {
    if (!isAlive) return;

    // Convert world coordinates to screen coordinates
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    // Select different frames based on health status
    int frameIndex = 0;
    if (health < maxHealth * 0.3f) {
        frameIndex = 1;
    }
    if (currentState == ATTACK) {
        frameIndex = 2;
    }
    // Hit state: flicker or color change effect
    if (isHit) {
        SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    }

    // Render enemy using screen coordinates
    RenderImage(screenX, screenY, width, height, texture, frameIndex, 1, 3);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    // Render health bar (also needs screen coordinates)
    RenderHealthBar(camera);
}

void Enemy::RenderHealthBar(const Camera& camera) {
    // Convert world coordinates to screen coordinates
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    float barWidth = width;
    float barHeight = 0.02f;
    float barX = screenX;
    float barY = screenY + height + 0.02f;

    // Background bar (red)
    RenderImage(barX, barY, barWidth, barHeight, g_groundTexture, 0, 1, 1);

    // Health bar (green)
    float healthRatio = health / maxHealth;
    RenderImage(barX, barY, barWidth * healthRatio, barHeight, g_groundTexture, 1, 1, 1);
}

bool Enemy::CheckPlayerCollision() {
    return CheckCollision(posX, posY, width, height,
        g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT);
}

bool Enemy::CheckCollisionWithTiles(const std::vector<MapTile>& solidTiles) {
    for (const auto& tile : solidTiles) {
        if (CheckCollisionWithTile(tile)) {
            return true;
        }
    }
    return false;
}

bool Enemy::CheckCollisionWithTile(const MapTile& tile) {
    return CheckCollision(posX, posY, width, height,
        tile.posX, tile.posY, tile.width, tile.height);
}

// ShieldEnemy implementation - modify damage multiplier settings
ShieldEnemy::ShieldEnemy(float x, float y) : Enemy(x, y, 150.0f) {
    // Shield enemy: reduced damage from front, increased damage from back
    SetDamageMultiplier(DIR_FRONT, 0.1f);
    SetDamageMultiplier(DIR_FRONT_UP, 0.3f);
    SetDamageMultiplier(DIR_FRONT_DOWN, 0.3f);
    SetDamageMultiplier(DIR_BACK, 2.0f);
    SetDamageMultiplier(DIR_BACK_UP, 1.5f);
    SetDamageMultiplier(DIR_BACK_DOWN, 1.5f);

    width = PLAYER_WIDTH * 1.5f;
    moveSpeed = MOVE_SPEED * 0.5f;
}


void ShieldEnemy::Update(float deltaTime, MapManager* mapManager) {
    Enemy::Update(deltaTime, mapManager);

    // Shield enemy special AI logic
    if (currentState == PATROL) {
        // Shield enemy patrols slower but more stable
        velocityX *= 0.8f;
    }
}

void ShieldEnemy::OnHit(int damage) {
    // Shield enemy may block when hit
    if (damage < 5) {
        health += damage; // Rollback damage
    }
}

void ShieldEnemy::OnDeath() {
    Enemy::OnDeath();
}
// MageEnemy implementation
MageEnemy::MageEnemy(float x, float y) : Enemy(x, y, 80.0f) {
    // Mage enemy: vulnerable from top and bottom
    SetDamageMultiplier(DIR_UP, 2.0f);
    SetDamageMultiplier(DIR_DOWN, 2.0f);

    spellCooldown = 3.0f;
    currentSpellCooldown = 0.0f;
    attackRange = 1.2f;
    moveSpeed = MOVE_SPEED * 0.4f;
}

void MageEnemy::Update(float deltaTime, MapManager* mapManager) {
    Enemy::Update(deltaTime, mapManager);

    if (currentSpellCooldown > 0) {
        currentSpellCooldown -= deltaTime;
    }

    // Cast spell in attack state
    if (currentState == ATTACK && currentSpellCooldown <= 0) {
        CastSpell();
        currentSpellCooldown = spellCooldown;
    }
}

void MageEnemy::CastSpell() {
    // Implement spell casting logic here
}

void MageEnemy::Render(ID3D11ShaderResourceView* texture, const Camera& camera) {
    if (!isAlive) return;

    // Convert world coordinates to screen coordinates
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    int frameIndex = 0;
    if (health < maxHealth * 0.3f) {
        frameIndex = 1;
    }
    if (currentState == ATTACK) {
        frameIndex = 2;
    }

    // Render mage enemy
    RenderImage(screenX, screenY, width, height, texture, frameIndex, 1, 3);

    // Render health bar
    RenderHealthBar(camera);

    // Mage enemy has magic effects (also needs screen coordinates)
    if (currentState == ATTACK) {
        float effectSize = width * 1.3f;
        float effectX = screenX - (effectSize - width) * 0.5f;
        float effectY = screenY - (effectSize - height) * 0.5f;

        RenderImage(effectX, effectY, effectSize, effectSize, g_chargeEffectTexture, 0, 1, 3);
    }
}

// FastEnemy implementation
FastEnemy::FastEnemy(float x, float y) : Enemy(x, y, 60.0f) {
    moveSpeed = MOVE_SPEED * 1.5f;
    dashCooldown = 2.0f;
    currentDashCooldown = 0.0f;
}

void FastEnemy::Update(float deltaTime, MapManager* mapManager) {
    Enemy::Update(deltaTime, mapManager);

    if (currentDashCooldown > 0) {
        currentDashCooldown -= deltaTime;
    }

    if (currentState == ATTACK && currentDashCooldown <= 0) {
        DashAttack();
        currentDashCooldown = dashCooldown;
    }
}

void FastEnemy::DashAttack() {
    velocityX = (g_player.posX > posX ? 1.0f : -1.0f) * moveSpeed * 3.0f;
}

// Enemy management functions
void InitEnemies() {
    // Load enemy textures
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_enemyTexture);
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_shieldEnemyTexture);
    LoadTexture(g_pDevice, "asset/Enemy.png", &g_mageEnemyTexture);
    LoadTexture(g_pDevice, "asset/Enemy_Shield.png", &g_fastEnemyTexture);

    if (!g_enemyTexture) g_enemyTexture = g_playerTexture;
    if (!g_shieldEnemyTexture) g_shieldEnemyTexture = g_enemyTexture;
    if (!g_mageEnemyTexture) g_mageEnemyTexture = g_enemyTexture;
    if (!g_fastEnemyTexture) g_fastEnemyTexture = g_enemyTexture;
}


void UpdateEnemies(float deltaTime, MapManager* mapManager) {    // Update damage numbers

    DamageNumberManager::Update(deltaTime);
    for (auto& enemy : g_enemies) {
        enemy->Update(deltaTime, mapManager);
    }

    // Remove dead enemies
    g_enemies.erase(
        std::remove_if(g_enemies.begin(), g_enemies.end(),
            [](Enemy* e) {
                if (!e->IsAlive()) {
                    delete e;
                    return true;
                }
                return false;
            }),
        g_enemies.end()
    );
}

// Modify RenderEnemies function to pass camera parameter
void RenderEnemies(const Camera& camera) {
    for (auto& enemy : g_enemies) {
        ID3D11ShaderResourceView* texture = g_enemyTexture;

        if (dynamic_cast<ShieldEnemy*>(enemy)) {
            texture = g_shieldEnemyTexture;
        }
        else if (dynamic_cast<MageEnemy*>(enemy)) {
            texture = g_mageEnemyTexture;
        }
        else if (dynamic_cast<FastEnemy*>(enemy)) {
            texture = g_fastEnemyTexture;
        }

        enemy->Render(texture, camera); // Pass camera parameter
    }
    DamageNumberManager::Render(camera);
}

void CleanupEnemies() {
    DamageNumberManager::Clear();
    for (auto& enemy : g_enemies) {
        delete enemy;
    }
    g_enemies.clear();
}


void DamageNumberManager::AddDamageNumber(float x, float y, int damage, bool isCritical) {
    damageNumbers.emplace_back(x, y, damage, isCritical);
}

void DamageNumberManager::Update(float deltaTime) {
    for (auto it = damageNumbers.begin(); it != damageNumbers.end();) {
        it->timer += deltaTime;
        it->posY += it->velocityY * deltaTime;
        it->velocityY -= 2.0f * deltaTime; // Gravity effect

        if (it->timer >= it->lifeTime) {
            it = damageNumbers.erase(it);
        }
        else {
            ++it;
        }
    }
}

void DamageNumberManager::Render(const Camera& camera) {
    for (auto& number : damageNumbers) {
        float screenX, screenY;
        // Use static world-to-screen coordinate conversion function
        float cameraX = camera.GetX();
        float cameraY = camera.GetY();
        screenX = number.posX - cameraX;
        screenY = number.posY - cameraY;

        // Calculate alpha (fade-out effect)
        float alpha = 1.0f - (number.timer / number.lifeTime);

        // Set color based on whether it's critical
        if (number.isCritical) {
            //SetColor(1.0f, 0.0f, 0.0f, alpha); // Red critical numbers
            SetColor(1.0f, 1.0f, 1.0f, alpha); // White normal numbers
        }
        else {
            SetColor(1.0f, 1.0f, 1.0f, alpha); // White normal numbers
        }

        // Use existing number rendering functionality
        RenderNumber(number.value, screenX, screenY, 0.07f, 0.1f, pTextureNum);
    }

    // Reset color
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void DamageNumberManager::Clear() {
    damageNumbers.clear();
}