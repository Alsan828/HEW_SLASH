#include "Enemy.h"
#include "Map.h"
#include "Projectile.h"

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
    OnEnemyDefeated();
}
void Enemy::Update(float deltaTime, MapManager* mapManager) {
    if (!isAlive) return;

    // Visibility detection and optimization logic
    bool isCurrentlyVisible = IsVisible(g_camera);

    if (!isCurrentlyVisible && !NeedsMinimalUpdate()) {
        offScreenTimer += deltaTime;
        if (offScreenTimer > MAX_OFFSCREEN_TIME &&
            currentState == PATROL &&
            !isHit &&
            health >= maxHealth) {
            return;
        }
    }

    if (isCurrentlyVisible && !wasVisible) {
        ResetOffScreenTimer();
    }
    wasVisible = isCurrentlyVisible;

    if (!isCurrentlyVisible && NeedsMinimalUpdate()) {
        UpdateMinimal(deltaTime);
        return;
    }

    // Full update logic
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

    // Horizontal movement
    posX += velocityX * deltaTime * 60.0f;
    if (CheckHorizontalCollision(mapManager, oldX, oldY)) {
        posX = oldX;
        velocityX = 0.0f;
    }

    // Vertical movement
    posY += velocityY * deltaTime * 60.0f;
    if (CheckVerticalCollision(mapManager, oldX, oldY)) {
        posY = oldY;
        velocityY = 0.0f;
    }

    // Boundary check
    if (posY < -5.0f) {
        isAlive = false;
        return;
    }

    // AI update
    if (isCurrentlyVisible) {
        UpdateAI(deltaTime);
    }
    else {
        UpdateAIMinimal(deltaTime);
    }

    if (!isCurrentlyVisible) {
        offScreenTimer += deltaTime;
    }
    else {
        offScreenTimer = 0.0f;
    }
}


// Simplified update (for off-screen enemies that need updating)
void Enemy::UpdateMinimal(float deltaTime) {
    // Simplified AI update (only handles state transitions, no path calculations, etc.)
    UpdateAIMinimal(deltaTime);

    // Update off-screen timer
    offScreenTimer += deltaTime;
}

// Simplified AI update
void Enemy::UpdateAIMinimal(float deltaTime) {
    // Only handles basic state maintenance, no complex calculations
    float dx = g_player.posX - posX;

    // Update facing direction
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // Simplified state machine: only handles timeouts or critical state transitions
    static float stateTimer = 0.0f;
    stateTimer += deltaTime;

    // Check state transitions every 5 seconds (reduce frequency)
    if (stateTimer >= 5.0f) {
        float distance = fabs(dx);

        // Simplified state transition logic
        switch (currentState) {
        case PATROL:
            if (distance < 3.0f) currentState = CHASE;
            break;
        case CHASE:
            if (distance > 8.0f) currentState = PATROL;
            break;
        case ATTACK:
            // Attack state remains until conditions change
            break;
        case FLEE:
            if (health > maxHealth * 0.5f) currentState = CHASE;
            break;
        }

        stateTimer = 0.0f;
    }
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

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
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

// Check collision with specific area
bool Enemy::CheckCollisionWithTilesAt(float checkX, float checkY, MapManager* mapManager) {
    if (!mapManager || !mapManager->GetCurrentMap()) {
        return false;
    }

    SpatialGrid* grid = mapManager->GetCurrentMap()->GetSpatialGrid();
    if (!grid) {
        // Fallback to original method
        auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (CheckCollision(checkX, checkY, GetWidth(), GetHeight(),
                tile.posX, tile.posY, tile.width, tile.height)) {
                return true;
            }
        }
        return false;
    }

    // Use spatial grid optimization
    std::vector<MapTile*> nearbyTiles;
    grid->GetTilesInArea(
        checkX - 0.5f,
        checkY - 0.5f,
        GetWidth() + 1.0f,
        GetHeight() + 1.0f,
        nearbyTiles
    );

    for (const auto& tile : nearbyTiles) {
        if (tile->tileInfo.isSolid &&
            CheckCollision(checkX, checkY, GetWidth(), GetHeight(),
                tile->posX, tile->posY, tile->width, tile->height)) {
            return true;
        }
    }

    return false;
}

// Update collision detection, using spatial grid optimization
bool Enemy::CheckCollisionWithTiles(MapManager* mapManager) {
    if (!mapManager || !mapManager->GetCurrentMap()) {
        return false;
    }

    // Cache spatial grid pointer
    SpatialGrid* grid = mapManager->GetCurrentMap()->GetSpatialGrid();
    // Use spatial grid optimization
    std::vector<MapTile*> nearbyTiles;
    float padding = 0.5f;  // Slightly expand detection range
    grid->GetTilesInArea(
        posX - padding,
        posY - padding,
        width + padding * 2,
        height + padding * 2,
        nearbyTiles
    );

    for (const auto& tile : nearbyTiles) {
        if (tile->tileInfo.isSolid && CheckCollisionWithTile(*tile)) {
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

// Add to global texture definitions
ID3D11ShaderResourceView* g_bombEnemyTexture = nullptr;

// BombEnemy implementation
// Enemy.cpp
// BombEnemy implementation
BombEnemy::BombEnemy(float x, float y) : Enemy(x, y, 120.0f) {
    // Bomb enemy: 10x damage from top and bottom, reduced damage from other directions
    SetDamageMultiplier(DIR_UP, 10.0f);        // 10x damage from top
    SetDamageMultiplier(DIR_DOWN, 10.0f);      // 10x damage from bottom
    SetDamageMultiplier(DIR_FRONT, 0.7f);     // Reduced damage from front
    SetDamageMultiplier(DIR_BACK, 0.7f);      // Reduced damage from back
    SetDamageMultiplier(DIR_FRONT_UP, 1.2f);  // Medium from front-top
    SetDamageMultiplier(DIR_FRONT_DOWN, 1.2f);// Medium from front-bottom
    SetDamageMultiplier(DIR_BACK_UP, 1.2f);   // Medium from back-top
    SetDamageMultiplier(DIR_BACK_DOWN, 1.2f); // Medium from back-bottom

    // Bomb enemy properties
    width = PLAYER_WIDTH * 1.3f;
    height = PLAYER_HEIGHT * 1.3f;
    moveSpeed = 0.0f;  // Does not move
    pulseTimer = 0.0f;
    baseSize = 1.0f;

    // Set patrol range to 0, since it doesn't move
    patrolMinX = posX;
    patrolMaxX = posX;
}

// Override TakeDamage function, add explosion detection
void BombEnemy::TakeDamage(int damage, float attackAngle) {
    if (!isAlive) return;

    // Get damage multiplier
    float multiplier = GetDamageMultiplier(attackAngle);
    int actualDamage = (int)(damage * multiplier);

    // Use independent damage number manager
    bool isCritical = (multiplier > 1.5f);
    DamageNumberManager::AddDamageNumber(
        posX + width * 0.5f,  // Enemy center X
        posY + height,        // Top of enemy
        actualDamage,
        multiplier >= 10.0f  // If from top/bottom direction, show as critical
    );

    // Check if attack is from top or bottom
    float relativeAngle = GetRelativeAngle(attackAngle);
    int directionIndex = AngleToDirectionIndex(relativeAngle);
    bool isVerticalAttack = (directionIndex == DIR_UP || directionIndex == DIR_DOWN);

    // If attack is from top or bottom, die immediately and trigger explosion
    if (multiplier >= 10.0f) {
        health = 0;  // Die immediately
        isAlive = false;
        OnDeath();  // Trigger explosion
        return;     // Return directly, skip subsequent logic
    }

    // Non-vertical attacks, handle damage normally
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

void BombEnemy::Update(float deltaTime, MapManager* mapManager) {
    if (!isAlive) return;

    // Call base class hit state update
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    // Bomb enemy doesn't move, so no need to handle gravity and collisions
    velocityX = 0.0f;
    velocityY = 0.0f;

    // Pulsing effect
    pulseTimer += deltaTime;
    float pulseEffect = sin(pulseTimer * 3.0f) * 0.1f;
    baseSize = 1.0f + pulseEffect;

    // Simple AI: only detect player distance
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    // Update facing direction
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // Simple state machine
    if (distance < 2.0f) {
        currentState = ATTACK;  // Enter attack state when player is close
    }
    else {
        currentState = PATROL;  // Otherwise maintain patrol state (stationary)
    }

    // Trigger explosion when dying
    if (health <= 0 && isAlive) {
        isAlive = false;
        OnDeath();
    }
}

// Override OnDeath function, handle explosion effect
void BombEnemy::OnDeath() {
    // First call base class OnDeath
    Enemy::OnDeath();

    // Then trigger explosion effect
    Explode();
}

void BombEnemy::Render(ID3D11ShaderResourceView* texture, const Camera& camera) {
    if (!isAlive) return;

    // Convert to screen coordinates
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    // Select frame based on state and health
    int frameIndex = 0;
    if (health < maxHealth * 0.3f) {
        frameIndex = 1;  // Low health frame
    }
    if (currentState == ATTACK) {
        frameIndex = 2;  // Attack state frame
    }

    // Apply color effects
    if (isHit) {
        SetColor(1.0f, 0.0f, 0.0f, 1.0f);  // Hit red
    }
    else if (currentState == ATTACK) {
        // Pulsing color change in attack state
        float pulse = 0.5f + 0.5f * sin(pulseTimer * 5.0f);
        SetColor(1.0f, 0.3f + pulse * 0.5f, 0.3f, 1.0f);  // Red-orange pulse
    }
    else {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);  // Normal color
    }

    // Apply pulse scaling
    float renderWidth = width * baseSize;
    float renderHeight = height * baseSize;
    float offsetX = (width - renderWidth) * 0.5f;
    float offsetY = (height - renderHeight) * 0.5f;

    // Render enemy
    RenderImage(screenX + offsetX, screenY + offsetY, renderWidth, renderHeight,
        texture, frameIndex, 1, 3);

    // Render health bar
    RenderHealthBar(camera);

    // If in attack state, show warning effect
    if (currentState == ATTACK) {
        float warningSize = renderWidth * 1.5f;
        float warningX = screenX - (warningSize - renderWidth) * 0.5f;
        float warningY = screenY - (warningSize - renderHeight) * 0.5f;

        float alpha = 0.3f + 0.3f * sin(pulseTimer * 4.0f);
        SetColor(1.0f, 0.3f, 0.1f, alpha);
        RenderImage(warningX, warningY, warningSize, warningSize,
            g_groundTexture, 0, 1, 1);
    }

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void BombEnemy::Explode() {
    // Play explosion sound effect
    // PlaySound("explosion.wav");

    // Create explosion effect
    // CreateExplosionEffect(posX, posY);

    // Create projectiles to left and right
    CreateProjectiles();
}

// Enemy.cpp
void BombEnemy::CreateProjectiles() {
    // Get ProjectileManager instance
    ProjectileManager& projectileManager = ProjectileManager::GetInstance();

    // Create fireball effect configuration
    ProjectileEffect fireballEffect;
    fireballEffect.damage = 30.0f;  // Base damage
    fireballEffect.burnDamage = 5.0f;  // Burn over time damage
    fireballEffect.areaRadius = 0.3f;  // Explosion radius
    fireballEffect.pierce = false;  // No piercing

    float projectileSpeed = 3.0f;  // Projectile speed

    // Shoot fireball to the left
    projectileManager.CreateFireball(
        posX,  // Start X
        posY + height * 0.5f,  // Shoot from enemy center height
        posX - 10.0f,  // Far left position
        posY + height * 0.5f,  // Horizontal direction
        true  // From player
    );

    // Shoot fireball to the right
    projectileManager.CreateFireball(
        posX,  // Start X
        posY + height * 0.5f,  // Shoot from enemy center height
        posX + 10.0f,  // Far right position
        posY + height * 0.5f,  // Horizontal direction
        true  // From player
    );

    // Can add particle effects here
    // CreateParticleEffect(posX, posY, "explosion");
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

void UpdateEnemies(float deltaTime, MapManager* mapManager) {
    DamageNumberManager::Update(deltaTime);

    int visibleEnemyCount = 0;
    int totalEnemyCount = g_enemies.size();

    for (auto& enemy : g_enemies) {
        // Debug info: count visible enemies
        if (enemy->IsVisible(g_camera)) {
            visibleEnemyCount++;
        }

        enemy->Update(deltaTime, mapManager);
    }

    // Debug output (optional)
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    if (debugTimer > 2.0f) {
        char debugMsg[256];
        sprintf_s(debugMsg, "Enemy optimization: Total=%d, Visible=%d, Optimization rate=%.1f%%\n",
            totalEnemyCount, visibleEnemyCount,
            (1.0f - (float)visibleEnemyCount / totalEnemyCount) * 100.0f);
        OutputDebugStringA(debugMsg);
        debugTimer = 0.0f;
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