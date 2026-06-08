#include "Game.h"
#include "Enemy.h"
#include "Audio.h"

static void PerformDashEndCircleHitTest();

static float ComputeDashHitStopTime()
{
    // ベースとなるヒット感（旧値）
    constexpr float BASE_HITSTOP = 0.075f;
    // チャージ段階ごとの追加ヒットストップ（0..3）
    constexpr float EXTRA_PER_LEVEL = 0.025f;
    constexpr float MAX_HITSTOP = 0.18f;

    int chargeLevel = 0;
    if (g_player.hasSavedCharge) {
        chargeLevel = g_player.GetChargeLevelFromTime(g_player.savedChargeTime);
    }
    else {
        chargeLevel = g_player.GetChargeLevelFromTime(g_player.chargeTime);
    }

    float hitStop = BASE_HITSTOP + static_cast<float>(chargeLevel) * EXTRA_PER_LEVEL;
    // 要望対応: 敵命中後のヒットストップ時間を 1.5 倍にする。
    hitStop *= 1.5f;
    return std::min(hitStop, MAX_HITSTOP);
}

// ダッシュ命中判定補助: 指定位置で、縮小かつ中央寄せした当たり判定で敵を確認する
static void PerformDashHitTest(float testX, float testY) {
    if (!g_player.isDashing) {
        return;
    }

    // ダッシュ斬撃の「攻撃判定」は、ダッシュ中の「被弾判定」と同一であるべきではない
    // （現在の物理当たり判定は 1/4 に縮小されている）。
    // ここでは命中判定だけ別に広げ、高速移動時のすれ違いを防ぐ。
    // 要望対応: 斬撃幅を広げる。
    constexpr float DASH_ATTACK_HITBOX_SCALE_X = 1.35f;
    constexpr float DASH_ATTACK_HITBOX_SCALE_Y = 0.85f;

    float playerWidth = PLAYER_WIDTH;
    float playerHeight = PLAYER_HEIGHT;
    float offsetX = 0.0f;
    float offsetY = 0.0f;

    if (g_player.isDashing) {
        playerWidth = PLAYER_WIDTH * DASH_ATTACK_HITBOX_SCALE_X;
        playerHeight = PLAYER_HEIGHT * DASH_ATTACK_HITBOX_SCALE_Y;
        offsetX = (PLAYER_WIDTH - playerWidth) * 0.5f;
        offsetY = (PLAYER_HEIGHT - playerHeight) * 0.5f;
    }

    float dashAngle = atan2(g_player.dashDirectionY, g_player.dashDirectionX);

    for (auto& enemy : g_enemies) {
        if (!enemy->IsAlive()) {
            continue;
        }

        // すでに命中済みの敵はスキップする
        if (std::find(g_player.hitEnemies.begin(), g_player.hitEnemies.end(), enemy) != g_player.hitEnemies.end()) {
            continue;
        }

        if (CheckCollision(testX + offsetX, testY + offsetY, playerWidth, playerHeight,
            enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight())) {

            //g_player.comboCount++;
            //g_player.comboTimer = 5.0f;

            //g_gameStats.UpdateMaxCombo(g_player.comboCount);// 1 月 22 日追加

            float multiplier = enemy->GetDamageMultiplier(dashAngle);
            // ヒットストップは無敵状態ではなくチャージ強度に依存させる。
            if (g_player.hitStopTriggered < 3) {
                g_player.hitStopTimer = ComputeDashHitStopTime();
                g_player.hitStopTriggered++;
                if (!g_player.isInvincible) {
                    g_camera.Shake(0.02f, 0.05f);
                    // 任意で全体スローモーション
                    // TriggerSlowMotion(0.05f, 0.3f);

                    if (g_player.gaugePoints < g_player.MAX_GAUGE_POINTS) {
                        // 命中ごとに常に 1 ゲージだけ与える。
                        // 弱点ヒット時の追加ゲージはなくし、増加量を統一する。
                        g_player.gaugePoints += 1;
                        if (g_player.gaugePoints > g_player.MAX_GAUGE_POINTS) {
                            g_player.gaugePoints = g_player.MAX_GAUGE_POINTS;
                        }
                    }
                }
            }

           /* if (g_player.gaugePoints < g_player.MAX_GAUGE_POINTS) {
                if (multiplier > 1.5f) {
                    g_player.gaugePoints += 2;
                }
                else {
                    g_player.gaugePoints += 1;
                }
                if (g_player.gaugePoints > g_player.MAX_GAUGE_POINTS) {
                    g_player.gaugePoints = g_player.MAX_GAUGE_POINTS;
                }
            }*/
 
            // 追加で消費したダッシュポイント 1 つごとに基礎ダメージを 1.5 倍する
            float damageScale = powf(1.5f, static_cast<float>(std::max(1, g_player.lastDashConsumedPoints) - 1));
            int scaledBase = static_cast<int>(g_player.attackDamage * damageScale);
            int actualDamage = enemy->CalculateDamageFromPlayer(scaledBase, dashAngle);
            enemy->TakeDamage(actualDamage, dashAngle);

            g_player.hitEnemies.push_back(enemy);
        }
    }

    // ダッシュ攻撃判定に触れた敵射弹にも命中させる
    // 上で使った同じ矩形を使う
    g_projectileManager.HandlePlayerSlashHitRect(testX + offsetX, testY + offsetY, playerWidth, playerHeight);
}

// ダッシュ終点に小さな円形命中判定を追加する: 終点到達時に一度だけ発動する。
// この判定も hitEnemies を使って重複除外するため、ダッシュ中のダメージと重ならない。
static void PerformDashEndCircleHitTest() {
    if (!g_player.isDashing || !g_player.hasMouseTarget) {
        return;
    }

    const float forwardDistance = PLAYER_WIDTH * 0.5f;
    const float radius = PLAYER_WIDTH * 0.35f;
    const float radiusSq = radius * radius;

    float endCenterX = g_player.mouseTargetX + g_player.dashDirectionX * forwardDistance;
    float endCenterY = g_player.mouseTargetY + g_player.dashDirectionY * forwardDistance;

    float dashAngle = atan2(g_player.dashDirectionY, g_player.dashDirectionX);

    for (auto& enemy : g_enemies) {
        if (!enemy || !enemy->IsAlive()) {
            continue;
        }

        if (std::find(g_player.hitEnemies.begin(), g_player.hitEnemies.end(), enemy) != g_player.hitEnemies.end()) {
            continue;
        }

        float enemyCenterX = enemy->GetX() + enemy->GetWidth() * 0.5f;
        float enemyCenterY = enemy->GetY() + enemy->GetHeight() * 0.5f;

        float dx = enemyCenterX - endCenterX;
        float dy = enemyCenterY - endCenterY;
        if (dx * dx + dy * dy <= radiusSq) {
            //g_player.comboCount++;
            //g_player.comboTimer = 5.0f;

            //g_gameStats.UpdateMaxCombo(g_player.comboCount); // 1 月 22 日追加

            float multiplier = enemy->GetDamageMultiplier(dashAngle);
            // ヒットストップは無敵状態ではなくチャージ強度に依存させる。
            if (g_player.hitStopTriggered < 3) {
                g_player.hitStopTimer = ComputeDashHitStopTime();
                g_player.hitStopTriggered++;
                if (!g_player.isInvincible) {
                    g_camera.Shake(0.02f, 0.05f);

                    if (g_player.gaugePoints < g_player.MAX_GAUGE_POINTS) {
                        // 命中ごとに常に 1 ゲージだけ与える。
                        // 弱点ヒット時の追加ゲージはなくし、増加量を統一する。
                        g_player.gaugePoints += 1;
                        if (g_player.gaugePoints > g_player.MAX_GAUGE_POINTS) {
                            g_player.gaugePoints = g_player.MAX_GAUGE_POINTS;
                        }
                    }
                }
            }

            /*if (g_player.gaugePoints < g_player.MAX_GAUGE_POINTS) {
                if (multiplier > 1.5f) {
                    g_player.gaugePoints += 2;
                }
                else {
                    g_player.gaugePoints += 1;
                }
                if (g_player.gaugePoints > g_player.MAX_GAUGE_POINTS) {
                    g_player.gaugePoints = g_player.MAX_GAUGE_POINTS;
                }
            }*/

            // 追加で消費したダッシュポイント 1 つごとに基礎ダメージを 1.5 倍する
            float damageScale = powf(1.5f, static_cast<float>(std::max(1, g_player.lastDashConsumedPoints) - 1));
            int scaledBase = static_cast<int>(g_player.attackDamage * damageScale);
            int actualDamage = enemy->CalculateDamageFromPlayer(scaledBase, dashAngle);
            enemy->TakeDamage(actualDamage, dashAngle);
            g_player.hitEnemies.push_back(enemy);
        }
    }

    // 終点円形範囲にある敵射弹にも命中させる
    g_projectileManager.HandlePlayerSlashHitCircle(endCenterX, endCenterY, radius);
}

void UpdatePlayerPhysics(float deltaTime) {
    if (g_player.isDead)
        return;

    if (g_player.oneWayPlatformDropTimer > 0.0f) {
        g_player.oneWayPlatformDropTimer = std::max(
            0.0f,
            g_player.oneWayPlatformDropTimer - deltaTime
        );
    }

    g_mapManager.GetCurrentMap()->BuildSpatialGrid();// TODO
    // 現在のプレイヤー当たり判定サイズを計算する
    float currentWidth = PLAYER_WIDTH;
    float currentHeight = PLAYER_HEIGHT;

    // 当たり判定のオフセットを計算し、中心位置が変わらないようにする
    float offsetX = 0.0f;
    float offsetY = 0.0f;

    // ダッシュ中は当たり判定を 1/4 に縮小し、中心を維持するためのオフセットを計算する
    if (g_player.isDashing) {
        float widthReduction = (PLAYER_WIDTH - PLAYER_WIDTH * 0.25f) / 2.0f;
        float heightReduction = (PLAYER_HEIGHT - PLAYER_HEIGHT * 0.25f) / 2.0f;
        currentWidth = PLAYER_WIDTH * 0.25f;
        currentHeight = PLAYER_HEIGHT * 0.25f;
        offsetX = widthReduction;
        offsetY = heightReduction;
    }

    // 重力を適用する
    // 無重力アフターマスモード有効時は、後硬直中を無重力状態として扱う。
    bool ignoreGravity = g_player.isDashing || (g_noGravityAftermathMode && g_player.isInDashAftermath);
    if (!ignoreGravity) {
        float fixedDeltaTime = std::min(deltaTime, 0.033f);

        // 壁滑り中は弱めの重力を適用する
        if (g_player.isWallSliding) {
            // 壁滑り中は落下速度を滑走速度に制限する
            if (g_player.velocityY < g_player.WALL_SLIDE_SPEED) {
                g_player.velocityY = g_player.WALL_SLIDE_SPEED;
            }
            else if (g_player.velocityY > 0) {
                // 上方向へ動いている場合は通常重力を適用する
                g_player.velocityY += GRAVITY * fixedDeltaTime * 60.0f;
            }
            else {
                // 下方向へ動いている場合は弱めの重力を適用する
                g_player.velocityY += GRAVITY * 0.3f * fixedDeltaTime * 60.0f;
            }

            // 壁滑り中は水平速度を徐々に減衰させる
            if (g_player.velocityX > 0) {
                g_player.velocityX = std::max<float>(0.0f, g_player.velocityX - 0.1f);
            }
            else if (g_player.velocityX < 0) {
                g_player.velocityX = std::min<float>(0.0f, g_player.velocityX + 0.1f);
            }
        }
        else {
            // 通常重力
            g_player.velocityY += GRAVITY * fixedDeltaTime * 60.0f;
        }

        if (g_player.velocityY < -0.15f) {
            g_player.velocityY = -0.15f;
        }
    }

    // 壁滑り状態をリセットする
    g_player.isWallSliding = false;
    g_player.wallSlideDirection = 0;

    // ダッシュ中でも地上でもない場合に壁滑りを判定する
    // 注意: 死亡中は環境に応じて向きや状態を書き換えない
    if (!g_player.isDead && !g_player.isDashing && !g_player.isOnGround && g_player.velocityY < 0) {
        // 現在のマップの空間グリッドを取得する
        SpatialGrid* spatialGrid = g_mapManager.GetCurrentMap()->GetSpatialGrid();
        if (spatialGrid) {
            std::vector<MapTile*> nearbyTiles;

            // プレイヤー周辺のタイルを取得する
            float padding = 1.0f;
            spatialGrid->GetTilesInArea(
                g_player.posX + offsetX - padding,
                g_player.posY + offsetY - padding,
                currentWidth + padding * 2,
                currentHeight + padding * 2,
                nearbyTiles
            );

            // プレイヤーの実際の当たり判定で接触を調べる
            float playerLeft = g_player.posX + offsetX;
            float playerRight = playerLeft + currentWidth;
            float playerTop = g_player.posY + offsetY;
            float playerBottom = playerTop + currentHeight;

            // 小さな接触しきい値を定義する
            // 少し大きめにして壁滑り判定を出しやすくする
            const float CONTACT_EPSILON = 0.006f;

            // 左右の壁接触を確認する
            for (const auto& tile : nearbyTiles) {
                if (tile->tileInfo.isSolid && tile->tileInfo.type != "platform") {
                    float tileLeft = tile->posX;
                    float tileRight = tile->posX + tile->width;
                    float tileTop = tile->posY;
                    float tileBottom = tile->posY + tile->height;

                    // 垂直方向の重なりを確認する（Y 軸上で重なる）
                    bool verticalOverlap = (playerTop < tileBottom && playerBottom > tileTop);

                    if (verticalOverlap) {
                        // 左側の壁接触を確認する
                        float leftDistance = tileRight - playerLeft;
                        if (leftDistance >= 0 && leftDistance <= CONTACT_EPSILON) {
                            g_player.isWallSliding = true;
                            g_player.wallSlideDirection = -1; // 左壁
                            g_player.facingRight = true;
                            break; // 1 面見つかれば十分
                        }

                        // 右側の壁接触を確認する
                        float rightDistance = playerRight - tileLeft;
                        if (rightDistance >= 0 && rightDistance <= CONTACT_EPSILON) {
                            g_player.isWallSliding = true;
                            g_player.wallSlideDirection = 1;  // 右壁
                            g_player.facingRight = false;
                            break; // 1 面見つかれば十分
                        }
                    }
                }
            }
        }
    }

    // 衝突判定用に元の位置を保持する
    float oldX = g_player.posX;
    float oldY = g_player.posY;

    // 移動量を計算する
    float moveX = g_player.velocityX * deltaTime * 60.0f;
    float moveY = g_player.velocityY * deltaTime * 60.0f;

    // 現在のマップの空間グリッドを取得する
    SpatialGrid* spatialGrid = g_mapManager.GetCurrentMap()->GetSpatialGrid();

    if (!spatialGrid) {
        // 空間グリッドがない場合は従来方式を使う
        g_player.posX += moveX;
        g_player.posY += moveY;
    }
    else {
        // === 速度に応じて判定回数を動的に調整する ===
        float speed = sqrt(g_player.velocityX * g_player.velocityX + g_player.velocityY * g_player.velocityY);
        int steps = 1;

        if (g_player.isDashing) {
            // ダッシュ時: 基本回数に速度補正を加え、上限を設ける
            steps = std::min(std::max(16, static_cast<int>(ceilf(speed * 20.0f))), 32);
        }
        else if (speed > 0.2f) {
            // 高速移動時は判定回数を増やす
            steps = std::min(static_cast<int>(speed * 10.0f), 16);
        }
        else {
            // 通常移動
            steps = 4; // これまでどおり 4 回にする
        }

        // === 衝突しそうなタイルを事前収集する ===
        std::vector<MapTile*> nearbyTiles;

        // 移動範囲を計算し、検出領域を広げる
        float paddingX = 3.0f;
        float paddingY = 3.0f;

        // 移動後を見越した位置範囲
        float minX = std::min(g_player.posX + offsetX, g_player.posX + offsetX + moveX) - paddingX;
        float minY = std::min(g_player.posY + offsetY, g_player.posY + offsetY + moveY) - paddingY;
        float maxX = std::max(g_player.posX + offsetX + currentWidth,
            g_player.posX + offsetX + moveX + currentWidth) + paddingX;
        float maxY = std::max(g_player.posY + offsetY + currentHeight,
            g_player.posY + offsetY + moveY + currentHeight) + paddingY;

        // 移動範囲内の全タイルを取得する
        spatialGrid->GetTilesInArea(minX, minY, maxX - minX, maxY - minY, nearbyTiles);

        // 通常の固体タイルと一方向足場を分離する
        std::vector<MapTile*> regularSolidTiles;
        std::vector<MapTile*> oneWayPlatformTiles;

        for (const auto& tile : nearbyTiles) {
            if (tile->tileInfo.isSolid) {
                if (tile->tileInfo.type == "platform" && tile->tileInfo.subtype == "one_way") {
                    oneWayPlatformTiles.push_back(tile);
                }
                else {
                    regularSolidTiles.push_back(tile);
                }
            }
        }

        if (g_player.oneWayPlatformDropTimer <= 0.0f &&
            (g_inputSystem.IsKeyPressed(VK_S) || g_inputSystem.IsKeyPressed(VK_DOWN))) {
            const float playerLeft = g_player.posX + offsetX;
            const float playerRight = playerLeft + currentWidth;
            const float standTolerance = 0.03f;

            for (const auto& tile : oneWayPlatformTiles) {
                const float platformLeft = tile->posX;
                const float platformRight = platformLeft + tile->width;
                const bool horizontalOverlap = (playerRight > platformLeft && playerLeft < platformRight);
                if (!horizontalOverlap) {
                    continue;
                }

                const float standY = tile->posY + currentHeight + offsetY;
                if (fabsf(g_player.posY - standY) <= standTolerance) {
                    g_player.oneWayPlatformDropTimer = g_player.ONE_WAY_PLATFORM_DROP_GRACE;
                    g_player.isOnGround = false;
                    g_player.velocityY = std::min(g_player.velocityY, -0.01f);
                    break;
                }
            }
        }

        // 連続衝突判定を使う
        float stepX = moveX / steps;
        float stepY = moveY / steps;

        for (int i = 0; i < steps; i++) {
            g_player.posX += stepX;

            // 水平衝突判定（通常の固体のみ。一方向足場は水平移動へ影響しない）
            bool xCollision = false;
            for (const auto& tile : regularSolidTiles) {
                if (CheckCollision(g_player.posX + offsetX, g_player.posY + offsetY,
                    currentWidth, currentHeight,
                    tile->posX, tile->posY, tile->width, tile->height)) {

                    // 衝突方向と押し戻し量を計算する
                    float playerCenterX = g_player.posX + offsetX + currentWidth / 2;
                    float playerCenterY = g_player.posY + offsetY + currentHeight / 2;
                    float tileCenterX = tile->posX + tile->width / 2;
                    float tileCenterY = tile->posY + tile->height / 2;

                    // 重なり量を計算する
                    float overlapX = (currentWidth / 2 + tile->width / 2) - fabs(playerCenterX - tileCenterX);
                    float overlapY = (currentHeight / 2 + tile->height / 2) - fabs(playerCenterY - tileCenterY);

                    // 重なりが最小の方向を選ぶ
                    if (overlapX < overlapY) {
                        // 水平衝突
                        if (playerCenterX < tileCenterX) {
                            g_player.posX = tile->posX - currentWidth - offsetX;
                        }
                        else {
                            g_player.posX = tile->posX + tile->width - offsetX;
                        }
                        g_player.velocityX = 0.0f;
                    }
                    else {
                        // 垂直衝突
                        if (playerCenterY < tileCenterY) {
                            g_player.posY = tile->posY - currentHeight - offsetY;
                        }
                        else {
                            g_player.posY = tile->posY + tile->height - offsetY;
                        }
                        g_player.velocityY = 0.0f;
                        if (stepY < 0) g_player.isOnGround = true;
                    }
                    xCollision = true;
                    break;
                }
            }

            g_player.posY += stepY;

            // 垂直衝突判定（まず通常の固体を調べる）
            bool yCollision = false;
            float preCollisionY = g_player.posY - stepY; // 衝突前の Y 座標を記録

            for (const auto& tile : regularSolidTiles) {
                if (CheckCollision(g_player.posX + offsetX, g_player.posY + offsetY,
                    currentWidth, currentHeight,
                    tile->posX, tile->posY, tile->width, tile->height)) {

                    // 衝突方向と押し戻し量を計算する
                    float playerCenterX = g_player.posX + offsetX + currentWidth / 2;
                    float playerCenterY = g_player.posY + offsetY + currentHeight / 2;
                    float tileCenterX = tile->posX + tile->width / 2;
                    float tileCenterY = tile->posY + tile->height / 2;

                    // 重なり量を計算する
                    float overlapX = (currentWidth / 2 + tile->width / 2) - fabs(playerCenterX - tileCenterX);
                    float overlapY = (currentHeight / 2 + tile->height / 2) - fabs(playerCenterY - tileCenterY);

                    // 重なりが最小の方向を選ぶ
                    if (overlapX < overlapY) {
                        // 水平衝突
                        if (playerCenterX < tileCenterX) {
                            g_player.posX = tile->posX - currentWidth - offsetX;
                        }
                        else {
                            g_player.posX = tile->posX + tile->width - offsetX;
                        }
                        g_player.velocityX = 0.0f;
                    }
                    else {
                        // 垂直衝突
                        if (playerCenterY < tileCenterY) {
                            g_player.posY = tile->posY - currentHeight - offsetY;
                        }
                        else {
                            g_player.posY = tile->posY + tile->height - offsetY;
                        }
                        g_player.velocityY = 0.0f;
                        if (stepY < 0) g_player.isOnGround = true;
                    }
                    yCollision = true;
                    break;
                }
            }

            // 通常の固体に当たっていない場合は一方向足場を確認する
            if (!yCollision) {
                for (const auto& tile : oneWayPlatformTiles) {
                    if (g_player.oneWayPlatformDropTimer > 0.0f) {
                        continue;
                    }

                    // プレイヤー境界を計算する
                    float playerLeft = g_player.posX + offsetX;
                    float playerRight = playerLeft + currentWidth;
                    float playerTop = g_player.posY + offsetY;
                    float playerBottom = playerTop + currentHeight;

                    // 足場境界を計算する
                    float platformLeft = tile->posX;
                    float platformRight = platformLeft + tile->width;
                    float platformTop = tile->posY;
                    float platformBottom = platformTop + tile->height;

                    // 1. 水平方向の重なりを確認する
                    bool horizontalOverlap = (playerRight > platformLeft && playerLeft < platformRight);

                    if (!horizontalOverlap)
                        continue;

                    if (g_player.velocityY >= 0.0f) 
                        continue; 

                    float currentBottom = playerBottom;

                    if (currentBottom > platformTop + currentHeight + offsetY) {
                        if (g_player.posY < platformTop + currentHeight + offsetY) {
                            g_player.isOnGround = true;
                            g_player.posY = platformTop + currentHeight + offsetY;
                            g_player.velocityY = 0.0f;
                        }
                        yCollision = true;
                        break;
                    }
                }
            }

            // 両方向で衝突した場合
            if (xCollision && yCollision) {
                // 必要ならここで斜め衝突の特別処理を追加できる
                g_player.posY -= stepY; // 垂直移動を巻き戻す
                g_player.posX -= stepX; // 水平移動を巻き戻す
            }

            // ダッシュ中は各サブステップ位置で敵命中判定を行い、高速すり抜けを防ぐ
            if (g_player.isDashing) {
                PerformDashHitTest(g_player.posX, g_player.posY);
            }
        }
    }

    // ポータル処理
    static float portalCooldown = 0.0f;
    if (portalCooldown > 0.0f) {
        portalCooldown -= deltaTime;
    }

    if (portalCooldown <= 0.0f) {
        std::string targetMap;
        int portalId, linkedSpawnId;

        // 正しい当たり判定サイズでポータルを確認する
        float portalWidth = PLAYER_WIDTH;
        float portalHeight = PLAYER_HEIGHT;
        float portalOffsetX = 0.0f;
        float portalOffsetY = 0.0f;

        if (g_player.isDashing) {
            portalWidth = PLAYER_WIDTH * 0.25f;
            portalHeight = PLAYER_HEIGHT * 0.25f;
            portalOffsetX = (PLAYER_WIDTH - portalWidth) / 2.0f;
            portalOffsetY = (PLAYER_HEIGHT - portalHeight) / 2.0f;
        }

        if (g_mapManager.GetCurrentMap()->CheckPortalCollision(
            g_player.posX + portalOffsetX, g_player.posY + portalOffsetY,
            portalWidth, portalHeight,
            targetMap, portalId, linkedSpawnId)) {

            // エリア切替後にコンボを保持するため保存する
            int savedCombo = g_player.comboCount;
            float savedTimer = g_player.comboTimer;

            //if (targetMap == "boss") {
            //    // it goes to boss stage (World 1, Stage 8)
            //    sceneManager.SwitchToStage(1, 8);
            //    portalCooldown = 1.0f;
            //}
            if (targetMap == "boss") {
                // 実際の現在マップ名からワールド番号を導き出し、
                // ボス扉遷移が SceneManager のステージ状態同期に依存しないようにする。
                const std::string currentMapName = g_mapManager.GetCurrentMapName();
                int world = 0;

                if (currentMapName.rfind("World", 0) == 0) {
                    const size_t worldStart = 5;
                    const size_t areaPos = currentMapName.find("Area", worldStart);
                    if (areaPos != std::string::npos) {
                        try {
                            world = std::stoi(currentMapName.substr(worldStart, areaPos - worldStart));
                        }
                        catch (...) {
                            world = 0;
                        }
                    }
                }

                if (world > 0) {
                    sceneManager.SwitchToStage(world, 8);   // そのワールドのボスステージへ移動する
                }
                else {
                    // フォールバック: 解析失敗時も直接マップ切替で安定動作させる。
                    g_mapManager.SwitchMap(targetMap, portalId, linkedSpawnId);
                }

                // 切替後にコンボを復元する
                g_player.comboCount = savedCombo;
                g_player.comboTimer = savedTimer;

                portalCooldown = 1.0f;
                return;
            }


			else {
                // 既定の扉挙動: ボス扉以外は次ステージへ進む。
				const std::string currentMapName = g_mapManager.GetCurrentMapName();
				int world = 0;
				int stage = 0;

				if (currentMapName.rfind("World", 0) == 0) {
					const size_t worldStart = 5;
					const size_t areaPos = currentMapName.find("Area", worldStart);
					if (areaPos != std::string::npos) {
						try {
							world = std::stoi(currentMapName.substr(worldStart, areaPos - worldStart));
							stage = std::stoi(currentMapName.substr(areaPos + 4));
						}
						catch (...) {
							world = 0;
							stage = 0;
						}
					}
				}

				if (world > 0 && stage > 0) {
					sceneManager.SwitchToStage(world, stage + 1);
				}
				else {
                    // 旧挙動へフォールバックする
					g_mapManager.SwitchMap(targetMap, portalId, linkedSpawnId);
				}
                // 切替後にコンボを復元する
                g_player.comboCount = savedCombo;
                g_player.comboTimer = savedTimer;

				portalCooldown = 1.0f;
			}
        }
    }

    // 垂直速度の絶対値が 0.05f を超えるなら地上にいないとみなす
    if (fabs(g_player.velocityY) > 0.05f) {
        g_player.isOnGround = false;
    }

    // 境界チェック
    if (g_player.posY < -4.0f) {
        g_gameStats.IncrementDeaths();

        // 死亡時はコンボをリセットする
        g_player.comboCount = 0;
        g_player.comboTimer = 0.0f;

        // ゲージ状態と視覚効果をクリアする（Game.cpp 側へ委譲）
        ClearGaugeOnDeath();

        // ポイントを持っているときだけ死亡回数を加算対象にする
        int killPoints = (g_gameStats.GetEnemiesKilled() * 10) + (g_gameStats.GetWeakPointKills() * 30);
        if (killPoints > 0) {
            g_gameStats.IncrementPenalizableDeaths();
        }

        ResetGame(true);
    }

    CheckDashAttack();
}

void CheckDashAttack() {
    if (!g_player.isDashing) {
        g_player.hitEnemies.clear();
        g_player.hitStopTriggered = 0; // ヒットストップ発生回数をリセット
        g_player.hitStopTimer = 0.0f;  // ヒットストップタイマーをリセット
        return;
    }

    // 現在の最終位置でも確認する（保険用）。サブステップ判定は UpdatePlayerPhysics 内で実施済み。
    PerformDashHitTest(g_player.posX, g_player.posY);
}

// 追加: プレイヤー死亡状態を更新する
void UpdatePlayerDeath(float deltaTime) {
    if (!g_player.isDead) {
        return;
    }

    // プレイヤー死亡時はコンボをリセットする
    g_player.comboCount = 0;
    g_player.comboTimer = 0.0f;

    g_player.deathTimer -= deltaTime;

    if (g_player.deathTimer <= 0.0f) {
        // 死亡タイムアウト後は完全リセットし、現在マップも再読込して
        // 敵やスポーン地点を復元する。
        ResetGame(true);
    }
}

// 追加: プレイヤー死亡処理
void OnPlayerDeath() {
    g_player.isDead = true;
    g_player.deathTimer = g_player.DEATH_RESPAWN_TIME;
    g_player.deathCount++;

    // 無敵アニメセットが残らないよう、ゲージ無敵フラグをクリアする。
    g_player.isGaugeInvincible = false;

    // 被弾 / 死亡時に小さくカメラを揺らす。
    // 酔い防止のため控えめにする。
    g_camera.Shake(0.1f, 0.16f);

    // 死亡回数を記録する
    g_gameStats.IncrementDeaths();

    g_player.comboCount = 0;
    g_player.comboTimer = 0.0f;

    // ポイントを持っているときだけ死亡回数を加算対象にする
    int killPoints = (g_gameStats.GetEnemiesKilled() * 10) + (g_gameStats.GetWeakPointKills() * 30);
    if (killPoints > 0) {
        g_gameStats.IncrementPenalizableDeaths();
    }

    g_gameStats.ResetCurrentStats();

    // プレイヤーの行動をすべて停止する
    g_player.isMoving = false;
    g_player.isDashing = false;
    g_player.isCharging = false;
    g_player.isInDashAftermath = false;
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;

    Audio::PlaySE(SoundEffect::DEATH);
    // 必要ならここに死亡音を追加できる
    // g_audioManager.PlaySFX("death_sound.wav");
}

// 追加: プレイヤーが死亡すべきか確認する
void CheckPlayerDeath() {
    if (g_player.isDead || g_player.isInvincible) {
        return;
    }

    // 生存中の敵すべてとの衝突を確認する
    for (auto& enemy : g_enemies) {
        if (!enemy->IsAlive()) continue;

        // 接触ダメージ条件を含む敵側の衝突ロジックを使う
        if (enemy->CheckPlayerCollision()) {
            // ダッシュ中なら死亡せず、その代わり敵へ攻撃する
            if (g_player.isDashing) {
                // 攻撃処理は CheckDashAttack 側で行う
                continue;
            }
            else {
                // それ以外ならプレイヤーは死亡する
                OnPlayerDeath();
            }
            return;
        }
    }

}

// UpdateDash 改良版: チャージ減衰更新を追加
void UpdateDash(float deltaTime) {

    // ダッシュ状態更新を優先する
    if (g_player.isDashing) {
        g_player.dashTimer -= deltaTime;

        if (g_player.dashTimer <= 0.0f) {
            g_player.isDashing = false;
            g_player.hasMouseTarget = false;

            // ダッシュ終了後に短い無敵時間を与える
            g_player.isInvincible = true;
            g_player.invincibleTimer = std::max<float>(g_player.invincibleTimer, 0.2f);

            // 要望対応: ダッシュ終了後、実時間ベースのスローモーションへ入る。
            // この間プレイヤーは無敵とする。
            g_player.isInDashEndSlowMo = true;
            g_player.dashEndSlowMoTimer = g_player.DASH_END_SLOWMO_REALTIME;
            g_player.invincibleTimer = std::max(g_player.invincibleTimer, g_player.DASH_END_SLOWMO_REALTIME);

            EnterDashAftermath(); // ダッシュ終了時に後硬直へ入る
        }
    }

    // ダッシュ終了後スローモーションのタイマーを実時間で更新する（非スケール）。
    if (g_player.isInDashEndSlowMo) {
        g_player.dashEndSlowMoTimer -= deltaTime;
        if (g_player.dashEndSlowMoTimer <= 0.0f) {
            g_player.isInDashEndSlowMo = false;
            g_player.dashEndSlowMoTimer = 0.0f;
        }
    }

    // その後で後硬直状態を更新する
    UpdateDashAftermath(deltaTime);
    // ダッシュポイント回復システムを更新する
    UpdateDashPoints(deltaTime);

    // チャージ減衰タイマーを更新する
    g_player.UpdateChargeDecay(deltaTime);

    // チャージ処理は後硬直状態と独立して動作させる
    if (g_player.isCharging) {
        const float prevChargeTime = g_player.chargeTime;
        g_player.chargeTime += deltaTime * g_player.GetChargeSpeedMultiplier();

        // 硬直中でもチャージは可能だが、上限時間は超えられない
        if (g_player.chargeTime >= g_player.MAX_CHARGE_TIME) {
            // チャージ最大時、後硬直中なら先に後硬直を解除する
            /*if (g_player.isInDashAftermath) {
                g_player.isInDashAftermath = false;
            }
            ExecuteMouseChargeDash();*/
            g_player.chargeTime = g_player.MAX_CHARGE_TIME; // 最大で打ち止め。クリックを離すまで発動しない
        }

        // === チャージ消費ポイント表示の調整 ===
        // 現在はチャージ時間に関係なく、予約消費は常に 1 点だけ表示する（follower 1 個だけ強調）。
        // 段階ごとの効果音は鳴らさず、最大到達時だけ一度再生する。
        if (!(g_player.isInvincible && g_player.isGaugeInvincible)) {
            g_player.isChargeCostHighlight = true;

            // 消費予定 1 点だけを表示し、多段の予約消費表示を避ける
            int maxAllowedPending = std::min(g_player.dashPoints, g_player.MAX_DASH_POINTS);
            g_player.chargePendingCost = (maxAllowedPending > 0) ? 1 : 0;
            // 予約消費ポイントを段階的に増やすタイマーは使わないため、残留防止でリセットする
            g_player.chargeCostTimer = 0.0f;
        }
        else {
            g_player.isChargeCostHighlight = false;
            g_player.chargePendingCost = 0;
            g_player.chargeCostTimer = 0.0f;
        }

        // 最大チャージ到達時に SE を 1 回だけ再生する
        if (prevChargeTime < g_player.MAX_CHARGE_TIME && g_player.chargeTime >= g_player.MAX_CHARGE_TIME) {
            Audio::PlaySE(SoundEffect::CHARGE_START);
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
    // 死亡中は移動入力を無効にする
    if (g_player.isDead) {
        return;
    }
    // チャージ中で移動不可設定なら処理しない
    if (g_player.isCharging && !g_player.allowMoveWhileCharging) {
        return;
    }

    // 後硬直中なら移動で中断する
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }
    // 移動時はダッシュ終了後スローモーションも解除する
    if (g_player.isInDashEndSlowMo) {
        g_player.isInDashEndSlowMo = false;
        g_player.dashEndSlowMoTimer = 0.0f;
    }

    g_player.velocityX = -MOVE_SPEED * g_player.GetMoveSpeedMultiplier();
    g_player.isMoving = true;
    g_player.facingRight = false;
}

void MovePlayerRight() {
    // 死亡中は移動入力を無効にする
    if (g_player.isDead) {
        return;
    }
    // チャージ中で移動不可設定なら処理しない
    if (g_player.isCharging && !g_player.allowMoveWhileCharging) {
        return;
    }

    // 後硬直中なら移動で中断する
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }
    // 移動時はダッシュ終了後スローモーションも解除する
    if (g_player.isInDashEndSlowMo) {
        g_player.isInDashEndSlowMo = false;
        g_player.dashEndSlowMoTimer = 0.0f;
    }

    g_player.velocityX = MOVE_SPEED * g_player.GetMoveSpeedMultiplier();
    g_player.isMoving = true;
    g_player.facingRight = true;
}

void StopPlayer() {
    if (g_player.isDead) {
        g_player.velocityX = 0.0f;
        g_player.isMoving = false;
        return;
    }
    if (!g_player.isDashing) {
        g_player.velocityX = 0.0f;
    }
    g_player.isMoving = false;
}

// 改良版ジャンプ関数
void Jump() {
    if (g_player.isDead) {
        return;
    }
    if (g_player.isDashing || g_player.isCharging) {
        return;
    }

    // 壁ジャンプ: 壁滑り中なら反対方向へ跳ぶ。
    if (g_player.isWallSliding && g_player.wallSlideDirection != 0) {
        g_player.isWallSliding = false;

        g_player.velocityY = JUMP_FORCE;
        // 壁から離す: wallSlideDirection は左壁で -1、右壁で +1。
        // 逆方向へ跳びたいので符号を反転して使う。
        g_player.velocityX = (-static_cast<float>(g_player.wallSlideDirection)) * MOVE_SPEED * 1.5f;
        g_player.facingRight = (g_player.wallSlideDirection == -1);
        g_player.isOnGround = false;
        return;
    }

    // 通常ジャンプ
    if (g_player.isOnGround) {
        g_player.velocityY = JUMP_FORCE;
        g_player.isOnGround = false;
    }
}

// 方法 3: マウス方向ダッシュ
void DashToMouse() {
    if (g_player.isDead) {
        return;
    }
    // ポイントが足りるか確認する
    if (g_player.dashPoints <= 0) {
        return;
    }

    // ダッシュポイントを消費する
    if (!ConsumeDashPoint()) {
        return;
    }

    // このダッシュで 1 点消費したことを記録する（ダメージ倍率計算用）
    g_player.lastDashConsumedPoints = 1;

	Audio::PlaySE(SoundEffect::DASH);

    g_mouseIndicator.showArrow(false);
    // マウスのワールド座標を取得する
    float mouseX, mouseY;
    g_inputSystem.GetMousePosition(mouseX, mouseY);

    // プレイヤーからマウスへの方向ベクトルを計算する
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dirX = mouseX - playerCenterX;
    float dirY = mouseY - playerCenterY;

    // 方向ベクトルを正規化する
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }
    else {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
        dirY = 0.0f;
    }

    // ダッシュ状態を設定する
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION;
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // 斬撃中はダッシュ方向へ向きを合わせる
    if (fabsf(dirX) > 1e-4f) {
        g_player.facingRight = (dirX >= 0.0f);
    }

    // ダッシュ速度を設定する
    // 基本（非チャージ）ダッシュの最短距離を 1.3 倍にする
    g_player.velocityX = dirX * DASH_SPEED * 1.3f;
    g_player.velocityY = dirY * DASH_SPEED * 1.3f;

    // マウス目標位置を保存する
    g_player.mouseTargetX = mouseX;
    g_player.mouseTargetY = mouseY;
    g_player.hasMouseTarget = true;
}
// 修正版 StartMouseChargeDash 関数
void StartMouseChargeDash() {
    if (g_player.isDead) {
        return;
    }
    // 条件: ダッシュ中でない、チャージ中でない、ポイントがある、行動可能である
    if (g_player.isDashing || g_player.isCharging || g_player.dashPoints <= 0) {
        return;
    }

    g_mouseIndicator.showArrow(true);
    g_player.isCharging = true;
    g_inputSystem.GetMousePosition(g_player.mouseTargetX, g_player.mouseTargetY);
    g_player.hasMouseTarget = true;

    // チャージ時間をリセットし、今回のチャージを 0 から開始する
    g_player.chargeTime = 0.0f;
    // 今回のチャージでのポイント消費累積をリセットする
    g_player.chargeCostTimer = 0.0f;
    g_player.isChargeCostHighlight = true;
    // 押下直後に 1 点を予約消費する（ポイントがあれば）
    if (!(g_player.isInvincible && g_player.isGaugeInvincible) && g_player.dashPoints > 0) {
        g_player.chargePendingCost = 1;
    }
    else {
        g_player.chargePendingCost = 0;
    }
    // 保存済みチャージは消さず、今回のチャージだけ新規に開始する
}

// 修正版 ExecuteMouseChargeDash 関数
void ExecuteMouseChargeDash() {
    if (g_player.isDead) {
        return;
    }
    if (!g_player.isCharging) return;

    if (g_player.dashPoints <= 0) return;

    g_mouseIndicator.showArrow(false);
	Audio::PlaySE(SoundEffect::CHARGE_RELEASE);
    // 新しいダッシュを許可するため後硬直状態を解除する
    if (g_player.isInDashAftermath) {
        g_player.isInDashAftermath = false;
    }

    // ダッシュ開始時はダッシュ終了後スローモーションを中断する
    if (g_player.isInDashEndSlowMo) {
        g_player.isInDashEndSlowMo = false;
        g_player.dashEndSlowMoTimer = 0.0f;
    }

    g_player.hitEnemies.clear();
    // チャージ消費を確定する:
    // 調整: チャージ時間に関係なく消費は常に 1 点（他のチャージ効果は維持）
    // - ゲージ無敵中はポイントを消費しない
    int costToConsume = 1;
    costToConsume = std::clamp(costToConsume, 0, g_player.MAX_DASH_POINTS);

    if (!(g_player.isInvincible && g_player.isGaugeInvincible)) {
        if (g_player.dashPoints < costToConsume) {
            // ポイント不足: 発動せず、チャージをキャンセルする
            g_player.isCharging = false;
            g_player.chargeTime = 0.0f;
            g_player.chargePendingCost = 0;
            g_player.chargeCostTimer = 0.0f;
            g_player.isChargeCostHighlight = false;
            return;
        }
        g_player.dashPoints -= costToConsume;
    }

    // このダッシュで消費したポイント数を記録する（最低 1）
    g_player.lastDashConsumedPoints = std::max(1, costToConsume);

    // 現在のマウス位置を取得する
    float currentMouseX, currentMouseY;
    g_inputSystem.GetMousePosition(currentMouseX, currentMouseY);

    // プレイヤーからマウスへの方向を計算する
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    float dirX = currentMouseX - playerCenterX;
    float dirY = currentMouseY - playerCenterY;

    // 方向を正規化する
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
    }
    else {
        dirX = g_player.facingRight ? 1.0f : -1.0f;
        dirY = 0.0f;
    }

    // === 重要変更: 離散段階ではなくチャージ時間ベースでダッシュ距離を計算する ===
    float effectiveChargeForDash = 0.0f;
    // 短押し連携: 短押しで保存済みチャージがあるなら savedChargeTime を使う
    if (g_player.chargeTime < g_player.CHARGE_THRESHOLD_LOW) {
        if (g_player.hasSavedCharge) {
            effectiveChargeForDash = g_player.savedChargeTime;
            // 減衰タイマーを更新する
            g_player.chargeDecayTimer = g_player.CHARGE_DECAY_TIME;
            // ヒットストップなど他ロジック用に savedCharge を chargeTime に反映する
            g_player.chargeTime = g_player.savedChargeTime;
        }
        else {
            effectiveChargeForDash = g_player.chargeTime;
        }
    }
    else {
        // 長押し: 現在のチャージ時間を使い、十分長ければ保存する
        effectiveChargeForDash = g_player.chargeTime;
        if (g_player.chargeTime >= g_player.MIN_CHARGE_TIME) {
            g_player.SaveCharge();
        }
        else {
            g_player.ClearSavedCharge();
        }
    }

    // [0,1] に正規化する
    float normalizedCharge = std::clamp(effectiveChargeForDash / g_player.MAX_CHARGE_TIME, 0.0f, 1.0f);
    // 最大チャージ時の速度倍率（従来のレベル 3 挙動に合わせる）
    const float MAX_SPEED_MULT = 2.0f;
    float speedMultiplier = 1.0f + normalizedCharge * (MAX_SPEED_MULT - 1.0f);
    float durationMultiplier = 1.0f; // keep duration fixed for now
    
    // ダッシュ状態を設定する
    g_player.isDashing = true;
    g_player.dashTimer = DASH_DURATION * durationMultiplier;
    g_player.dashDirectionX = dirX;
    g_player.dashDirectionY = dirY;

    // 斬撃中はダッシュ方向へ向きを合わせる
    if (fabsf(dirX) > 1e-4f) {
        g_player.facingRight = (dirX >= 0.0f);
    }

    // ダッシュ速度を適用する: 最長距離は維持しつつ、最短距離だけ 1.3 倍に延ばす
    // チャージ段階に応じて基本速度へ speedMultiplier を掛ける。
    // 最小（非チャージ）ダッシュではさらに 1.3 倍して最短距離を伸ばす。
    float baseSpeed = DASH_SPEED * speedMultiplier;
    // このダッシュが 1 ポイント消費だけなら（短距離ダッシュ）、さらに 1.3 倍する
    if (g_player.lastDashConsumedPoints <= 1) {
        baseSpeed *= 1.3f;
    }
    g_player.velocityX = dirX * baseSpeed;
    g_player.velocityY = dirY * baseSpeed;

    // 新しいマウス目標位置を保存する
    g_player.mouseTargetX = currentMouseX;
    g_player.mouseTargetY = currentMouseY;

    // チャージ状態を終了する
    g_player.isCharging = false;
    g_player.chargeTime = 0.0f;
    g_player.chargePendingCost = 0;
    g_player.chargeCostTimer = 0.0f;
    g_player.isChargeCostHighlight = false;
}
// ダッシュ後硬直状態へ入る
void EnterDashAftermath() {
    // 後硬直に入る前に、終点小円の攻撃判定を 1 回だけ追加で行う。
    // 注意: ここではまだ isDashing=true のため、hitEnemies により重複ダメージは起こらない。
    PerformDashEndCircleHitTest();

    // プレイヤーを静止させるため速度をすべてクリアする
    g_player.velocityX = 0.0f;
    g_player.velocityY = 0.0f;

    // ポイントが残っていないなら後硬直へ入らない
    if (g_player.dashPoints <= 0) {
        g_player.ClearSavedCharge();
        return;
    }

    g_player.isInDashAftermath = true;
    g_player.dashAftermathTimer = g_player.DASH_AFTERMATH_DURATION;
}

// 後硬直状態を更新する
void UpdateDashAftermath(float deltaTime) {
    if (!g_player.isInDashAftermath) return;

    g_player.dashAftermathTimer -= deltaTime;

    // 移動入力による中断を確認する
    if (g_inputSystem.IsMovingLeft() || g_inputSystem.IsMovingRight()) {
        g_player.isInDashAftermath = false;
        g_player.velocityY = 0.0f;
        return;
    }

    // 後硬直終了
    if (g_player.dashAftermathTimer <= 0.0f) {
        g_player.isInDashAftermath = false;
        g_player.velocityY = 0.0f;
    }
}

// ダッシュポイント回復を更新する
void UpdateDashPoints(float deltaTime) {
    // 地上でのポイント回復:
    // - isOnGround が一定時間続いたあとで回復開始
    // - 開始後は一定間隔ごとに 1 点ずつ最大まで回復する
    constexpr float DASH_POINT_RECOVER_DELAY = 0.3f;
    constexpr float DASH_POINT_RECOVER_INTERVAL = 0.2f;

    // 重要修正: 短押しダッシュでは「着地しているのにダッシュ / チャージ関連状態が残る」ことがある。
    // そのままだとダッシュ中も地上回復タイマーが進み、常に回復しているように見える。
    // そのためダッシュ関連状態中は回復を禁止し、タイマーもリセットする。
    if (g_player.isDashing || g_player.isCharging || g_player.isInDashAftermath || g_player.isInDashEndSlowMo) {
        g_player.dashPointRecoverTimer = 0.0f;
        return;
    }

    // 地上、または壁滑り中なら回復を許可する。
    // 以前は地上限定だったが、壁滑りも有効な回復状態として扱う。
    if (!(g_player.isOnGround || g_player.isWallSliding) || g_player.dashPoints >= g_player.MAX_DASH_POINTS) {
        g_player.dashPointRecoverTimer = 0.0f;
        return;
    }

    g_player.dashPointRecoverTimer += deltaTime;

    if (g_player.dashPointRecoverTimer <= DASH_POINT_RECOVER_DELAY) {
        return;
    }

    float timeAfterDelay = g_player.dashPointRecoverTimer - DASH_POINT_RECOVER_DELAY;
    int pointsToRecover = static_cast<int>(floorf(timeAfterDelay / DASH_POINT_RECOVER_INTERVAL));
    if (pointsToRecover <= 0) {
        return;
    }

    int missing = g_player.MAX_DASH_POINTS - g_player.dashPoints;
    int actualRecover = std::min(pointsToRecover, missing);
    g_player.dashPoints += actualRecover;
    if (actualRecover > 0) {
        Audio::PlaySE(SoundEffect::SLASHCOUNT);
    }

    // 余り時間を保持し、「一定間隔ごと 1 回」の安定したテンポを維持する
    float leftover = fmodf(timeAfterDelay, DASH_POINT_RECOVER_INTERVAL);
    g_player.dashPointRecoverTimer = DASH_POINT_RECOVER_DELAY + leftover;
}

// ダッシュポイントを消費する
bool ConsumeDashPoint() {
    // ゲージ由来の無敵中は、ダッシュ / 斬撃は無料（ポイント消費なし）。
    if (g_player.isInvincible && g_player.isGaugeInvincible) {
        return true;
    }

    if (g_player.dashPoints > 0) {
        g_player.dashPoints--;
        return true;
    }
    return false;
}

// 敵撃破時のキル処理
void OnEnemyDefeated() {
    // 統計用にキルを記録する
    g_gameStats.IncrementKills();
}

void OnEnemyDefeated(bool wasWeakPointKill, float enemyWorldX, float enemyWorldY) {
    // 統計用にキルを記録する
    if (wasWeakPointKill) {
        g_gameStats.IncrementWeakPointKills();  // 30 点
        g_gameStats.AddScore(30);
        printf("[POINTS] Weak kill +30 → total now = %d\n", g_gameStats.GetTotalEnemyPoints());

        // 弱点撃破時の視覚フィードバックを強化する
        SpawnWeakPointKillEffect(enemyWorldX, enemyWorldY);

        // 演出強化のため斬撃ヒットストップを 1.5 倍にする
        if (g_player.hitStopTimer > 0.0f) {
            g_player.hitStopTimer *= 1.5f;
        }
        else {
            g_player.hitStopTimer = ComputeDashHitStopTime() * 1.5f;
        }
    }
    else {
        g_gameStats.IncrementKills();  // 10 点
    }

    // 撃破時にゲージポイントを付与するが、すでにゲージ無敵中なら増やさない。
    // これにより、効果中に追加キルを稼いで無敵時間を延長できないようにする。
    if (!(g_player.isInvincible && g_player.isGaugeInvincible)) {
        // 1 キルにつき常に 1 ゲージ。弱点撃破の追加ゲージはなくす。
        int gaugeGain = 1;
        g_player.gaugePoints += gaugeGain;
        if (g_player.gaugePoints > g_player.MAX_GAUGE_POINTS) {
            g_player.gaugePoints = g_player.MAX_GAUGE_POINTS;
        }
    }

    // ダッシュポイントを回復する
    if (g_player.dashPoints < g_player.MAX_DASH_POINTS) {
        g_player.dashPoints++;
        Audio::PlaySE(SoundEffect::SLASHCOUNT);
        g_gameStats.AddScore(10);
        printf("[POINTS] Normal kill +10 → total now = %d\n", g_gameStats.GetTotalEnemyPoints());

        // プレイヤー位置ではなく敵の死亡位置にインジケータを出す。
        // slash-count UI は DrawGame() 内で静的な補間状態を持つため、
        // 現在位置を死亡地点へ「瞬間移動」させ、そこからプレイヤー側へ戻るようにする。
        extern float g_slashCountSpawnX;
        extern float g_slashCountSpawnY;
        extern bool g_slashCountSpawnPending;
        g_slashCountSpawnX = enemyWorldX;
        g_slashCountSpawnY = enemyWorldY;
        g_slashCountSpawnPending = true;
    }

    g_player.comboCount++;
    g_player.comboTimer = 5.0f;

    // ゲージ撃破バースト用パーティクル演出
    // ゲージ無敵中は、赤いバーストをプレイヤー位置ではなく倒した敵の位置から出し、
    // エフェクトが敵由来に見えるようにする。
    if (g_player.isInvincible && g_player.isGaugeInvincible) {
        SpawnGaugeKillParticlesRed(enemyWorldX, enemyWorldY);
    }
}
