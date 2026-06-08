// Projectile.cpp
#include "Projectile.h"

// グローバル射弹マネージャーのインスタンス
ProjectileManager& ProjectileManager::GetInstance() {
    static ProjectileManager instance;
    return instance;
}

bool Projectile::CheckCollisionWithRect(float rectX, float rectY, float rectW, float rectH) const {
    if (!isActive) return false;

    // posX/posY は射弹の中心座標として保持されているので、そのまま使う。
    float visualSize = size * scaleEffect;
    float centerX = posX;
    float centerY = posY;

    float rectCenterX = rectX + rectW * 0.5f;
    float rectCenterY = rectY + rectH * 0.5f;

    float dx = centerX - rectCenterX;
    float dy = centerY - rectCenterY;
    float distance = sqrtf(dx * dx + dy * dy);

    float collisionRadius = (visualSize + std::min(rectW, rectH)) * 0.5f * 0.9f;
    return distance < collisionRadius;
}

void Projectile::OnHitByPlayer() {
    // 命中時の視覚・音響エフェクトを発生させる
    // 射弹位置に弱点ヒットエフェクトを生成する
    // 射弹ヒット用エフェクトは 0.75 倍で使う
    // posX/posY は射弹の中心を表す
    SpawnWeakPointHitEffectScaled(posX, posY, 0.75f);

    // 敵命中時より短いヒットストップと弱めのカメラシェイクにする
    g_player.hitStopTimer = 0.025f; // 通常より短い
    if (!g_player.isInvincible) {
        g_camera.Shake(0.01f, 0.03f);
    }

    // 射弹を無効化する
    isActive = false;
}

bool Projectile::IsHostileAndAimedAtPlayer() const {
    // 敵が撃った射弹だけを対象にする（プレイヤー弾は除外）
    if (fromPlayer) return false;

    // プレイヤーへの方向を計算する
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float projCenterX = posX + size * 0.5f;
    float projCenterY = posY + size * 0.5f;

    float dx = playerCenterX - projCenterX;
    float dy = playerCenterY - projCenterY;
    float len = sqrtf(dx * dx + dy * dy);
    if (len <= 0.001f) return true; // 非常に近いなら狙っているとみなす

    dx /= len; dy /= len;

    // 射弹の進行方向
    float pvlen = sqrtf(velocityX * velocityX + velocityY * velocityY);
    if (pvlen <= 0.001f) return false;
    float pvx = velocityX / pvlen;
    float pvy = velocityY / pvlen;

    // 内積: 射弹がだいたいプレイヤー方向へ向かっていれば dot > cos(60度)=0.5
    float dot = dx * pvx + dy * pvy;
    return dot > 0.5f;
}

void Projectile::SetRotation(float r) {
    rotation = r;
}
Projectile::Projectile(ProjectileType type, float startX, float startY,
    float targetX, float targetY, float speed,
    const ProjectileEffect& effect, bool fromPlayer)
    : type(type), posX(startX), posY(startY), speed(speed), effect(effect),
    fromPlayer(fromPlayer), isActive(true), homingTarget(nullptr),
    currentPierceCount(0), rotation(0.0f), scaleEffect(1.0f) {

    // 方向ベクトルを計算する
    float dx = targetX - startX;
    float dy = targetY - startY;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance > 0) {
        velocityX = (dx / distance) * speed;
        velocityY = (dy / distance) * speed;
    }
    else {
        velocityX = speed;
        velocityY = 0;
    }

    // 根据速度方向计算初始旋转角度
    rotation = CalculateDirectionAngle();

    // 对于普通子弹（BULLET），我们不希望它随方向旋转，使用固定旋转0
    if (type == ProjectileType::BULLET) {
        rotation = 0.0f;
    }

    // 種類ごとに初期プロパティを設定する
    switch (type) {
    case ProjectileType::FIREBALL:
        size = 0.08f;
        maxLifeTime = 3.0f;
        homingStrength = 0.0f;
        break;
    case ProjectileType::BULLET: 
        size = 0.15f;
        maxLifeTime = 6.0f;
        homingStrength = 0.0f;
        break;
    case ProjectileType::ICE_SHARD:
        size = 0.05f;
        maxLifeTime = 4.0f;
        homingStrength = 0.0f;
        break;
    case ProjectileType::MAGIC_MISSILE:
        size = 0.04f;
        maxLifeTime = 5.0f;
        homingStrength = 5.0f;
        break;
    case ProjectileType::LIGHTNING:
        size = 0.02f;
        maxLifeTime = 0.5f; // ライトニングは持続時間が非常に短い
        homingStrength = 0.0f;
        break;
    case ProjectileType::POISON_DART:
        size = 0.03f;
        maxLifeTime = 3.0f;
        homingStrength = 0.0f;
        break;
    case ProjectileType::HOLY_BOLT:
        size = 0.06f;
        maxLifeTime = 4.0f;
        homingStrength = 2.0f;
        break;
    }

    lifeTime = 0.0f;
}

void Projectile::Update(float deltaTime, MapManager* mapManager, std::vector<Enemy*>& enemies) {
    if (!isActive) return;

    lifeTime += deltaTime;

    // 生存時間を確認する
    if (lifeTime >= maxLifeTime) {
        isActive = false;
        CreateImpactEffect();
        return;
    }

    // 保存当前速度方向
    float oldVelocityX = velocityX;
    float oldVelocityY = velocityY;

    // 種類ごとの更新処理
    switch (type) {
    case ProjectileType::FIREBALL:
        UpdateFireball(deltaTime);
        break;
    case ProjectileType::BULLET:
        UpdateBullet(deltaTime);
        break;
    case ProjectileType::ICE_SHARD:
        UpdateIceShard(deltaTime);
        break;
    case ProjectileType::MAGIC_MISSILE:
        UpdateMagicMissile(deltaTime, enemies);
        break;
    case ProjectileType::LIGHTNING:
        UpdateLightning(deltaTime);
        break;
    case ProjectileType::POISON_DART:
        UpdatePoisonDart(deltaTime);
        break;
    case ProjectileType::HOLY_BOLT:
        UpdateHolyBolt(deltaTime);
        break;
    }

    // 如果速度方向发生变化，更新旋转角度
    if (fabs(velocityX - oldVelocityX) > 0.001f || fabs(velocityY - oldVelocityY) > 0.001f) {
        // 计算新的方向角度
        float newDirectionAngle = CalculateDirectionAngle();
        float oldDirectionAngle = atan2(oldVelocityY, oldVelocityX);

        // 只对非自转子弹类型更新基础方向
        switch (type) {
        case ProjectileType::LIGHTNING:
        case ProjectileType::HOLY_BOLT:
            // 这些类型没有自转，直接更新旋转角度
            rotation = newDirectionAngle;
            break;
        default:
            // 其他类型保持原有的自转逻辑
            break;
        }
    }

    // 移動と当たり判定
    Move(deltaTime);

    // マップとの衝突を確認する
    if (CheckMapCollision(mapManager)) {
        isActive = false;
        CreateImpactEffect();
        return;
    }
    CheckPlayerCollision();
    CheckEnemyCollision(enemies);
}
// プレイヤーとの衝突を確認する
void  Projectile::CheckPlayerCollision() {
    // プレイヤー弾でない場合のみプレイヤーとの衝突を確認する
    if (!isActive || fromPlayer) {
        return;
    }

    // プレイヤーの位置とサイズを取得する
    float playerX = g_player.posX;
    float playerY = g_player.posY;
    float playerWidth = PLAYER_WIDTH;
    float playerHeight = PLAYER_HEIGHT;

    // 考虑玩家冲刺时的碰撞体变化
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    if (g_player.isDashing) {
        playerWidth = PLAYER_WIDTH * 0.25f;
        playerHeight = PLAYER_HEIGHT * 0.25f;
        offsetX = (PLAYER_WIDTH - playerWidth) * 0.5f;
        offsetY = (PLAYER_HEIGHT - playerHeight) * 0.5f;
    }

    // 计算射弹的实际碰撞体大小（考虑缩放效果）
    float actualSize = size * scaleEffect;

    // 射弹中心点（posX/posY are center coordinates）
    float projectileCenterX = posX;
    float projectileCenterY = posY;

    // 玩家碰撞体中心点
    float playerCenterX = playerX + offsetX + playerWidth * 0.5f;
    float playerCenterY = playerY + offsetY + playerHeight * 0.5f;

    // 使用中心点距离检测碰撞（更准确）
    float dx = projectileCenterX - playerCenterX;
    float dy = projectileCenterY - playerCenterY;
    float distance = sqrt(dx * dx + dy * dy);
    
    // 碰撞半径
    // 敌人射弹判定略微缩小，避免贴图较大导致“擦边也算命中”。
    // 注意：这里只影响碰撞，不影响渲染。
    // 敵射弹の当たり判定をさらに縮小して、かすっただけの誤判定を減らす
    constexpr float ENEMY_PROJECTILE_HITBOX_SCALE = 0.80f;
    float collisionRadius = (actualSize + std::min(playerWidth, playerHeight)) * 0.5f * ENEMY_PROJECTILE_HITBOX_SCALE;

    if (distance < collisionRadius)
    {
        //if (!g_player.isDashing && !g_player.isDead) {
        //    // プレイヤーに効果を適用する
        //    OnPlayerDeath();
        //    isActive = false; // 射弹は命中後に消えるべき
        //}
        
        // 無敵中はプレイヤーが死亡しないようにする
        if (!g_player.isDashing && !g_player.isDead && !g_player.isInvincible) {
            // プレイヤーに効果を適用する
            OnPlayerDeath();
            isActive = false; // 射弹は命中後に消えるべき
        }
    }
}
void Projectile::UpdateFireball(float deltaTime) {
    // Fireball: 時間経過で大きくなり、加速する
    scaleEffect = 1.0f + lifeTime * 0.5f;
    speed += deltaTime * 2.0f;
    // 火球的自转效果保留
    rotation += deltaTime * 10.0f;
}

void Projectile::UpdateBullet(float deltaTime) {
    // Bullet: シンプルな射弹
    // BULLET の回転は無効化し、常に固定しておく
    rotation = 0.0f;
    // 拡縮効果なし - 常に同じ大きさを保つ
}

void Projectile::UpdateIceShard(float deltaTime) {
    // Ice Shard: 基础方向 + 自转效果
    rotation += deltaTime * 15.0f;

    // 氷の軌跡エフェクト
    if (fmod(lifeTime, 0.1f) < 0.05f) {
        // ここで氷のパーティクルを追加できる
    }
}

void Projectile::UpdateLightning(float deltaTime) {
    // Lightning: 基础方向 + 闪烁效果，不自转
    scaleEffect = 0.8f + 0.4f * sin(lifeTime * 30.0f);
    // 闪电不自转，旋转角度来自基础方向
}

void Projectile::UpdatePoisonDart(float deltaTime) {
    // Poison Dart: 基础方向 + 自转
    // 正弦波运动
    float waveOffset = sin(lifeTime * 10.0f) * 0.02f;
    posX += waveOffset * deltaTime * 10.0f;
    rotation += deltaTime * 20.0f;
}

void Projectile::UpdateHolyBolt(float deltaTime) {
    // Holy Bolt: 基础方向 + 脉冲效果，不自转
    scaleEffect = 1.0f + 0.2f * sin(lifeTime * 8.0f);
    // 圣光箭不自转，旋转角度来自基础方向
}
void Projectile::UpdateMagicMissile(float deltaTime, std::vector<Enemy*>& enemies) {
    // Magic Missile: 最も近い敵を追尾する
    if (homingTarget && !homingTarget->IsAlive()) {
        homingTarget = nullptr;
    }

    if (!homingTarget) {
        // 最も近い敵を探す
        float closestDistance = 1000.0f;
        for (auto& enemy : enemies) {
            if (enemy->IsAlive()) {
                // Get メソッドで敵の位置を取得する
                float enemyX = enemy->GetX();
                float enemyY = enemy->GetY();

                float dx = enemyX - posX;
                float dy = enemyY - posY;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance < closestDistance) {
                    closestDistance = distance;
                    homingTarget = enemy;
                }
            }
        }
    }

    // 以前の回転角度を保持する
    float oldDirectionAngle = CalculateDirectionAngle();

    // ターゲットへ追尾する
    if (homingTarget && homingStrength > 0) {
        float dx = homingTarget->GetX() - posX;
        float dy = homingTarget->GetY() - posY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance > 0) {
            // 徐々に向きを補正する
            float targetVX = (dx / distance) * speed;
            float targetVY = (dy / distance) * speed;

            velocityX += (targetVX - velocityX) * homingStrength * deltaTime;
            velocityY += (targetVY - velocityY) * homingStrength * deltaTime;

            // 速度を正規化する
            float currentSpeed = sqrt(velocityX * velocityX + velocityY * velocityY);
            velocityX = (velocityX / currentSpeed) * speed;
            velocityY = (velocityY / currentSpeed) * speed;

            // 计算新的方向角度
            float newDirectionAngle = CalculateDirectionAngle();

            // 回転角度を滑らかに補間する
            float angleDiff = newDirectionAngle - oldDirectionAngle;

            // 将角度差标准化到[-π, π]范围内
            while (angleDiff > 3.14159f) angleDiff -= 2 * 3.14159f;
            while (angleDiff < -3.14159f) angleDiff += 2 * 3.14159f;

            // 補間で回転遷移を滑らかにする
            rotation += angleDiff * 0.5f;
        }
    }
    else {
        // 追尾対象がいない場合は元の自転を維持する
        rotation += deltaTime * 8.0f;
    }
}
void Projectile::Move(float deltaTime) {
    posX += velocityX * deltaTime;
    posY += velocityY * deltaTime;
}

bool Projectile::CheckMapCollision(MapManager* mapManager) {
    if (!mapManager || !mapManager->GetCurrentMap()) return false;

    auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
    // 射弹の中心点が固体タイルに入った場合だけ衝突とみなす。
    // posX/posY は射弹の中心として保持されている。
    float visualSize = size * scaleEffect;
    float centerX = posX;
    float centerY = posY;

    for (const auto& tile : solidTiles) {
        // 中心点がタイル矩形内にあるか確認する
        if (centerX >= tile.posX && centerX <= tile.posX + tile.width &&
            centerY >= tile.posY && centerY <= tile.posY + tile.height) {
            return true;
        }
    }
    return false;
}

void Projectile::CheckEnemyCollision(std::vector<Enemy*>& enemies) {
    // プレイヤー弾の場合のみ敵との衝突を確認する
    if (!isActive || !fromPlayer) {
        return;
    }
    for (auto& enemy : enemies) {
        if (!enemy->IsAlive()) continue;

        // Get メソッドで敵の属性を取得する
        float enemyX = enemy->GetX();
        float enemyY = enemy->GetY();
        float enemyWidth = enemy->GetWidth();
        float enemyHeight = enemy->GetHeight();

        // 射弹を posX/posY 中心・scaleEffect 反映済みサイズの矩形として扱う
        float projW = size * scaleEffect;
        float projH = size * scaleEffect;
        float projLeft = posX - projW * 0.5f;
        float projTop = posY - projH * 0.5f;
        float projRight = projLeft + projW;
        float projBottom = projTop + projH;

        // 衝突判定（AABB）
        if (projLeft < enemyX + enemyWidth &&
            projRight > enemyX &&
            projTop < enemyY + enemyHeight &&
            projBottom > enemyY) {

            ApplyEffectToEnemy(enemy);

            if (!effect.pierce || currentPierceCount >= effect.maxPierceCount) {
                isActive = false;
                CreateImpactEffect();
                return;
            }
            else {
                currentPierceCount++;
            }
        }
    }
}


void Projectile::ApplyEffectToEnemy(Enemy* enemy) {
    if (!enemy || !enemy->IsAlive()) return;

    // 中心点を使って攻撃角度を計算する
    float enemyCenterX = enemy->GetX() + enemy->GetWidth() * 0.5f;
    float enemyCenterY = enemy->GetY() + enemy->GetHeight() * 0.5f;
    float dx = enemyCenterX - posX;
    float dy = enemyCenterY - posY;
    float attackAngle = atan2(dy, dx);

    // ダメージを適用する
    enemy->TakeDamage((int)effect.damage, attackAngle);

    // 特殊効果を適用する
    // TODO: 燃焼・減速・スタンなどの状態異常を追加する
    // Enemy クラス側に状態異常システムを追加する必要がある
}

void Projectile::CreateImpactEffect() {
    // TODO: 着弾エフェクトを作成する
    // ここにパーティクルや効果音などを追加できる
}


ID3D11ShaderResourceView* ProjectileManager::GetTextureForType(ProjectileType type) {
    switch (type) {
    case ProjectileType::FIREBALL: return fireballTexture;
    case ProjectileType::ICE_SHARD: return iceShardTexture;
    case ProjectileType::MAGIC_MISSILE: return magicMissileTexture;
    case ProjectileType::LIGHTNING: return lightningTexture;
    case ProjectileType::POISON_DART: return poisonDartTexture;
    case ProjectileType::HOLY_BOLT: return holyBoltTexture;
    case ProjectileType::BULLET: return bulletTexture;
    default: return fireballTexture;
    }
}

void Projectile::Render(const Camera& camera) {
    if (!isActive) return;

    // 対応するテクスチャを取得する
    ID3D11ShaderResourceView* texture = ProjectileManager::GetInstance().GetTextureForType(type);
    if (!texture) return;

    // 画面座標へ変換する（world -> screen）
    float screenX, screenY;
    float cameraX = camera.GetX();
    float cameraY = camera.GetY();
    screenX = posX - cameraX;
    screenY = posY - cameraY;

    // 種類に応じて色を設定する
    switch (type) {
    case ProjectileType::FIREBALL:
        SetColor(1.0f, 0.5f, 0.2f, 1.0f);
        break;
    case ProjectileType::BULLET: 
        SetColor(0.9f, 0.9f, 0.9f, 1.0f);
        break;
    case ProjectileType::ICE_SHARD:
        SetColor(0.6f, 0.8f, 1.0f, 1.0f);
        break;
    case ProjectileType::MAGIC_MISSILE:
        SetColor(0.8f, 0.3f, 0.9f, 1.0f);
        break;
    case ProjectileType::LIGHTNING:
        SetColor(0.9f, 0.9f, 0.2f, 1.0f);
        break;
    case ProjectileType::POISON_DART:
        SetColor(0.4f, 0.8f, 0.3f, 1.0f);
        break;
    case ProjectileType::HOLY_BOLT:
        SetColor(1.0f, 1.0f, 0.8f, 1.0f);
        break;
    }

    // 获取总旋转角度
    float totalRotation = GetRotationAngle();

    // 渲染射弹（带旋转和缩放）
    float renderSize = size * scaleEffect;

    // 默认使用正方形渲染（多数子弹）
    float renderWidth = renderSize;
    float renderHeight = renderSize;

    // 将绘制坐标调整为以中心为锚（便于旋转/翻转）
    float drawX = screenX - renderWidth * 0.5f;
    float drawY = screenY - renderHeight * 0.5f;

    // 对于普通子弹，禁用旋转并根据水平速度决定水平翻转
    bool flipHoriz = false;
    float drawRotation = totalRotation;
    if (type == ProjectileType::BULLET) {
        drawRotation = 0.0f; // 回転しないことを保証する
        // 左へ進んでいる場合はスプライトを左右反転する
        renderHeight = renderSize * 1.5f; // 弾スプライトを縦長にする
		renderWidth = renderSize * 1.0f;
        flipHoriz = (velocityX < 0.0f);
    }

    RenderImage(drawX, drawY, renderWidth, renderHeight, texture, 0, 1, 1, false, drawRotation, flipHoriz);

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}
// ProjectileManager クラスの実装
void ProjectileManager::AddProjectile(ProjectileType type, float startX, float startY,
    float targetX, float targetY, float speed,
    const ProjectileEffect& effect, bool fromPlayer) {
    projectiles.emplace_back(type, startX, startY, targetX, targetY, speed, effect, fromPlayer);
}

void ProjectileManager::Update(float deltaTime, MapManager* mapManager, std::vector<Enemy*>& enemies) {
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        it->Update(deltaTime, mapManager, enemies);

        if (!it->IsActive()) {
            it = projectiles.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ProjectileManager::HandlePlayerSlashHitRect(float rectX, float rectY, float rectW, float rectH) {
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        if (it->CheckCollisionWithRect(rectX, rectY, rectW, rectH)) {
            // 敵対的で、かつ実際にプレイヤーを狙っている射弹だけ斬れるようにする
            if (it->IsHostileAndAimedAtPlayer()) {
                it->OnHitByPlayer();
            }
        }

        if (!it->IsActive()) {
            it = projectiles.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ProjectileManager::HandlePlayerSlashHitCircle(float centerX, float centerY, float radius) {
    // 簡易判定のため円を矩形に変換する
    float rectX = centerX - radius;
    float rectY = centerY - radius;
    float rectW = radius * 2.0f;
    float rectH = radius * 2.0f;
    HandlePlayerSlashHitRect(rectX, rectY, rectW, rectH);
}

void ProjectileManager::Render(const Camera& camera) {
    for (auto& projectile : projectiles) {
        projectile.Render(camera);
    }
}

void ProjectileManager::ClearAll() {
    projectiles.clear();
}

void ProjectileManager::LoadTextures(ID3D11Device* device) {
    // 各種射弹テクスチャを読み込む
    LoadTexture(device, "asset/enemy/enemy_005_thorn/enemy_005_thorn_Pbullet_right.png", &fireballTexture);
    LoadTexture(device, "asset/enemy/enemy_003_fort/enemy_003_fort_bullet.png", &bulletTexture);
    LoadTexture(device, "asset/Projectile_IceShard.png", &iceShardTexture);

}

// 定義済み効果を使った射弹生成関数
void ProjectileManager::CreateFireball(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 100000.0f;
    effect.burnDamage = 5.0f;
    effect.areaRadius = 0.3f;

    AddProjectile(ProjectileType::FIREBALL, startX, startY, targetX, targetY, 1.0f, effect, fromPlayer);
}

void ProjectileManager::CreateBullet(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 100000.0f;
    effect.burnDamage = 5.0f;
    effect.areaRadius = 0.3f;

    AddProjectile(ProjectileType::BULLET, startX, startY, targetX, targetY, 0.2f, effect, fromPlayer);
}

void ProjectileManager::CreateIceShard(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 15.0f;
    effect.slowEffect = 0.5f;
    effect.stunDuration = 0.5f;

    AddProjectile(ProjectileType::ICE_SHARD, startX, startY, targetX, targetY, 10.0f, effect, fromPlayer);
}

void ProjectileManager::CreateMagicMissile(float startX, float startY, Enemy* target, bool fromPlayer) {
    if (!target) return;

    ProjectileEffect effect;
    effect.damage = 20.0f;
    effect.pierce = true;
    effect.maxPierceCount = 2;

    AddProjectile(ProjectileType::MAGIC_MISSILE, startX, startY,
        target->GetX(), target->GetY(), 6.0f, effect, fromPlayer);
}

void ProjectileManager::CreateLightningStrike(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 40.0f;
    effect.stunDuration = 1.0f;

    AddProjectile(ProjectileType::LIGHTNING, startX, startY, targetX, targetY, 20.0f, effect, fromPlayer);
}

void ProjectileManager::CreatePoisonDart(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 8.0f;
    effect.burnDamage = 3.0f; // Using burnDamage as poison over time damage

    AddProjectile(ProjectileType::POISON_DART, startX, startY, targetX, targetY, 12.0f, effect, fromPlayer);
}

void ProjectileManager::CreateHolyBolt(float startX, float startY, float targetX, float targetY, bool fromPlayer) {
    ProjectileEffect effect;
    effect.damage = 30.0f;
    effect.pierce = true;
    effect.maxPierceCount = 3;

    AddProjectile(ProjectileType::HOLY_BOLT, startX, startY, targetX, targetY, 9.0f, effect, fromPlayer);
}

// 在Projectile.cpp中添加以下方法实现

// 计算速度方向角度
float Projectile::CalculateDirectionAngle() const {
    // 计算速度方向的角度（弧度）
    return atan2(velocityY, velocityX);
}

// 获取旋转角度
float Projectile::GetRotationAngle() const {
    // 基础的方向角度
    float baseAngle = CalculateDirectionAngle();

    // 根据子弹类型调整旋转
    switch (type) {
    case ProjectileType::FIREBALL:
        // 火球：基础方向角度 + 自转角度
        return baseAngle;
    case ProjectileType::BULLET:
        // 普通子弹：禁用方向旋转，始终保持不变
        return 0.0f;
    case ProjectileType::ICE_SHARD:
        // 冰箭：基础方向角度 + 自转角度
        return baseAngle ;
    case ProjectileType::MAGIC_MISSILE:
        // 魔法飞弹：基础方向角度 + 自转角度
        return baseAngle + rotation;
    case ProjectileType::LIGHTNING:
        // 闪电：基础方向角度，加上随机的闪烁效果
        return baseAngle;
    case ProjectileType::POISON_DART:
        // 毒箭：基础方向角度 + 自转角度
        return baseAngle + rotation;
    case ProjectileType::HOLY_BOLT:
        // 圣光箭：基础方向角度
        return baseAngle;
    default:
        return baseAngle;
    }
}