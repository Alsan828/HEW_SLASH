// Enemy.cpp
#include "Enemy.h"
#include "Audio.h"
#include "Map.h"
#include "Projectile.h"

// ダメージ数字マネージャーを初期化する
std::vector<DamageNumber> DamageNumberManager::damageNumbers;

// 敵テクスチャを定義する
ID3D11ShaderResourceView* g_enemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_enemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_blindEyeEnemyIdleTexture = nullptr;

ID3D11ShaderResourceView* g_flyEnemyIdleTexture = nullptr;  // 飛行敵テクスチャ
ID3D11ShaderResourceView* g_flyEnemyDeathTexture = nullptr;  // 飛行敵の死亡テクスチャ

ID3D11ShaderResourceView* g_mageEnemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_mageEnemyAttackTexture = nullptr;
ID3D11ShaderResourceView* g_mageEnemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_fastEnemyRunTexture = nullptr;
ID3D11ShaderResourceView* g_fastEnemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_bombEnemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_bombEnemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_squareEnemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_squareEnemyDeathTexture = nullptr;

ID3D11ShaderResourceView* g_beamEnemyIdleTexture = nullptr;
ID3D11ShaderResourceView* g_beamEnemyPreAttackTexture = nullptr;
ID3D11ShaderResourceView* g_beamEnemyAttackTexture = nullptr;
ID3D11ShaderResourceView* g_beamEnemyPostAttackTexture = nullptr;
ID3D11ShaderResourceView* g_beamEnemyPreDeathTexture = nullptr;
ID3D11ShaderResourceView* g_beamEnemyDeathTexture = nullptr;
ID3D11ShaderResourceView* g_beamEnemyPostDeathTexture = nullptr;

// ボステクスチャは `Globals.cpp` で定義されている。

namespace {
    struct ThrownEnemyState {
        Enemy* enemy = nullptr;
        bool active = false;
        float vx = 0.0f;
        float vy = 0.0f;
    };

    static std::unordered_map<Enemy*, ThrownEnemyState> g_thrownEnemies;

    static void UpdateThrownEnemies(float deltaTime, MapManager* mapManager) {
        if (!mapManager || !mapManager->GetCurrentMap()) {
            return;
        }

        

        auto* grid = mapManager->GetCurrentMap()->GetSpatialGrid();
        auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();

        for (auto it = g_thrownEnemies.begin(); it != g_thrownEnemies.end();) {
            Enemy* e = it->first;
            ThrownEnemyState& s = it->second;

            if (!e || !e->IsAlive()) {
                it = g_thrownEnemies.erase(it);
                continue;
            }

            if (s.active) {
                it = g_thrownEnemies.erase(it);
                continue;
            }

            float posX = e->GetX();
            float posY = e->GetY();
            const float w = e->GetWidth();
            const float h = e->GetHeight();

            float oldX = posX;
            float oldY = posY;

            float gravityPerFrame = GRAVITY;
            s.vy += gravityPerFrame * deltaTime * 60.0f;

            posX += s.vx * deltaTime * 60.0f;
            posY += s.vy * deltaTime * 60.0f;

            bool landed = false;
            float newY = posY;

            if (grid) {
                std::vector<MapTile*> nearbyTiles;
                grid->GetTilesInArea(posX - 0.5f, posY - 0.5f, w + 1.0f, h + 1.0f, nearbyTiles);
                for (auto* tile : nearbyTiles) {
                    if (!tile || !tile->tileInfo.isSolid) continue;
                    if (CheckCollision(posX, posY, w, h, tile->posX, tile->posY, tile->width, tile->height)) {
                        if (s.vy < 0.0f) {
                            newY = tile->posY + tile->height;
                            landed = true;
                        }
                        break;
                    }
                }
            }
            else {
                for (const auto& tile : solidTiles) {
                    if (!tile.tileInfo.isSolid) continue;
                    if (CheckCollision(posX, posY, w, h, tile.posX, tile.posY, tile.width, tile.height)) {
                        if (s.vy < 0.0f) {
                            newY = tile.posY + tile.height;
                            landed = true;
                        }
                        break;
                    }
                }
            }

            e->SetPosition(posX, landed ? newY : posY);

            // 水平方向速度に基づく簡易向き更新
            if (fabs(s.vx) > 0.001f) {
                e->SetFacingRight(s.vx > 0.0f);
            }

            if (landed) {
                s.active = true;
                s.vx = 0.0f;
                s.vy = 0.0f;
                e->SetVelocity(0.0f, 0.0f);
                it = g_thrownEnemies.erase(it);
                continue;
            }

            ++it;
        }
    }
}

// InitEnemies 関数: 全テクスチャを読み込む
void InitEnemies() {
    // 敵テクスチャを読み込む
    // 通常敵
    LoadTexture(g_pDevice, "asset/enemy/enemy_001_eye/enemy_001_eye_idle.png", &g_enemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_001_eye/enemy_001_eye_death.png", &g_enemyDeathTexture);

    // 盲目通常敵（idle と通常アニメは同一画像、死亡は通常敵の死亡を流用）
    LoadTexture(g_pDevice, "asset/enemy/enemy_001_eye/blind_eye.png", &g_blindEyeEnemyIdleTexture);

    // 飛行敵
    LoadTexture(g_pDevice, "asset/enemy/enemy_004_wing/enemy_004_wing_right.png", &g_flyEnemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_004_wing/enemy_004_wing_death.png", &g_flyEnemyDeathTexture);

    // 魔法使い敵
    LoadTexture(g_pDevice, "asset/enemy/enemy_003_fort/enemy_003_fort_idle.png", &g_mageEnemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_003_fort/enemy_003_fort_attack.png", &g_mageEnemyAttackTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_003_fort/enemy_003_fort_death.png", &g_mageEnemyDeathTexture);

    // 高速敵
    LoadTexture(g_pDevice, "asset/enemy/enemy_002_ant/enemy_002_ant_right.png", &g_fastEnemyRunTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_002_ant/enemy_002_ant_death.png", &g_fastEnemyDeathTexture);

    // 爆弾敵
    LoadTexture(g_pDevice, "asset/enemy/enemy_005_thorn/enemy_005_thorn_idle.png", &g_bombEnemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_005_thorn/enemy_005_thorn_death.png", &g_bombEnemyDeathTexture);

    // Square enemy
    LoadTexture(g_pDevice, "asset/enemy/enemy_006_square/enemy_006_square.png", &g_squareEnemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_006_square/enemy_006_square_death.png", &g_squareEnemyDeathTexture);

    // Beam enemy
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_idle.png", &g_beamEnemyIdleTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_attack_before.png", &g_beamEnemyPreAttackTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_attack.png", &g_beamEnemyAttackTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_attack_after.png", &g_beamEnemyPostAttackTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_death_attack_before.png", &g_beamEnemyPreDeathTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_death_attack.png", &g_beamEnemyDeathTexture);
    LoadTexture(g_pDevice, "asset/enemy/enemy_007_beam/enemy_007_beam_death_attack_after.png", &g_beamEnemyPostDeathTexture);

    // ボス敵テクスチャ（必要に応じて自分のリソースパスへ差し替える）
    LoadTexture(g_pDevice, "asset/boss/boss_idle.png", &g_bossIdleTexture);
    LoadTexture(g_pDevice, "asset/boss/boss_attack.png", &g_bossAttackTexture);
    LoadTexture(g_pDevice, "asset/boss/boss_death.png", &g_bossDeathTexture);
    // ボスのチャージ用テクスチャを読み込む（各画像の正しいパスを指定する）
    LoadTexture(g_pDevice, "asset/boss/boss_charge_stage1.png", &g_bossChargeStage1Texture);
    LoadTexture(g_pDevice, "asset/boss/boss_charge_stage2.png", &g_bossChargeStage2Texture);
    // ボスのダッシュ用スプライト（2 フレーム）を読み込む
    LoadTexture(g_pDevice, "asset/boss/boss_dash.png", &g_bossDashTexture);
    LoadTexture(g_pDevice, "asset/boss/boss_dash_over.png", &g_bossDashOverTexture);
    LoadTexture(g_pDevice, "asset/boss/boss_slash_prep.png", &g_bossSlashPrepTexture);
    LoadTexture(g_pDevice, "asset/boss/boss_slash_active.png", &g_bossSlashActiveTexture);
    LoadTexture(g_pDevice, "asset/boss/boss_down_before.png", &g_bossDownBeforeTexture);
    LoadTexture(g_pDevice, "asset/boss/boss_down_hori.png", &g_bossDownHorizontalTexture);

    LoadTexture(g_pDevice, "asset/boss_final/boss_idle.png", &g_finalbossIdleTexture);
    LoadTexture(g_pDevice, "asset/boss_final/boss_attack.png", &g_finalbossAttackTexture);
    LoadTexture(g_pDevice, "asset/boss_final/boss_death.png", &g_finalbossDeathTexture);
    // 最終ボスのチャージ用テクスチャを読み込む（各画像の正しいパスを指定する）
    LoadTexture(g_pDevice, "asset/boss_final/boss_charge_stage1.png", &g_finalbossChargeStage1Texture);
    LoadTexture(g_pDevice, "asset/boss_final/boss_charge_stage2.png", &g_finalbossChargeStage2Texture);
    // 最終ボスのダッシュ用スプライト（2 フレーム）を読み込む
    LoadTexture(g_pDevice, "asset/boss_final/boss_dash.png", &g_finalbossDashTexture);
    LoadTexture(g_pDevice, "asset/boss_final/boss_dash_over.png", &g_finalbossDashOverTexture);
    LoadTexture(g_pDevice, "asset/boss_final/boss_slash_prep.png", &g_finalbossSlashPrepTexture);
    LoadTexture(g_pDevice, "asset/boss_final/boss_slash_active.png", &g_finalbossSlashActiveTexture);
    LoadTexture(g_pDevice, "asset/boss_final/boss_down_before.png", &g_finalbossDownBeforeTexture);
    LoadTexture(g_pDevice, "asset/boss_final/boss_down_hori.png", &g_finalbossDownHorizontalTexture);
}

// ========== BlindEyeEnemy ==========
BlindEyeEnemy::BlindEyeEnemy(float x, float y)
    : Enemy(x, y, 10.0f) {
    // 盲目敵は追跡しない。巡回処理内で自前で向きを変えるため、向き変更クールダウンは不要。
    useTurnCooldown = false;  
detectionRange = 0.0f;
    loseSightRange = 0.0f;

    anim.ClearClips();
    // idle / 通常: 同じ画像を使う（単フレーム / 単セル）
    anim.AddClip("idle", 0, 0, 1, 1, 0.1f, true, g_blindEyeEnemyIdleTexture);
    // 死亡アニメ: 通常敵の死亡を流用する
    anim.AddClip("death", 0, 4, 1, 5, 0.06f, false, g_enemyDeathTexture);
    anim.SetClip("idle");

    // 初期弱点: 正面からは 2 倍ダメージ
    SetDamageMultiplier(DIR_FRONT, 2.0f);

    // 少し遅めにして「普通の巡回敵」らしくする
    moveSpeed = MOVE_SPEED * 0.55f;
    patrolDirection = 1.0f;

    // テスト用に明確な弱点を用意する（上から当てると一撃）
    SetDamageMultiplier(DIR_UP, 100.0f);
}

void BlindEyeEnemy::Update(float deltaTime, MapManager* mapManager) {
    Enemy::Update(deltaTime, mapManager);
}

void BlindEyeEnemy::ChaseBehavior(float deltaTime) {
    // 追跡しない
    currentState = PATROL;
}

bool BlindEyeEnemy::IsGroundAhead(MapManager* mapManager, float directionSign) const {
    if (!mapManager || !mapManager->GetCurrentMap()) {
        return true;
    }

    // 足元の少し前方に小さな探査点を置き、地面が続いているか確認する
    const float aheadX = posX + (directionSign > 0.0f ? width : 0.0f) + directionSign * 0.02f;
    const float probeY = posY - 0.02f;
    const float probeW = 0.02f;
    const float probeH = 0.02f;

    SpatialGrid* grid = mapManager->GetCurrentMap()->GetSpatialGrid();
    if (!grid) {
        auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (!tile.tileInfo.isSolid) continue;
            if (CheckCollision(aheadX, probeY, probeW, probeH, tile.posX, tile.posY, tile.width, tile.height)) {
                return true;
            }
        }
        return false;
    }

    std::vector<MapTile*> nearbyTiles;
    grid->GetTilesInArea(aheadX - 0.1f, probeY - 0.1f, probeW + 0.2f, probeH + 0.2f, nearbyTiles);
    for (const auto* tile : nearbyTiles) {
        if (!tile || !tile->tileInfo.isSolid) continue;
        if (CheckCollision(aheadX, probeY, probeW, probeH, tile->posX, tile->posY, tile->width, tile->height)) {
            return true;
        }
    }
    return false;
}

void BlindEyeEnemy::PatrolBehavior(float deltaTime) {
    if (!isAlive || isDying) return;

    // 盲目敵は巡回のみ: 壁か崖に当たったら反転する
    MapManager* mapManager = &g_mapManager;
    const float dir = patrolDirection;

    // 1) 前方に地面があるか（崖判定）
    if (!IsGroundAhead(mapManager, dir)) {
        patrolDirection = -patrolDirection;
    }
    else {
        // 2) 壁判定: 予測位置に少しオフセットを置き、壁に当たるなら反転する
        float nextX = posX + dir * moveSpeed * 0.5f * deltaTime * 60.0f;
        if (CheckCollisionWithTilesAt(nextX, posY, mapManager)) {
            patrolDirection = -patrolDirection;
        }
    }

    velocityX = patrolDirection * moveSpeed * 1.0f;
    facingRight = (velocityX > 0.0f);
}

// ========== ThrowerEnemy ==========
ThrowerEnemy::ThrowerEnemy(float x, float y)
    : Enemy(x, y, 40.0f) {
    // 要望どおり、魔法使い / projectile 敵のテクスチャを再利用する。
    anim.ClearClips();
    anim.AddClip("idle", 0, 1, 1, 1, 0.1f, true, g_mageEnemyIdleTexture);
    anim.AddClip("death", 0, 4, 1, 5, 0.06f, false, g_mageEnemyDeathTexture);
    anim.SetClip("idle");

    moveSpeed = MOVE_SPEED * 0.4f;
    detectionRange = 10.0f;
    loseSightRange = 12.0f;
    throwCooldown = 5.0f;
    currentThrowCooldown = 0.75f;
    throwRange = 10.0f;
    // Larger flight time => slower projectile speed while keeping the same ballistic arc formula
    throwFlyTime = 0.9f;

    // 一撃弱点（上からの攻撃）
    SetDamageMultiplier(DIR_UP, 100.0f);
}

bool ThrowerEnemy::CanThrow() const {
    return currentThrowCooldown <= 0.0f;
}

void ThrowerEnemy::TryThrow(MapManager* mapManager) {
    if (!mapManager || !mapManager->GetCurrentMap()) return;
    if (!CanThrow()) return;

    float enemyCenterX = posX + width * 0.5f;
    float enemyCenterY = posY + height * 0.5f;
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dx = playerCenterX - enemyCenterX;
    float dy = playerCenterY - enemyCenterY;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist > throwRange) return;

    // Spawn a base enemy at thrower's position and launch it.
    Enemy* thrown = new FlyEnemy(posX, posY);
    g_enemies.push_back(thrown);

    float T = std::max(0.01f, throwFlyTime);
    float g = GRAVITY;

    // Update（vel * dt * 60）に合わせて、ゲーム単位 / 秒へ変換する
    // => 1 秒あたりの移動量は vel * 60 になる。
    float vx = dx / (T * 60.0f);
    float vy = (dy - 0.5f * g * (T * 60.0f) * (T * 60.0f)) / (T * 60.0f);

    g_thrownEnemies[thrown] = ThrownEnemyState{ thrown, false, vx, vy };

    facingRight = (dx >= 0.0f);
    currentThrowCooldown = throwCooldown;
}

void ThrowerEnemy::PatrolBehavior(float deltaTime) {
    patrolTimer += deltaTime;
    if (patrolTimer > 2.0f) {
        patrolTimer = 0.0f;
        patrolDirection = -patrolDirection;
    }
    velocityX = patrolDirection * moveSpeed;
    facingRight = (velocityX > 0.0f);
}

void ThrowerEnemy::ChaseBehavior(float deltaTime) {
    float dx = g_player.posX - posX;
    facingRight = (dx >= 0.0f);

    // 距離を少し保ち、強く追い過ぎないようにする。
    if (fabs(dx) > 1.5f) {
        velocityX = (dx > 0 ? 1.0f : -1.0f) * moveSpeed;
    }
    else {
        velocityX = 0.0f;
    }
}

void ThrowerEnemy::Update(float deltaTime, MapManager* mapManager) {
    if (!isAlive) {
        Enemy::Update(deltaTime, mapManager);
        return;
    }

    currentThrowCooldown -= deltaTime;

    // 投げた敵を先に更新し、このフレーム内で着地できるようにする。
    UpdateThrownEnemies(deltaTime, mapManager);

    Enemy::Update(deltaTime, mapManager);

    // 移動処理の後で投げることで、照準に現在位置を使えるようにする。
    if (currentState == CHASE) {
        TryThrow(mapManager);
    }
}

// Enemy クラス実装
Enemy::Enemy(float x, float y, float hp)
    : posX(x), posY(y), health(hp), maxHealth(hp), isAlive(true),
    currentState(PATROL), patrolMinX(-1.0f), patrolMaxX(1.0f), weakSpotDeath(false), attackRange(0.0f) {

    // 通常敵の当たり判定がスプライトより大きくなり過ぎないよう、縮小して中心を維持する
    const float oldWidth = PLAYER_WIDTH * 1.2f;
    const float oldHeight = PLAYER_HEIGHT * 1.2f;
    width = PLAYER_WIDTH * 1.0f;
    height = PLAYER_HEIGHT * 1.0f;
    posX += (oldWidth - width) * 0.5f;
    posY += (oldHeight - height) * 0.5f;
    moveSpeed = MOVE_SPEED * 0.65f;

    // 基底 Enemy 用の既定アニメーションクリップを追加する
    anim.AddClip("idle", 0, 1, 1, 1, 0.1f, true, g_enemyIdleTexture);
    anim.AddClip("death", 0, 4, 1, 5, 0.06f, false, g_enemyDeathTexture);

    anim.SetClip("idle");

    facingRight = true;  // 初期状態では右向き
    velocityX = 0.0f;
    velocityY = 0.0f;

    // 基本となる 8 方向ダメージ倍率を設定する
    SetDamageMultiplier(DIR_FRONT, 1.0f);
    SetDamageMultiplier(DIR_FRONT_UP, 1.0f);
    SetDamageMultiplier(DIR_UP, 1.0f);
    SetDamageMultiplier(DIR_BACK_UP, 1.0f);
    SetDamageMultiplier(DIR_BACK, 1.0f);
    SetDamageMultiplier(DIR_BACK_DOWN, 1.0f);
    SetDamageMultiplier(DIR_DOWN, 1.0f);
    SetDamageMultiplier(DIR_FRONT_DOWN, 1.0f);

    // AI パラメータを初期化する
    patrolDirection = 1.0f;
    patrolTimer = 0.0f;
    detectionRange = 3.0f;  // 検知範囲
    loseSightRange = 8.0f;  // 見失う範囲

    // 基本 / 通常敵は既定で向き変更クールダウンを使う
    useTurnCooldown = true;
}

void Enemy::SetDamageMultiplier(Direction dir, float multiplier) {
    if (dir >= DIR_FRONT && dir <= DIR_FRONT_DOWN) {
        // ダメージ倍率の下限を 1.0f にして、敵が減衰ダメージを受けないようにする
        if (multiplier < 1.0f) multiplier = 1.0f;
        damageMultipliers[static_cast<int>(dir)] = multiplier;
    }
}

float Enemy::GetDamageMultiplier(float attackAngle) {
    float relativeAngle = GetRelativeAngle(attackAngle);
    int directionIndex = AngleToDirectionIndex(relativeAngle);
    return damageMultipliers[directionIndex];
}

// 相対角度を計算する（敵の向き基準）
float Enemy::GetRelativeAngle(float attackAngle) const {
    // 右向きなら 0 度が正面、左向きなら 180 度が正面
    float enemyFrontAngle = facingRight ? 0.0f : 3.14159f;
    float relativeAngle = attackAngle - enemyFrontAngle;

    // [-π, π] に正規化する
    while (relativeAngle > 3.14159f) relativeAngle -= 2 * 3.14159f;
    while (relativeAngle < -3.14159f) relativeAngle += 2 * 3.14159f;

    return relativeAngle;
}

// 角度を方向インデックスへ変換する（8 方向）
int Enemy::AngleToDirectionIndex(float relativeAngle) {
    // 相対角度を [0, 2π] に正規化する
    float angle = relativeAngle;
    if (angle < 0) angle += 2 * 3.14159f;

    // 8 方向で各 45 度
    float sector = 3.14159f / 4.0f;

    // 方向インデックスを計算する
    int index = static_cast<int>((angle + sector / 2) / sector) % 8;
    return index;
}

float Enemy::NormalizeAngle(float angle) {
    while (angle < 0) angle += 2 * 3.14159f;
    while (angle >= 2 * 3.14159f) angle -= 2 * 3.14159f;
    return angle;
}

// 攻撃角度からダメージを計算する
int Enemy::CalculateDamageFromPlayer(int baseDamage, float playerDashAngle) {
    float multiplier = GetDamageMultiplier(playerDashAngle);
    return (int)(baseDamage * multiplier);
}

// TakeDamage 内で DamageNumberManager を使用する
void Enemy::TakeDamage(int damage, float attackAngle) {
    if (!isAlive) return;

    weakSpotDeath = false;

    float multiplier = GetDamageMultiplier(attackAngle);
    int actualDamage = (int)(damage * multiplier);

    // 独立したダメージ数字マネージャーを使う
    bool isCritical = (multiplier > 1.5f);
    DamageNumberManager::AddDamageNumber(
        posX + width * 0.5f,  // 敵の中心 X
        posY + height,        // 敵の上端
        actualDamage,
        isCritical
    );

    // 命中が成功したら必ずヒットエフェクトを出す。
    // 安定したヒット基準点を使う（ダメージ数字と同じで X=中心、Y=上端）。
    // 頭上ではなく胴体に見えるよう少し下へずらす。
    SpawnWeakPointHitEffect(posX + width * 0.5f, posY + height * 0.85f);

    if (isCritical) {
        // 弱点 / クリティカル命中時は少し強めに揺らす
        g_camera.Shake(0.08f, 0.6f);
    }
    else {
        // 通常ヒットでは控えめに揺らす
        g_camera.Shake(0.05f, 0.5f);
    }

    health -= actualDamage;
	Audio::PlaySE(SoundEffect::ENEMY_HIT);
    isHit = true;
    hitTimer = HIT_DURATION;
    OnHit(actualDamage);

    if (health <= 0) {
        health = 0;

        /*if (multiplier >= 2.0f) {
            weakSpotDeath = true;
        }*/
        if (multiplier > 1.5f) {  // >= 2.0f から > 1.5f に変更
            weakSpotDeath = true;
        }
        OnDeath();
    }

}

void Enemy::OnHit(int damage) {
    // 基本敵は被弾時に特別な処理を行わない
}

void Enemy::OnDeath() {
    if (isDying) return;  // 重複発火を防ぐ

    isAlive = false;
    isDying = true;
    // 死亡アニメへ確実に切り替える
    anim.SetClip("death");

    // アニメを先頭フレームへ戻す
    anim.Reset();
	Audio::PlaySE(SoundEffect::ENEMY_DEATH);

    // increments the player combo when enemy dies
    //g_player.comboCount++;
    //g_player.comboTimer = 5.0f; // it resets the timer

    OnEnemyDefeated(weakSpotDeath, posX + width * 0.5f, posY + height * 0.5f);
    // 後で削除
    char debugMsg[256];
    sprintf_s(debugMsg, "Total Enemy Points: %d\n", g_gameStats.GetTotalEnemyPoints());
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "Current Area Points: %d\n", g_gameStats.GetCurrentAreaEnemyPoints());
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "Total Kills: %d (Normal: %d, Weak: %d)\n",
        g_gameStats.GetEnemiesKilled() + g_gameStats.GetWeakPointKills(),
        g_gameStats.GetEnemiesKilled(),
        g_gameStats.GetWeakPointKills());
    OutputDebugStringA(debugMsg);
    OutputDebugStringA("===================\n\n");

    //OnEnemyDefeated();
    g_gameStats.UpdateMaxCombo(g_player.comboCount);
    // 後で削除
    
    sprintf_s(debugMsg, "=== ENEMY KILLED ===\n");
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "Current Combo: %d\n", g_player.comboCount);
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "Max Combo: %d\n", g_gameStats.GetMaxCombo());
    OutputDebugStringA(debugMsg);
    sprintf_s(debugMsg, "Weak Spot Kill: %s\n", weakSpotDeath ? "YES" : "NO");
    OutputDebugStringA(debugMsg);

  
}

void Enemy::Update(float deltaTime, MapManager* mapManager) {
    // 死亡状態を優先して処理する
    if (isDying) {
        anim.Update(deltaTime);  // 死亡アニメを確実に更新する

        // アニメの再生終了を確認する
        if (anim.IsFinished()) {
            markedForDeletion = true;
        }
        return;  // 死亡アニメ中は他の処理を行わない
    }

    if (!isAlive) {
        // すでに死亡していて死亡アニメが未開始なら開始する
        OnDeath();
        return;
    }

    // 可視性判定と最適化処理
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

    anim.Update(deltaTime);

    // 被弾状態の処理
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    // 重力を適用する
    velocityY += GRAVITY * deltaTime * 60.0f;

    // 衝突判定用に前フレーム位置を保存する
    float oldX = posX;
    float oldY = posY;

    // 水平移動
    posX += velocityX * deltaTime * 60.0f;
    if (CheckHorizontalCollision(mapManager, oldX, oldY)) {
        posX = oldX;
        velocityX = 0.0f;
    }

    // 垂直移動
    posY += velocityY * deltaTime * 60.0f;
    if (CheckVerticalCollision(mapManager, oldX, oldY)) {
        posY = oldY;
        velocityY = 0.0f;
    }

    // 境界チェック
    if (posY < -5.0f) {
        isAlive = false;
        OnDeath();  // 死亡アニメを開始する
        return;
    }

    // AI を更新する
    UpdateAI(deltaTime);

    if (!isCurrentlyVisible) {
        offScreenTimer += deltaTime;
    }
    else {
        offScreenTimer = 0.0f;
    }
}

// 簡易更新（更新が必要な画面外の敵向け）
void Enemy::UpdateMinimal(float deltaTime) {
    // 簡易 AI 更新（状態遷移のみ処理し、経路計算などは行わない）
    UpdateAIMinimal(deltaTime);

    // 画面外タイマーを更新する
    offScreenTimer += deltaTime;
}

// 簡易 AI 更新
void Enemy::UpdateAIMinimal(float deltaTime) {
    // 基本的な状態維持のみ行い、複雑な計算はしない
    float dx = g_player.posX - posX;

    // 向きを更新する
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // 簡易ステートマシン: タイムアウトや重要な状態遷移のみ処理する
    static float stateTimer = 0.0f;
    stateTimer += deltaTime;

    // 5 秒ごとに状態遷移を確認する（頻度を下げる）
    if (stateTimer >= 5.0f) {
        float distance = fabs(dx);

        // 状態遷移ロジックを簡略化する
        switch (currentState) {
        case PATROL:
            if (distance < detectionRange) currentState = CHASE;
            break;
        case CHASE:
            if (distance > loseSightRange) currentState = PATROL;
            break;
        }

        stateTimer = 0.0f;
    }
}

void Enemy::UpdateAI(float deltaTime) {
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    if (useTurnCooldown && turnCooldownTimer > 0.0f) {
        turnCooldownTimer -= deltaTime;
        if (turnCooldownTimer < 0.0f) turnCooldownTimer = 0.0f;
    }

    // 向きを更新する（通常敵はクールダウン制限あり。他は即時反転）
    if (dx != 0) {
        bool desiredFacingRight = (dx > 0);
        if (useTurnCooldown) {
            if (desiredFacingRight != facingRight && turnCooldownTimer <= 0.0f) {
                facingRight = desiredFacingRight;
                turnCooldownTimer = TURN_COOLDOWN_SECONDS;
            }
        }
        else {
            facingRight = desiredFacingRight;
        }
    }

    // ステートマシン処理
    switch (currentState) {
    case PATROL:
        PatrolBehavior(deltaTime);
        if (distance < detectionRange) {
            currentState = CHASE;
        }
        break;
    case CHASE:
        ChaseBehavior(deltaTime);
        if (distance > loseSightRange) {
            currentState = PATROL;
        }
        break;
    }
}

void Enemy::PatrolBehavior(float deltaTime) {
    patrolTimer += deltaTime;

    // 2 秒ごとに向きを変える必要があるか確認する
    if (patrolTimer >= 2.0f) {
        if (posX <= patrolMinX) {
            patrolDirection = 1.0f;  // 右へ進む
        }
        else if (posX >= patrolMaxX) {
            patrolDirection = -1.0f;  // 左へ進む
        }
        patrolTimer = 0.0f;
    }

    velocityX = patrolDirection * moveSpeed * 0.5f;

    // 完全停止を避けるため小さな垂直速度を加える
    if (velocityY == 0) {
        velocityY = 0.01f;
    }
}

void Enemy::ChaseBehavior(float deltaTime) {
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;  // 垂直方向も含めて計算する
    float distance = sqrt(dx * dx + dy * dy);  // 実際の距離を使う

    // 遠距離敵だけが攻撃範囲内で停止する
    if (attackRange > 0 && distance <= attackRange) {
        // 遠距離敵は攻撃範囲内で停止する
        velocityX = 0;
    }
    else {
        // 通常敵: 現在の向きのまま移動する（プレイヤー位置で移動方向は変えない）
        velocityX = (facingRight ? 1.0f : -1.0f) * moveSpeed;
    }
}

void Enemy::WorldToScreenPosition(float worldX, float worldY, float& screenX, float& screenY, const Camera& camera) {
    // カメラ位置（カメラ中心座標）を取得する
    float cameraX = camera.GetX();
    float cameraY = camera.GetY();

    // ワールド座標をスクリーン座標（相対座標）へ変換する
    // レンダリングシステムは画面中心を原点 (0,0) と仮定する
    screenX = worldX - cameraX;
    screenY = worldY - cameraY;
}

void Enemy::Render(ID3D11ShaderResourceView* texture, const Camera& camera) {
    if (!isAlive && !isDying) return;

    // tint が設定されていれば適用する（既定は白）
    SetColor(tintR, tintG, tintB, 1.0f);

    // ワールド座標をスクリーン座標へ変換する
    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    // 注: HP バーは敵の後で描画し、前面に見えるようにする

    // スプライトシートアニメ用の UV オフセットを取得する
    DirectX::XMFLOAT2 uvOffset = anim.GetUVOffset();

    // スケールをスプライトサイズへ反映する
    float renderWidth = width * scale;
    float renderHeight = height * scale;
    // 拡大したスプライトを当たり判定の中心に合わせる
    float offsetX = (renderWidth - width) * 0.5f;
    float offsetY = (renderHeight - height) * 0.5f;

    // 敵スプライトを描画する
    RenderImage(
        screenX - offsetX,
        screenY - offsetY,
        renderWidth,
        renderHeight,
        anim.GetCurrentClipTexture(),
        anim.GetCurrentFrame(),
        anim.GetSplitX(),
        anim.GetSplitY(),
        false,             // enableCulling
        0.0f,              // rotation
        !facingRight       // flipHorizontal: 座標系次第で !facingRight が適切か確認する
    );

    // Boss may use its own tint
    SetColor(tintR, tintG, tintB, 1.0f);

    // 死亡中でなければ HP バーを描画する（HP 10 以下の敵も含む）
    if (!isDying) {
        RenderHealthBar(camera);
    }
}

void BossEnemy::Render(ID3D11ShaderResourceView* texture, const Camera& camera) {
    if (!isAlive && !isDying) return;

    // 敵ごとの tint を適用する（BR 派生を SetTint 時に赤く描画できる）
    SetColor(tintR, tintG, tintB, 1.0f);

    float screenX, screenY;
    WorldToScreenPosition(posX, posY, screenX, screenY, camera);

    float renderWidth = width * scale;
    float renderHeight = height * scale;
    float offsetX = (renderWidth - width) * 0.5f;
    float offsetY = (renderHeight - height) * 0.5f;

    RenderImage(
        screenX - offsetX,
        screenY - offsetY,
        renderWidth,
        renderHeight,
        anim.GetCurrentClipTexture(),
        anim.GetCurrentFrame(),
        anim.GetSplitY(),
        anim.GetSplitX(),
        false,
        0.0f,
        facingRight
    );

    // ボス描画後に描画色を白へ戻す
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // ボスは HUD 固定の HP バーを使う（GameplayScene 側で描画）。
    // UI 重複を避けるため、頭上 HP バーは描画しない。
}

void Enemy::RenderHealthBar(const Camera& camera) {
    // follower 風の HP アイコン（asset/UI/Health.png の 1x3 シートを使用）
    // 表示するアイコン数を計算する: HP 10 ごとに 1 個（切り上げ）
    int icons = std::max(0, (int)std::ceil(maxHealth / 10.0f));
    if (icons <= 0) return;

    // healthFollowers ベクタのサイズを適正に保つ
    if ((int)healthFollowers.size() != icons) {
        healthFollowers.clear();
        healthFollowers.resize(icons);
    }

    // ワールド→スクリーン変換用の基準アンカー: 敵の上方
    float baseWorldX = posX + width * 0.5f;
    float baseWorldY = posY + height + 0.02f;

    // アイコンごとのレイアウト
    // アイコンサイズは固定し、敵幅に収めるため間隔だけを縮める
    const float iconW = width * 0.28f; // 敵幅に対する基準アイコンサイズ
    const float iconH = iconW;
    // 既定の間隔をかなり縮めて、インジケータ同士を近づける
    float spacing = iconW * 0.12f; // かなり小さい間隔
    const float minSpacing = width * 0.01f; // 絶対的な最小間隔

    // インジケータ全体の幅が敵幅を超えないようにする
    // 端がはみ出さないよう少し余白を残す
    const float margin = width * 0.05f;
    const float maxBarWidth = std::max(0.0f, width - margin * 2.0f);
    float totalWidth = icons * iconW + (icons - 1) * spacing;
    if (totalWidth > maxBarWidth) {
        if (icons > 1) {
            float availableForSpacing = maxBarWidth - icons * iconW;
            spacing = std::max(minSpacing, availableForSpacing / (icons - 1));
            if (spacing < 0.0f) spacing = 0.0f;
        } else {
            spacing = 0.0f;
        }
        totalWidth = icons * iconW + (icons - 1) * spacing;
    }

    // 補間には実際の delta time を使う
    const float dt = std::max(0.0f, g_gameTimer.GetDeltaTime());

    for (int i = 0; i < icons; ++i) {
        auto& hf = healthFollowers[i];

        // 横並びで敵の真上中央に置く目標位置
        float totalWidth = icons * iconW + (icons - 1) * spacing;
        float startX = baseWorldX - totalWidth * 0.5f;
        float targetX = startX + i * (iconW + spacing);
        float targetY = baseWorldY;

        // follower を目標位置へ直接合わせ、頭上に固定表示する
        hf.x = targetX;
        hf.y = targetY;
        hf.init = true;

        // スクリーン座標へ変換する
        float sx, sy;
        WorldToScreenPosition(hf.x, hf.y, sx, sy, camera);

        // 表示可否を判定する: 各アイコンは厳密に HP 10 を表し、health > i*10 のとき表示する
        float threshold = i * 10.0f;
        bool visible = (health > threshold + 0.0001f);

        float alpha = visible ? 1.0f : 0.35f;
        SetColor(1.0f, 1.0f, 1.0f, alpha);

        int totalFrames = g_healthAnim.GetSplitX() * g_healthAnim.GetSplitY();
        int frame = 0;
        if (totalFrames > 0) frame = g_healthAnim.GetCurrentFrame() % totalFrames;
        RenderImage(sx, sy, iconW, iconH, g_healthTexture, frame, g_healthAnim.GetSplitX(), g_healthAnim.GetSplitY(), false);
    }

    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

bool Enemy::CheckPlayerCollision() {
    // 基本的な矩形重なり判定
    bool overlap = CheckCollision(posX, posY, width, height,
        g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT);

    if (!overlap) return false;

    // 重なっていても、ダメージ衝突とみなすのは次のどちらかの場合だけ:
    //  - この敵種が接触ダメージを持ち（CanDamageOnContact()）、かつ
    //    プレイヤーがダッシュ中でない場合（ダッシュは攻撃扱い）、または
    //  - 敵が明示的な攻撃動作中である場合（IsCurrentlyAttacking()）。
    if (IsCurrentlyAttacking()) return true;

    if (CanDamageOnContact() && !g_player.isDashing) return true;

    return false;
}

// 特定領域との衝突を確認する
bool Enemy::CheckCollisionWithTilesAt(float checkX, float checkY, MapManager* mapManager) {
    if (!mapManager || !mapManager->GetCurrentMap()) {
        return false;
    }

    SpatialGrid* grid = mapManager->GetCurrentMap()->GetSpatialGrid();
    if (!grid) {
        // 元の方法へフォールバックする
        auto& solidTiles = mapManager->GetCurrentMap()->GetSolidTiles();
        for (const auto& tile : solidTiles) {
            if (CheckCollision(checkX, checkY, GetWidth(), GetHeight(),
                tile.posX, tile.posY, tile.width, tile.height)) {
                return true;
            }
        }
        return false;
    }

    // 空間グリッドで最適化する
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

// 衝突判定を更新し、空間グリッドで最適化する
bool Enemy::CheckCollisionWithTiles(MapManager* mapManager) {
    if (!mapManager || !mapManager->GetCurrentMap()) {
        return false;
    }

    // 空間グリッドへのポインタを保持する
    SpatialGrid* grid = mapManager->GetCurrentMap()->GetSpatialGrid();
    // 空間グリッドで最適化する
    std::vector<MapTile*> nearbyTiles;
    float padding = 0.5f;  // 判定範囲を少し広げる
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
    // 危険トゲは中央寄せした小さめの当たり判定（1/3 サイズ）を使う
    if (tile.tileInfo.type == std::string("hazard")) {
        float shrinkFactor = 1.0f / 3.0f;
        float hw = tile.width * shrinkFactor;
        float hh = tile.height * shrinkFactor;
        float hx = tile.posX + (tile.width - hw) * 0.5f;
        float hy = tile.posY + (tile.height - hh) * 0.5f;
        return CheckCollision(posX, posY, width, height, hx, hy, hw, hh);
    }
    return CheckCollision(posX, posY, width, height,
        tile.posX, tile.posY, tile.width, tile.height);
}

// FlyEnemy 実装 - 飛行敵で、重力の影響を受けない
FlyEnemy::FlyEnemy(float x, float y) : Enemy(x, y, 10.0f) {
    useTurnCooldown = false;
    // 飛行敵: 空中ユニット
	targetAltitude = y;
    attackRange = 0.0f;  // 近接敵
    SetDamageMultiplier(DIR_FRONT, 0.8f);
    SetDamageMultiplier(DIR_FRONT_UP, 0.8f);
    SetDamageMultiplier(DIR_FRONT_DOWN, 0.8f);
    SetDamageMultiplier(DIR_BACK, 1.5f);
    SetDamageMultiplier(DIR_BACK_UP, 1.5f);
    SetDamageMultiplier(DIR_BACK_DOWN, 1.5f);
    SetDamageMultiplier(DIR_UP, 1.2f);
    SetDamageMultiplier(DIR_DOWN, 1.2f);

    // アニメーションクリップを追加する
    anim.AddClip("idle", 0, 3, 1, 4, 0.15f, true, g_flyEnemyIdleTexture);
    anim.AddClip("death", 0, 3, 1, 4, 0.06f, false, g_flyEnemyDeathTexture);

    anim.SetClip("idle");
    width = PLAYER_WIDTH * 1.5f;
    moveSpeed = MOVE_SPEED * 0.8f;

    // 飛行敵専用パラメータ
    patrolMinX = x - 2.0f;  // 巡回範囲を広げる
    patrolMaxX = x + 2.0f;
    detectionRange = 5.0f;  // より広い検知距離
    patrolAltitude = y;  // 巡回高度
    altitudeChangeTimer = 0.0f;
    altitudeChangeRate = 0.05f;  // 高度変化速度

    scale = 3.0f;

    // 一撃テスト用に、明確な弱点方向（背面）を用意する
    SetDamageMultiplier(DIR_BACK, 100.0f);
}


void FlyEnemy::PatrolBehavior(float deltaTime) {
    patrolTimer += deltaTime;
    altitudeChangeTimer += deltaTime;

    // 2 秒ごとに水平方向変更が必要か確認する
    if (patrolTimer >= 2.0f) {
        if (posX <= patrolMinX) {
            patrolDirection = 1.0f;  // 右へ進む
        }
        else if (posX >= patrolMaxX) {
            patrolDirection = -1.0f;  // 左へ進む
        }
        patrolTimer = 0.0f;
    }

    // 垂直方向の浮遊演出
    float altitudeVariation = sin(altitudeChangeTimer * 2.0f) * 0.1f;
    targetAltitude = patrolAltitude + altitudeVariation;

    // 目標高度へ滑らかに移動する
    if (fabs(posY - targetAltitude) > 0.01f) {
        if (posY < targetAltitude) {
            velocityY = altitudeChangeRate;
        }
        else {
            velocityY = -altitudeChangeRate;
        }
    }
    else {
        velocityY = 0.0f;
    }

    velocityX = patrolDirection * moveSpeed * 0.3f;  // 巡回中は遅め
}


void FlyEnemy::ChaseBehavior(float deltaTime) {
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    // プレイヤーが検知範囲内なら直接向かう
    if (distance > 0.1f) {
        // 方向ベクトルを正規化する
        dx /= distance;
        dy /= distance;

        // 地形を無視して直接プレイヤーへ向かう
        velocityX = dx * moveSpeed;
        velocityY = dy * moveSpeed;
    }
    else {
        // 接近したら少し減速する
        velocityX *= 0.5f;
        velocityY *= 0.5f;
    }
}

void FlyEnemy::OnHit(int damage) {
    // 飛行敵は被弾時に短い硬直が入る
    velocityX *= 0.5f;
    velocityY = 0.0f;
}

void FlyEnemy::OnDeath() {
    Enemy::OnDeath();
    // 飛行敵死亡時に特殊演出を入れられる
}

// MageEnemy 実装
MageEnemy::MageEnemy(float x, float y) : Enemy(x, y, 10.0f) {
    useTurnCooldown = false;
    // 魔法使い敵: 上下が弱点（一撃必殺）
    SetDamageMultiplier(DIR_UP, 100.0f);
    SetDamageMultiplier(DIR_DOWN, 100.0f);
    SetDamageMultiplier(DIR_FRONT, 0.7f);
    SetDamageMultiplier(DIR_BACK, 0.7f);

    spellCooldown = 3.0f;
    currentSpellCooldown = 0.0f;
    detectionRange = 4.0f;  // より広い検知距離
    attackRange = 2.5f;  // 射撃攻撃範囲
    moveSpeed = MOVE_SPEED * 0.4f;

    // アニメーションクリップを追加する
    anim.AddClip("idle", 0, 1, 1, 2, 0.2f, true, g_mageEnemyIdleTexture);
    anim.AddClip("death", 0, 3, 1, 4, 0.06f, false, g_mageEnemyDeathTexture); // 敵撃破時の死亡アニメ用
    anim.SetClip("idle");


    // 射弾関連パラメータ
    projectileSpeed = 2.0f;
    lastAttackTime = 0.0f;
    attackCooldown = 1.5f;  // 攻撃クールダウン
    projectileDamage = 20.0f;

    scale = 3.0f;

    // 一撃テスト用に弱点方向を明示する
    SetDamageMultiplier(DIR_BACK, 100.0f);
}

void MageEnemy::Update(float deltaTime, MapManager* mapManager) {
    // 死亡アニメ中は新しい射弾を生成しない
    if (isDying) {
        anim.Update(deltaTime);
        if (anim.IsFinished()) {
            markedForDeletion = true;
        }
        return;
    }

    Enemy::Update(deltaTime, mapManager);

    // Enemy::Update は死亡時や画面外の簡易更新時に早期 return する可能性があるため、
    // ここでもう一度確認し、その状態で射弾が出ないことを保証する。
    if (!isAlive) {
        return;
    }

    lastAttackTime += deltaTime;

    // 追跡状態で射弾を発射する
    if (currentState == CHASE && lastAttackTime >= attackCooldown) {
        CastProjectile();
        lastAttackTime = 0.0f;
    }
}

void MageEnemy::ChaseBehavior(float deltaTime) {
    float dx = g_player.posX - posX;
    float distance = fabs(dx);

    // 魔法使いは追跡時に距離を保つ
    if (distance > attackRange) {
        if (dx > 0) {
            velocityX = moveSpeed * 0.5f;  // ゆっくり移動する
        }
        else {
            velocityX = -moveSpeed * 0.5f;
        }
    }
    else {
        velocityX = 0;  // 攻撃距離内では停止する
    }
}

void MageEnemy::CastProjectile() {
    // 死亡していたら
    if (!isAlive) return;

    // プレイヤー中心を狙い、敵の体中心から発射する
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dx = playerCenterX - (posX + width * 0.5f);
    float dy = playerCenterY - (posY + height * 0.5f);
    float distance = sqrt(dx * dx + dy * dy);

    if (distance > 0.1f) {
        // ProjectileManager インスタンスを取得する
        ProjectileManager& projectileManager = ProjectileManager::GetInstance();

        // 魔法射弾の設定を作成する
        ProjectileEffect magicEffect;
        magicEffect.damage = projectileDamage;
        magicEffect.areaRadius = 0.2f;
        magicEffect.pierce = false;

        // 射弾の目標位置をプレイヤー中心にする
        float targetX = playerCenterX;
        float targetY = playerCenterY;

        // 魔法射弾を発射する: 敵の体中心から撃つ
        projectileManager.CreateBullet(
            posX + width * 0.5f,  // 体の中心から発射
            posY + height * 0.5f,  // 敵中心の Y 座標
            targetX,
            targetY,
            false
        );

        // 攻撃アニメを再生する
        //PlayAnimation("attack");
    }
}

// FastEnemy 実装
FastEnemy::FastEnemy(float x, float y) : Enemy(x, y, 10.0f) {
    useTurnCooldown = false;
    moveSpeed = MOVE_SPEED * 1.5f;
    dashCooldown = 2.0f;
    currentDashCooldown = 0.0f;
    detectionRange = 4.0f;
    attackRange = 0.5f;

    attackRange = 0.0f;  // 近接敵
    anim.AddClip("run", 0, 3, 1, 4, 0.05f, true, g_fastEnemyRunTexture);
    anim.AddClip("death", 0, 3, 1, 4, 0.06f, false, g_fastEnemyDeathTexture);
    anim.SetClip("run");

    scale = 3.0f;

    // テスト用に上方向からの一撃弱点を設定する
    SetDamageMultiplier(DIR_UP, 100.0f);
}

// この関数実装を追加する
void FastEnemy::Update(float deltaTime, MapManager* mapManager) {
    Enemy::Update(deltaTime, mapManager);

    if (currentDashCooldown > 0) {
        currentDashCooldown -= deltaTime;
    }

    if (currentState == CHASE && currentDashCooldown <= 0) {
        DashAttack();
        currentDashCooldown = dashCooldown;
    }
}

void FlyEnemy::Update(float deltaTime, MapManager* mapManager) {  
    if (isDying) {
        anim.Update(deltaTime);  // 死亡アニメを確実に更新する

        // アニメの再生終了を確認する
        if (anim.IsFinished()) {
            markedForDeletion = true;
        }
        return;  // 死亡アニメ中は他の処理を行わない
    }

    if (!isAlive) {
        // すでに死亡していて死亡アニメが未開始なら開始する
        OnDeath();
        return;
    }

    // 可視性判定と最適化処理
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

    anim.Update(deltaTime);

    // 被弾状態の処理
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    // AI を更新する
    UpdateAI(deltaTime);

    // 水平移動を適用する
    posX += velocityX * deltaTime * 60.0f;

    // 飛行敵の垂直移動（浮遊演出）
    posY += velocityY * deltaTime * 60.0f;

    // 境界チェック（ワールド外へ飛び出すのを防ぐ）
    if (posY < -50.0f) {
        isAlive = false;
        return;
    }

    if (!isCurrentlyVisible) {
        offScreenTimer += deltaTime;
    }
    else {
        offScreenTimer = 0.0f;
    }
}

void FastEnemy::ChaseBehavior(float deltaTime) {
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);  // 実際の距離を使う

    // 高速敵は常にプレイヤーへ向かう
    if (dx > 0) {
        velocityX = moveSpeed;
    }
    else {
        velocityX = -moveSpeed;
    }

    // ダッシュ攻撃
    if (currentDashCooldown <= 0 && distance < 2.0f) {  // ダッシュ距離しきい値
        DashAttack();
        currentDashCooldown = dashCooldown;
    }
}

void FastEnemy::DashAttack() {
    // 高速敵が前方へダッシュする
    velocityX = (g_player.posX > posX ? 1.0f : -1.0f) * moveSpeed * 2.5f;
}

// BombEnemy 実装
BombEnemy::BombEnemy(float x, float y) : Enemy(x, y, 30.0f) {
    useTurnCooldown = false;
    // 爆弾敵: 上下が弱点（一撃必殺）、他方向はダメージ軽減
    SetDamageMultiplier(DIR_UP, 100.0f);
    SetDamageMultiplier(DIR_DOWN, 100.0f);
    SetDamageMultiplier(DIR_FRONT, 0.7f);
    SetDamageMultiplier(DIR_BACK, 0.7f);
    SetDamageMultiplier(DIR_FRONT_UP, 1.2f);
    SetDamageMultiplier(DIR_FRONT_DOWN, 1.2f);
    SetDamageMultiplier(DIR_BACK_UP, 1.2f);
    SetDamageMultiplier(DIR_BACK_DOWN, 1.2f);

    width = PLAYER_WIDTH * 1.3f;
    height = PLAYER_HEIGHT * 1.3f;
    moveSpeed = 0.0f;  // 移動しない
    detectionRange = 2.0f;

    anim.AddClip("idle", 0, 0, 1, 1, 0.3f, true, g_bombEnemyIdleTexture);
    anim.AddClip("death", 0, 3, 1, 4, 0.06f, false, g_bombEnemyDeathTexture);

    anim.SetClip("idle");

    pulseTimer = 0.0f;
    baseSize = 1.0f;
    explosionRadius = 1.5f;
    explosionDamage = 50.0f;

    scale = 3.0f;
}

// TakeDamage をオーバーライドし、爆発判定を追加する
void BombEnemy::TakeDamage(int damage, float attackAngle) {
    if (!isAlive) return;

    // ダメージ倍率を取得する
    float multiplier = GetDamageMultiplier(attackAngle);
    int actualDamage = (int)(damage * multiplier);

    // 独立したダメージ数字マネージャーを使う
    bool isCritical = (multiplier > 1.5f);
    DamageNumberManager::AddDamageNumber(
        posX + width * 0.5f,  // 敵の中心 X
        posY + height,        // 敵の上端
        actualDamage,
        multiplier >= 10.0f  // 上下方向からならクリティカル表示
    );

    // 上下からの攻撃かどうかを確認する
    float relativeAngle = GetRelativeAngle(attackAngle);
    int directionIndex = AngleToDirectionIndex(relativeAngle);
    bool isVerticalAttack = (directionIndex == DIR_UP || directionIndex == DIR_DOWN);

    // 上下からの攻撃なら即死して爆発を発生させる
    if (multiplier >= 10.0f) {
        health = 0;  // 即死させる
        // 撃破報酬や演出が有効になるよう、弱点撃破として記録する
        weakSpotDeath = true;
        isAlive = false;
        OnDeath();  // 爆発を発生させる
        return;     // 後続処理を飛ばして終了する
    }

    // 致命でも垂直ヒットでもない場合は通常の被弾演出を出す
    SpawnWeakPointHitEffect(posX + width * 0.5f, posY + height * 0.85f);
    Audio::PlaySE(SoundEffect::ENEMY_HIT);
    g_camera.Shake(0.05f, 0.5f);

    // 垂直以外の攻撃は通常どおりダメージ処理する
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

void BombEnemy::Update(float deltaTime, MapManager* mapManager) {    // 死亡状態を優先して処理する
    if (isDying) {
        anim.Update(deltaTime);  // 死亡アニメを確実に更新する

        // アニメの再生終了を確認する
        if (anim.IsFinished()) {
            markedForDeletion = true;
        }
        return;  // 死亡アニメ中は他の処理を行わない
    }

    if (!isAlive) {
        // すでに死亡していて死亡アニメが未開始なら開始する
        OnDeath();
        return;
    }
    // 基底クラスの被弾状態更新を呼ぶ
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    // 脈動エフェクト
    pulseTimer += deltaTime;
    float pulseEffect = sin(pulseTimer * 3.0f) * 0.1f;
    baseSize = 1.0f + pulseEffect;

    // 簡単な AI: プレイヤー距離だけを見る
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    // 向きを更新する
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // 簡易ステートマシン
    if (distance < detectionRange) {
        currentState = CHASE;  // プレイヤー接近時は追跡状態へ入る
    }
    else {
        currentState = PATROL;  // それ以外は巡回状態（静止）を保つ
    }

    // 死亡時に爆発を発生させる
    if (health <= 0 && isAlive) {
        isAlive = false;
        OnDeath();
    }
}

void BombEnemy::ChaseBehavior(float deltaTime) {
    // 追跡状態でプレイヤーが爆発範囲内なら自爆する
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < explosionRadius) {
        OnDeath();  // 爆発を発生させる
    }
}

void BombEnemy::OnDeath() {
    // 先に基底クラスの OnDeath を呼ぶ
    Explode();
    Enemy::OnDeath();
}

void BombEnemy::Explode() {
    // 爆発音を再生する
    // PlaySound("explosion.wav");

    // 爆発エフェクトを作成する
    // CreateExplosionEffect(posX, posY);

    // 左右へ射弾を発射する
    CreateProjectiles();
}

// 敵が 8 方向へ射弾を投げるようにする
void BombEnemy::CreateProjectiles() {
    // ProjectileManager インスタンスを取得する
    ProjectileManager& projectileManager = ProjectileManager::GetInstance();

    float distance = 10.0f;  // 目標までの距離

    // 敵の中心位置を計算する
    float centerX = posX + width * 0.5f;
    float centerY = posY + height * 0.3f;  // 敵の高さ 30% 地点から発射する（元は 0.5f）

    // 8 方向パターン用
    for (int i = 0; i < 8; i++) {
        float targetX = centerX + EIGHT_DIRECTIONS[i].x * distance;
        float targetY = centerY + EIGHT_DIRECTIONS[i].y * distance;

        projectileManager.CreateFireball(
            centerX,
            centerY,
            targetX,
            targetY,
            true  // プレイヤーへはダメージを与えない
        );
    }
}


BossEnemy::BossEnemy(float x, float y) : Enemy(x, y, 150.0f)
{
    useTurnCooldown = false;
    SetMaxHealth(150.0f); 
    SetHealth(150.0f);

    // ボスは当たり判定もスプライトも 3 倍サイズ
    // Enemy(x,y,...) で基準サイズは設定済みなので、中心を保ったまま拡大する。
    const float oldW = width;
    const float oldH = height;

    scale = 3.0f;
    width = oldW * 3.0f;
    height = oldH * 3.0f;
    posX -= (width - oldW) * 0.5f;
    posY -= (height - oldH) * 0.5f;

    moveSpeed = MOVE_SPEED * 0.3f;

    anim.ClearClips();
    // idle: 4 列 1 行。必要に応じて調整する
    anim.AddClip("idle",   0, 3, 4, 1, 0.12f, true,  g_bossIdleTexture);

    // dash: 2 フレーム（画像に合わせる）。4 回繰り返し再生用
	anim.AddClip("dash", 0, 1, 2, 1, 0.05f, true, g_bossDashTexture);
    // dash over: 1x4。再生後に次段階へ戻る
	anim.AddClip("dash_over", 0, 3, 4, 1, 0.06f, false, g_bossDashOverTexture);

    // チャージ / 溜め（従来仕様を維持）
    anim.AddClip("charge_stage1", 0, 3, 4, 1, 0.10f, true, g_bossChargeStage1Texture);
    anim.AddClip("charge_stage2", 0, 2, 3, 1, 0.06f, false, g_bossChargeStage2Texture);

    // 追加: slash の準備と発動。提供された 2 枚の画像を使う
    // 画像 5: 4 フレームを右から左へ再生 => start=3, end=0, splitX=4, splitY=1
    anim.AddClip("slash_prep",   3, 0, 4, 1, 0.06f, true, g_bossSlashPrepTexture);
    // 画像 6: 8 フレームを右から左へ再生 => start=7, end=0, splitX=8, splitY=1
    // 速度を 0.5 倍に落とす（フレーム時間を 2 倍にする）
    anim.AddClip("slash_active", 0, 7, 8, 1, slashFrameTime, false, g_bossSlashActiveTexture);

    // down_before から down へ
    anim.AddClip("down_before", 0, 4, 5, 1, 0.06f, false, g_bossDownBeforeTexture);
    anim.AddClip("down_hori", 0, 0, 1, 1, 0.1f, false, g_bossDownHorizontalTexture);

    // death: 5 フレームの例
    anim.AddClip("death",  0, 14, 15, 1, 0.06f, false, g_bossDeathTexture);

    anim.SetClip("idle");
}

// ボスは単純な接触だけではダメージを与えないようにする
bool BossEnemy::CanDamageOnContact() const {
    return false;
}

void BossEnemy::Update(float deltaTime, MapManager* mapManager)
{
    // 死亡状態を処理する
    if (isDying) {
        anim.Update(deltaTime);
        if (anim.IsFinished()) {
            markedForDeletion = true;
        }
        return;
    }

    if (!isAlive) {
        OnDeath();
        return;
    }

    if (bossFreezeTimer > 0.0f) {
        bossFreezeTimer -= deltaTime;

        // idleアニメを維持し、行動・攻撃を一切行わない
        if (anim.GetCurrentClipName() != "idle") {
            anim.SetClip("idle");
        }
        velocityX = 0.0f;

        // 重力と地形衝突だけは通常どおり適用する（宙に浮かないように）
        velocityY += GRAVITY * deltaTime * 60.0f;

        float oldX = posX;
        float oldY = posY;

        posX += velocityX * deltaTime * 60.0f;
        if (CheckHorizontalCollision(mapManager, oldX, oldY)) {
            posX = oldX;
            velocityX = 0.0f;
        }

        posY += velocityY * deltaTime * 60.0f;
        if (CheckVerticalCollision(mapManager, oldX, oldY)) {
            posY = oldY;
            velocityY = 0.0f;
        }

        anim.Update(deltaTime);
        return;  // ステートマシン・攻撃処理をすべてスキップ
    }

    // フェーズ変化の例
    float healthPercent = health / maxHealth;
    if (healthPercent < 0.3f && phase == 1) {
        phase = 2;
        moveSpeed *= 1.5f;
    }

    // 解放中に固定されていない限り、プレイヤー方向へ向きを更新する
    if (!facingLocked) {
        float dxFace = g_player.posX - posX;
        if (dxFace != 0) facingRight = (dxFace > 0);
    }

    // ステートマシン
    stateTimer += deltaTime;
    switch (bossState) {
    case BOSS_IDLE:
        // Choose between dash or slash only (leap disabled)
        if (stateTimer >= idleDuration) {
            int r = rand() % 2;
            if (r == 0) EnterState(BOSS_DASH_CHARGE);
            else EnterState(BOSS_SLASH_CHARGE);
        }
        break;
    case BOSS_DASH_CHARGE:
        UpdateDashCharge(deltaTime);
        break;
    case BOSS_DASH_MOVING:
        UpdateDashMoving(deltaTime, mapManager);
        break;
    case BOSS_DASH_AFTER:
        UpdateDashAfter(deltaTime);
        break;
    case BOSS_SLASH_CHARGE:
        UpdateSlashCharge(deltaTime);
        break;
    case BOSS_SLASH_ACTIVE:
        UpdateSlashActive(deltaTime);
        break;
    case BOSS_DOWN_BEFORE:
        UpdateDownBefore(deltaTime);
        break;
    case BOSS_DOWN:
        UpdateDown(deltaTime);
        break;
    case BOSS_DOWN_AFTER:
        UpdateDownAfter(deltaTime);
        break;
    }

    // leap 移動中の運動と重力は UpdateLeapMoving で処理される。
    // ここで基底移動を重ねると二重積分となり、消失の原因になるため避ける。
    // それ以外は基底 Enemy と同様に重力と移動を処理する
    velocityY += GRAVITY * deltaTime * 60.0f;

    float oldX = posX;
    float oldY = posY;

    posX += velocityX * deltaTime * 60.0f;
    if (CheckHorizontalCollision(mapManager, oldX, oldY)) {
        posX = oldX;
        velocityX = 0.0f;
    }

    posY += velocityY * deltaTime * 60.0f;
    if (CheckVerticalCollision(mapManager, oldX, oldY)) {
        posY = oldY;
        velocityY = 0.0f;
    }
    
    anim.Update(deltaTime);
}
void BossEnemy::ChaseBehavior(float deltaTime) {
    float dx = g_player.posX - posX;
    float distance = fabs(dx);

    // ボスはプレイヤーへ向かって移動する
    if (distance > attackRange) {
        if (dx > 0) {
            velocityX = moveSpeed;
        }
        else {
            velocityX = -moveSpeed;
        }
    }
    else {
        velocityX = 0;  // 攻撃範囲で停止する
    }
}

void BossEnemy::SpecialAttack() {
    // ボスの特殊攻撃 - 複数射弾の発射など
    // ProjectileManager を使って攻撃を生成する
}

void BossEnemy::OnHit(int damage) {
    Enemy::OnHit(damage);
    // ボス被弾リアクション
}

void BossEnemy::OnDeath() {
    Enemy::OnDeath();
    // ボス死亡時 - カットシーンやステージクリアを起動する想定
}

// ボスの被ダメージ処理: DOWN 中は軽減し、一定回数被弾後に弱点ラインを変更する
void BossEnemy::TakeDamage(int damage, float attackAngle) {
    if (!isAlive) return;
    // ボスが active slash 中で、かつ脆弱ウィンドウ内
    // （最初の数フレーム）なら、即座に down シーケンスへ入る。
    // これにより slash 序盤で攻撃を当てると確定でダウンする。
    if (bossState == BOSS_SLASH_ACTIVE) {
        int curFrame = anim.GetCurrentFrame();
        if (curFrame < 3) {
            EnterState(BOSS_DOWN_BEFORE);
            return;
        }
    }

    float multiplier = GetDamageMultiplier(attackAngle);
    int actualDamage = (int)(damage * multiplier);

    // down 中: ダメージ軽減かつ死亡しない
    if (bossState == BOSS_DOWN) {
        actualDamage = std::max(1, actualDamage / 4); // 軽減する
        inDownImmortal = true;
    }

    DamageNumberManager::AddDamageNumber(posX + width * 0.5f, posY + height, actualDamage, multiplier > 1.5f);

    // ヒット時の演出（通常敵と同じ）
    Audio::PlaySE(SoundEffect::ENEMY_HIT);
    SpawnWeakPointHitEffect(posX + width * 0.5f, posY + height * 0.5f);

    health -= actualDamage;
    isHit = true;
    hitTimer = HIT_DURATION;

    hitsTaken++;
    if (bossState != BOSS_DOWN && (hitsTaken == 15 || multiplier > 1.5f)) {
        // down シーケンスへ入る
        EnterState(BOSS_DOWN_BEFORE);
    }

    if (health <= 0) {
        if (inDownImmortal) {
            // down 中は小さな正の HP に丸める
            health = std::max(1.0f, health);
        } else {
            health = 0;
            Audio::PlaySE(SoundEffect::BOSS_DEATH);
            OnDeath();
        }
    }
}

// ===== ボス補助関数群 =====
void BossEnemy::EnterState(BossState s) {
    bossState = s;
    stateTimer = 0.0f;
    // 状態突入時の各種時間を少し乱数化し、行動を読みづらくする
    auto Randomize = [this](float base) {
        float v = timingVariance;
        float r = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f; // [-1,1]
        return base * (1.0f + r * v);
    };
    switch (s) {
    case BOSS_IDLE:
        anim.SetClip("idle");
        velocityX = 0.0f;
        facingLocked = false;
        break;
    case BOSS_DASH_CHARGE:
        // stage1 を使ってチャージアニメを開始する
        if (anim.GetCurrentClipName() != "charge_stage1") {
            Audio::PlaySE(SoundEffect::BOSS_CHARGE);
            anim.SetClip("charge_stage1");
        }
        velocityX = 0.0f;
        // チャージ開始時に向きを固定する
        fixedFacingRight = facingRight;
        // ダッシュ方向決定後は向きを変えない
        facingLocked = true;
        // ランダム化したチャージ時間
        randomizedChargeDuration = Randomize(chargeDuration);
        break;
    case BOSS_DASH_MOVING:
        Audio::PlaySE(SoundEffect::BOSS_DASH);
        anim.SetClip("dash");
        // ダッシュ移動中は固定した向きを維持する
        facingRight = fixedFacingRight;
        facingLocked = true;
        // ランダム化したダッシュ移動時間（後で dashMaxDuration でも制限）
        randomizedDashMovingDuration = Randomize(dashMaxDuration * 0.25f);
        break;
    case BOSS_DASH_AFTER:
        // まず dash_over を再生し、完了後に idle へ戻る
        anim.SetClip("dash_over");
        velocityX = 0.0f;
        facingLocked = false;
        randomizedDashAfterDuration = Randomize(dashAfterDuration);
        break;
    case BOSS_SLASH_CHARGE:
        Audio::PlaySE(SoundEffect::BOSS_CHARGE2);
        anim.SetClip("slash_prep");
        velocityX = 0.0f;
        // slash 開始時に向きを固定する
        fixedFacingRight = facingRight;
        facingLocked = true;
        // slash でも同じチャージ時間ランダム化を使う
        randomizedChargeDuration = Randomize(chargeDuration);
        break;
    case BOSS_SLASH_ACTIVE:
        Audio::PlaySE(SoundEffect::BOSS_SLASH1);
        anim.SetClip("slash_active");
        hasSpawnedSlashProjectiles = false;
        // 固定した向きを維持する
        facingRight = fixedFacingRight;
        // active slash 突入時は攻撃中扱いにし、接触でプレイヤーへダメージを与えられるようにする
        // （IsCurrentlyAttacking は active 中に true を返す）
        // 実際の判定は状態とアニメフレームで行う。
        break;
    case BOSS_DOWN_BEFORE:
        Audio::PlaySE(SoundEffect::BOSS_DOWN);
        anim.SetClip("down_before");
        velocityX = 0.0f;
        facingLocked = false;
        break;
    case BOSS_DOWN:
        anim.SetClip("down_hori");
        velocityX = 0.0f;
        inDownImmortal = true;
        facingLocked = true; // down 中は向きを固定する
        randomizedDownDuration = Randomize(downDuration);
        break;
    case BOSS_DOWN_AFTER:
        anim.SetClip("idle");
        velocityX = 0.0f;
        inDownImmortal = false;
        facingLocked = false;
        break;
    }
}

void BossEnemy::UpdateDashCharge(float dt) {
    // チャージ中、途中で stage1 から stage2 へ進める
    float half = randomizedChargeDuration * 0.6f;
    if (stateTimer >= half && anim.GetCurrentClipName() == std::string("charge_stage1")) {
        anim.SetClip("charge_stage2");
    }
    if (stateTimer >= randomizedChargeDuration * 0.9f) {
        EnterState(BOSS_DASH_MOVING);
        // 固定した向きへ高速移動する（見た目の向きと一致させる）
        float dir = fixedFacingRight ? 1.0f : -1.0f;
        float dashMul = dashSpeedMultiplier * (1.0f + 0.25f * (dashLevel - 1));
        velocityX = dir * moveSpeed * dashMul;
        // 見た目の向きがダッシュ方向と一致するようにする
        facingRight = fixedFacingRight;
    }
}

void BossEnemy::UpdateDashMoving(float dt, MapManager* mapManager) {
    // dash_over へ移る前にダッシュアニメのループを繰り返す。
    // 必要時間を計算しつつ、短め / 長めのランダム化も許可する。
    constexpr float kDashLoopSeconds = 2.0f * 0.05f;
    constexpr int kDashLoops = 4;
    const float requiredTime = kDashLoopSeconds * kDashLoops;

    float allowedTime = requiredTime;
    // randomizedDashMovingDuration があれば使い、なければ dashMaxDuration の 1/4 を使う
    if (randomizedDashMovingDuration > 0.0f) allowedTime = randomizedDashMovingDuration;

    // 必要時間 / ランダム時間のあいだダッシュを続けつつ、最大時間でも打ち切る
    if (stateTimer >= allowedTime || stateTimer > dashMaxDuration) {
        EnterState(BOSS_DASH_AFTER);
        velocityX = 0.0f;
        return;
    }
}

void BossEnemy::UpdateDashAfter(float dt) {
    // `dash_over` は非ループ。再生終了または短いタイムアウト後に idle へ戻る
    if (anim.IsFinished() || stateTimer >= randomizedDashAfterDuration) {
        EnterState(BOSS_IDLE);
    }
}

void BossEnemy::UpdateSlashCharge(float dt) {
    // 準備アニメ完了後（またはフォールバック時間経過後）に active slash へ入る
    if (stateTimer >= randomizedChargeDuration * 0.5) {        //(anim.IsFinished() || stateTimer >= chargeDuration)
        EnterState(BOSS_SLASH_ACTIVE);
    }
}

void BossEnemy::UpdateSlashActive(float dt) {
    // ダメージは最後から 2 番目のフレームで与える。
    // startFrame=7 / endFrame=0 の逆再生では、そのフレームは 1 になる。
    if (anim.GetCurrentFrame() == 1) {
        float range = 0.25f;
        float hx = facingRight ? (posX + width) : (posX - range);
        float hw = range;
        float hy = posY;
        float hh = height;
        // プレイヤーが未死亡・無敵でない・ダッシュ中でない場合のみ倒す。
        // （ダッシュは攻撃状態なので中断させない）
        // beam enemy 側の条件とそろえ、挙動差を防ぐ。
        if (CheckCollision(hx, hy, hw, hh, g_player.posX, g_player.posY, PLAYER_WIDTH, PLAYER_HEIGHT)
            && !g_player.isDead && !g_player.isInvincible && !g_player.isDashing) {
            g_player.health = 0.0f;
            OnPlayerDeath();
        }
        // 射弾ばら撒きを 1 回だけ生成する
        if (!hasSpawnedSlashProjectiles) {
            hasSpawnedSlashProjectiles = true;
            ProjectileManager& pm = ProjectileManager::GetInstance();
            float originX = posX + width * 0.5f;
            float originY = posY + height * 0.5f;
            // ボスの向きに沿って扇状弾幕を撃つ
            const int bulletCount = 9;
            const float totalSpread = 0.9f; // ラジアン。広めの扇形
            // 固定向きが有効ならそれを使い、なければ現在の向きを使う
            bool faceRight = facingLocked ? fixedFacingRight : facingRight;
            float baseAngle = faceRight ? 0.0f : 3.14159f;
            for (int i = 0; i < bulletCount; ++i) {
                float t = (bulletCount == 1) ? 0.0f : (float)i / (bulletCount - 1);
                float ang = baseAngle + (t - 0.5f) * totalSpread;
                float dx = cosf(ang);
                float dy = sinf(ang);
                float targetX = originX + dx * 4.0f;
                float targetY = originY + dy * 4.0f;
                pm.CreateBullet(originX, originY, targetX, targetY, false);
            }
        }
    }
    // クリップが終わったら slash を終了する
    if (anim.IsFinished()) {
        EnterState(BOSS_IDLE);
    }
}

bool BossEnemy::IsCurrentlyAttacking() const {
    // slash 攻撃: ダメージは特定フレーム（frame 1）で発生する
    if (bossState == BOSS_SLASH_ACTIVE) {
        return (anim.GetCurrentFrame() == 1);
    }

    // ダッシュ移動中: dash 状態では接触ダメージありの攻撃中として扱う
    if (bossState == BOSS_DASH_MOVING) {
        return true;
    }

    return false;
}

void BossEnemy::UpdateDownBefore(float dt) {
    if (stateTimer >= 0.5f) {
        EnterState(BOSS_DOWN);
        RecomputeWeakMultipliers();
    }
}

void BossEnemy::UpdateDown(float dt) {
    if (stateTimer >= randomizedDownDuration) {
        EnterState(BOSS_DOWN_AFTER);
    }
}

void BossEnemy::UpdateDownAfter(float dt) {
    if (stateTimer >= 0.5f) {
        // ボスが down-after 復帰を終えて立ち上がったら、
        // 累積ヒット数をリセットし、次の down には新たな被弾が必要にする。
        hitsTaken = 0;
        EnterState(BOSS_IDLE);
    }
}

void BossEnemy::RecomputeWeakMultipliers() {
    // 弱点方向を巡回させ、弱点ラインが変化するように見せる
    weakCycleIndex = (weakCycleIndex + 1) % 4;
    // すべて 1.0 に戻す
    SetDamageMultiplier(DIR_FRONT, 1.0f);
    SetDamageMultiplier(DIR_BACK, 1.0f);
    SetDamageMultiplier(DIR_UP, 1.0f);
    SetDamageMultiplier(DIR_DOWN, 1.0f);
    // 毎回 1 方向だけ大きな弱点にする
    switch (weakCycleIndex) {
    case 0: SetDamageMultiplier(DIR_FRONT, 2.0f); break;
    case 1: SetDamageMultiplier(DIR_BACK, 2.0f); break;
    case 2: SetDamageMultiplier(DIR_UP, 2.0f); break;
    case 3: SetDamageMultiplier(DIR_DOWN, 2.0f); break;
    }
}


FinalBossEnemy::FinalBossEnemy(float x, float y) : BossEnemy(x, y)
{
    SetMaxHealth(250.0f);
    SetHealth(250.0f);

    dashSpeedMultiplier = 25.0f;  // faster dash
    dashMaxDuration = 0.6f;       // 
    dashAfterDuration = 0.25f;    //
    idleDuration = 0.5f;          //
    chargeDuration = 0.7f;        // shorter charge window (harder to react)
    downDuration = 2.0f;          // shorter vulnerability window
    timingVariance = 0.15f;       // less predictable timing
    bossFreezeTimer = 3.0f;       // AIfreeze when enter boss map

    anim.ClearClips();

    anim.AddClip("idle", 0, 3, 4, 1, 0.12f, true, g_finalbossIdleTexture);
    anim.AddClip("dash", 0, 5, 6, 1, 0.09f, false, g_finalbossDashTexture);
    anim.AddClip("dash_over", 0, 3, 4, 1, 0.06f, false, g_finalbossDashOverTexture);
    anim.AddClip("charge_stage1", 0, 3, 4, 1, 0.10f, true, g_finalbossChargeStage1Texture);
    anim.AddClip("charge_stage2", 0, 2, 3, 1, 0.06f, false, g_finalbossChargeStage2Texture);
    anim.AddClip("slash_prep", 3, 0, 4, 1, 0.06f, true, g_finalbossSlashPrepTexture);
    anim.AddClip("slash_active", 0, 7, 8, 1, slashFrameTime, false, g_finalbossSlashActiveTexture);
    anim.AddClip("down_before", 0, 4, 5, 1, 0.06f, false, g_finalbossDownBeforeTexture);
    anim.AddClip("down_hori", 0, 0, 1, 1, 0.1f, false, g_finalbossDownHorizontalTexture);
    anim.AddClip("death", 0, 14, 15, 1, 0.06f, false, g_finalbossDeathTexture);

    anim.SetClip("idle");
}

void FinalBossEnemy::Render(ID3D11ShaderResourceView* texture, const Camera& camera)
{
    BossEnemy::Render(texture, camera);
}

void FinalBossEnemy::ResetState()
{
    BossEnemy::ResetState();
}



// SquareEnemy 実装 - 静止する敵
SquareEnemy::SquareEnemy(float x, float y) : Enemy(x, y, 10.0f) {
    useTurnCooldown = false;
    // Square enemy: 全方向から通常ダメージを受ける
    SetDamageMultiplier(DIR_FRONT, 1.0f);
    SetDamageMultiplier(DIR_BACK, 1.0f);
    SetDamageMultiplier(DIR_UP, 1.0f);
    SetDamageMultiplier(DIR_DOWN, 1.0f);
    SetDamageMultiplier(DIR_FRONT_UP, 1.0f);
    SetDamageMultiplier(DIR_FRONT_DOWN, 1.0f);
    SetDamageMultiplier(DIR_BACK_UP, 1.0f);
    SetDamageMultiplier(DIR_BACK_DOWN, 1.0f);

    width = PLAYER_WIDTH * 1.0f;
    height = PLAYER_HEIGHT * 1.0f;
    moveSpeed = 0.0f;  // 移動しない
    detectionRange = 0.0f;  // 追跡しない
    attackRange =  0.0f;  // 接触ダメージのみ

    // アニメーションを追加する（フレーム数はスプライトに応じて調整）
    anim.AddClip("idle", 0, 7, 1, 8, 0.15f, true, g_squareEnemyIdleTexture);
    anim.AddClip("death", 0, 3, 1, 4, 0.06f, false, g_squareEnemyDeathTexture); // 必要に応じてフレーム数調整

    anim.SetClip("idle");

    pulseTimer = 0.0f;
    scale = 3.0f;
}

void SquareEnemy::Update(float deltaTime, MapManager* mapManager) {
    // まず死亡状態を処理する
    if (isDying) {
        anim.Update(deltaTime);

        if (anim.IsFinished()) {
            markedForDeletion = true;
        }
        return;
    }

    if (!isAlive) {
        OnDeath();
        return;
    }

    // 可視性判定
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

    anim.Update(deltaTime);

    // 被弾状態を処理する
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    // Square enemy は移動しない
    velocityX = 0.0f;
    velocityY = 0.0f;

    // 視覚フィードバック用の脈動エフェクト
    pulseTimer += deltaTime;

    // その場に留まるだけ
    currentState = PATROL;

    if (!isCurrentlyVisible) {
        offScreenTimer += deltaTime;
    }
    else {
        offScreenTimer = 0.0f;
    }
}

void SquareEnemy::OnHit(int damage) {
    // Square enemy は被弾時の特殊挙動を持たない
    Enemy::OnHit(damage);
}

void SquareEnemy::OnDeath() {
    // 基底の死亡処理を呼ぶ
    Enemy::OnDeath();
}


BeamEnemy::BeamEnemy(float x, float y) : Enemy(x, y, 30.0f) {
    // 弱点: 縦線と横線（一撃弱点）
    SetDamageMultiplier(DIR_UP, 100.0f);
    SetDamageMultiplier(DIR_DOWN, 100.0f);
    SetDamageMultiplier(DIR_FRONT, 100.0f);
    SetDamageMultiplier(DIR_BACK, 100.0f);

    width = PLAYER_WIDTH * 1.3f;
    height = PLAYER_HEIGHT * 1.3f;
    moveSpeed = 0.0f;
    detectionRange = 0.6f;  // 必要な範囲に応じて調整する

    // アニメーション用
    anim.AddClip("idle", 0, 2, 1, 3, 0.25f, true, g_beamEnemyIdleTexture);
    anim.AddClip("pre_attack", 0, 3, 1, 4, 0.8f, false, g_beamEnemyPreAttackTexture);
    anim.AddClip("attack", 0, 3, 1, 4, 0.06f, true, g_beamEnemyAttackTexture);
    anim.AddClip("post_attack", 0, 2, 1, 3, 0.15f, false, g_beamEnemyPostAttackTexture);
    anim.AddClip("pre_death", 0, 5, 1, 6, 0.03f, false, g_beamEnemyPreDeathTexture);
    anim.AddClip("death", 0, 5, 1, 6, 0.06f, false, g_beamEnemyDeathTexture);
    anim.AddClip("post_death", 0, 2, 1, 3, 0.15f, false, g_beamEnemyPostDeathTexture);

    anim.SetClip("idle");

    scale = 13.2f;
    beamState = BEAM_IDLE;
    currentCooldown = 0.0f;
    stateTimer = 0.0f;
    deathAnimationPhase = 0;
    hasExploded = false;
    hasKilledPlayerThisAttack = false;
    pulseTimer = 0.0f;
}

void BeamEnemy::TakeDamage(int damage, float attackAngle) {
    if (!isAlive) return;

    float multiplier = GetDamageMultiplier(attackAngle);
    int actualDamage = (int)(damage * multiplier);

    // ダメージ数字を表示する
    bool isCritical = (multiplier >= 8.0f);
    DamageNumberManager::AddDamageNumber(
        posX + width * 0.5f,
        posY + height,
        actualDamage,
        isCritical
    );

    // 弱点に当たったら即死
    if (multiplier >= 8.0f) {
        health = 0;
        isAlive = false;
        OnDeath();
        return;
    }

    // 通常ダメージ
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

void BeamEnemy::Update(float deltaTime, MapManager* mapManager) {
    // 死亡アニメシーケンスを処理する
    if (isDying) {
        anim.Update(deltaTime);

        // Phase 0: pre_death アニメ
        if (deathAnimationPhase == 0) {
            if (anim.GetCurrentClipName() != "pre_death") {
                anim.SetClip("pre_death");
            }
            if (anim.IsFinished()) {
                deathAnimationPhase = 1;
                anim.SetClip("death");
            }
        }
        // Phase 1: 爆発付きの本死亡アニメ
        else if (deathAnimationPhase == 1) {
    // 爆発を発生させる
            if (!hasExploded && anim.GetCurrentFrame() >= 1) {
                CreateDeathExplosion();
                hasExploded = true;
            }
            if (anim.IsFinished()) {
                deathAnimationPhase = 2;
                anim.SetClip("post_death");
            }
        }
        // Phase 2: post_death アニメ（ビームが消えていく）
        else if (deathAnimationPhase == 2) {
            if (anim.IsFinished()) {
                markedForDeletion = true;
            }
        }

        return;
    }

    if (!isAlive) {
        OnDeath();
        return;
    }

    // 被弾状態を処理する
    if (isHit) {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f) {
            isHit = false;
        }
    }

    velocityX = 0.0f;
    velocityY = 0.0f;
    pulseTimer += deltaTime;

    // プレイヤーまでの距離を計算する
    float dx = g_player.posX - posX;
    float dy = g_player.posY - posY;
    float distance = sqrt(dx * dx + dy * dy);

    // 向きを更新する
    if (dx != 0) {
        facingRight = (dx > 0);
    }

    // 現在フレームを取得する（switch 前に宣言してスコープ問題を避ける）
    int currentFrame = anim.GetCurrentFrame();

    // beam 攻撃サイクル用のステートマシン
    switch (beamState) {
    case BEAM_IDLE:
        if (anim.GetCurrentClipName() != "idle") {
            anim.SetClip("idle");
        }

        // クールダウンタイマー
        if (currentCooldown > 0.0f) {
            currentCooldown -= deltaTime;
        }

        // プレイヤーが範囲内かつクールダウン終了か確認する
        if (distance < detectionRange && currentCooldown <= 0.0f) {
            beamState = BEAM_PRE_ATTACK;
            stateTimer = 0.0f;
            anim.SetClip("pre_attack");
        }
        break;

    case BEAM_PRE_ATTACK:
        stateTimer += deltaTime;

        hasKilledPlayerThisAttack = false; // 新しい攻撃開始時に撃破フラグをリセットする

        // 溜めアニメ完了後に攻撃開始する
        if (stateTimer >= preAttackDuration || anim.IsFinished()) {
            beamState = BEAM_ATTACKING;
            stateTimer = 0.0f;
            anim.SetClip("attack");
        }
        break;

    case BEAM_ATTACKING:
        stateTimer += deltaTime;

        CheckBeamDamage(); // ビームダメージは特定フレームだけでなく攻撃中毎フレーム確認する

        // 攻撃時間終了後に後隙段階へ入る
        if (stateTimer >= attackDuration) {
            beamState = BEAM_POST_ATTACK;
            stateTimer = 0.0f;
            anim.SetClip("post_attack");
        }
        break;

    case BEAM_POST_ATTACK:
        stateTimer += deltaTime;

        // 後隙アニメ後に idle へ戻る
        if (stateTimer >= postAttackDuration || anim.IsFinished()) {
            beamState = BEAM_IDLE;
            currentCooldown = attackCooldown;  // クールダウンをリセットする
            anim.SetClip("idle");
        }
        break;
    }

    anim.Update(deltaTime);  // アニメを更新する
}

void BeamEnemy::CheckBeamDamage() {
    // 1 回の攻撃につきプレイヤーを 1 回だけ倒すようにする
    if (hasKilledPlayerThisAttack) return;

    // 中心座標を取得する
    float centerX = posX + width * 0.5f;
    float centerY = posY + height * 0.5f;

    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float distanceX = fabs(playerCenterX - centerX);
    float distanceY = fabs(playerCenterY - centerY);

    bool hitHorizontal = false;
    bool hitVertical = false;

    // 水平ビーム（左右の線）を確認する
    if (distanceY < beamHitboxWidth && distanceX < beamHorizontalLength) {
        hitHorizontal = true;
    }

    // 垂直ビーム（上下の線）を確認する
    if (distanceX < beamHitboxWidth && distanceY < beamVerticalLength) {
        hitVertical = true;
    }

    // ど真ん中に小さな安全地帯を作る
    float centerSafeZone = 0.15f;
    bool inCenterSafeZone = (distanceX < centerSafeZone && distanceY < centerSafeZone);

    // どちらかの線に触れていれば命中。ただし中央安全地帯内は除く
    bool hit = (hitHorizontal || hitVertical) && !inCenterSafeZone;

    // プレイヤーは即死する
    if (hit && !g_player.isDead && !g_player.isInvincible && !g_player.isDashing) {
        g_player.health = 0.0f;
        OnPlayerDeath();
        hasKilledPlayerThisAttack = true;
    }
}

void BeamEnemy::OnDeath() {
    if (isDying) return;

    // 死亡アニメシーケンスを開始する
    Enemy::OnDeath();
    deathAnimationPhase = 0;  // pre_death アニメから始める
    hasExploded = false;
}

void BeamEnemy::CreateDeathExplosion() {
    // プレイヤーではなく他の敵だけにダメージを与える
    // この beam enemy の中心位置
    float centerX = posX + width * 0.5f;
    float centerY = posY + height * 0.5f;

    // 以前の大きな円形爆発版
    // 周囲の敵へダメージを与える
    /*for (auto& enemy : g_enemies) {
        if (!enemy->IsAlive() || enemy == this) continue;

        float dx = enemy->GetX() + enemy->GetWidth() * 0.5f - centerX;
        float dy = enemy->GetY() + enemy->GetHeight() * 0.5f - centerY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance <= deathExplosionRadius) {
            float angle = atan2(dy, dx);
            enemy->TakeDamage((int)deathExplosionDamage, angle);
        }
    }*/

    // こちらは beam enemy 撃破時に「+」型レーザー爆発になる版
    for (auto& enemy : g_enemies) {
        if (!enemy->IsAlive() || enemy == this) continue;

        // 敵の中心位置を計算する
        float enemyCenterX = enemy->GetX() + enemy->GetWidth() * 0.5f;
        float enemyCenterY = enemy->GetY() + enemy->GetHeight() * 0.5f;

        // 2 つの中心間の水平距離・垂直距離を求める（中心は beam enemy）
        // fabs により常に正の値になる
        float distanceX = fabs(enemyCenterX - centerX);
        float distanceY = fabs(enemyCenterY - centerY);

        bool hitHorizontal = false;
        bool hitVertical = false;

        // 水平ビーム（左右の線）を確認する
        if (distanceY < beamHitboxWidth && distanceX < deathExplosionRadius) {
            hitHorizontal = true;
        }

        // 垂直ビーム（上下の線）を確認する
        if (distanceX < beamHitboxWidth && distanceY < deathExplosionRadius) {
            hitVertical = true;
        }

        // どちらかの線に触れていれば命中
        bool hit = (hitHorizontal || hitVertical);

        if (hit) {
            float dx = enemyCenterX - centerX;
            float dy = enemyCenterY - centerY;
            float angle = atan2(dy, dx); // beam enemy から他敵への角度（ラジアン）を計算する
            enemy->TakeDamage((int)deathExplosionDamage, angle);
        }
    }
}



// 敵更新関数
void UpdateEnemies(float deltaTime, MapManager* mapManager) {
    DamageNumberManager::Update(deltaTime);

    int visibleEnemyCount = 0;
    int totalEnemyCount = (int)g_enemies.size();

    // 注意:
    // 一部の敵（例: ThrowerEnemy）は Update() 中に新しい敵を生成することがある。
    // 変更中の std::vector を range-for で回すと参照 / イテレータが無効化されて
    // クラッシュする可能性があるため、初期個数をインデックスで走査する。
    const size_t initialCount = g_enemies.size();
    for (size_t i = 0; i < initialCount; ++i) {
        Enemy* enemy = g_enemies[i];
        if (!enemy) continue;

        // デバッグ情報: 可視敵数を数える
        if (enemy->IsVisible(g_camera)) {
            visibleEnemyCount++;
        }

        enemy->Update(deltaTime, mapManager);
    }

    // デバッグ出力（任意）
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

    // 死亡した敵を取り除く
    g_enemies.erase(
        std::remove_if(g_enemies.begin(), g_enemies.end(),
            [](Enemy* e) {
                if (!e) {
                    return true;
                }
                if (/*!e->IsAlive()*/e->IsMarkedForDeletion()) {
                    delete e;
                    return true;
                }
                return false;
            }),
        g_enemies.end()
    );
}

// RenderEnemies 関数を修正する
void RenderEnemies(const Camera& camera) {
    for (auto& enemy : g_enemies) {
        ID3D11ShaderResourceView* texture = g_enemyIdleTexture;  // 既定テクスチャ

        if (dynamic_cast<FlyEnemy*>(enemy)) {  // FlyEnemy を使う
            texture = g_flyEnemyIdleTexture;
        }
        else if (dynamic_cast<MageEnemy*>(enemy)) {
            texture = g_mageEnemyIdleTexture;
        }
        else if (dynamic_cast<FastEnemy*>(enemy)) {
            texture = g_fastEnemyRunTexture;
        }
        else if (dynamic_cast<BombEnemy*>(enemy)) {
            texture = g_bombEnemyIdleTexture;
        }
        else if (dynamic_cast<SquareEnemy*>(enemy)) {
            texture = g_squareEnemyIdleTexture;
        }
        else if (dynamic_cast<FinalBossEnemy*>(enemy)) {
            texture = g_finalbossIdleTexture;
        }
        else if (dynamic_cast<BossEnemy*>(enemy)) {
            texture = g_bossIdleTexture;
        }

        enemy->Render(texture, camera); // カメラ引数を渡す
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
