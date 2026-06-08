#include "Enemy.h"

void DamageNumberManager::AddDamageNumber(float x, float y, int damage, bool isCritical) {
    damageNumbers.emplace_back(x, y, damage, isCritical);
}

void DamageNumberManager::Update(float deltaTime) {
    for (auto it = damageNumbers.begin(); it != damageNumbers.end();) {
        it->timer += deltaTime;
        it->posY += it->velocityY * deltaTime;
        it->velocityY -= 2.0f * deltaTime; // 重力効果

        if (it->timer >= it->lifeTime) {
            it = damageNumbers.erase(it);
        }
        else {
            ++it;
        }
    }
}

void DamageNumberManager::Render(const Camera& camera) {
    // ダメージ数値の表示を一時的に無効化する。
    return;

    for (auto& number : damageNumbers) {
        float screenX, screenY;
        // 既存のワールド座標からスクリーン座標への変換を使う
        float cameraX = camera.GetX();
        float cameraY = camera.GetY();
        screenX = number.posX - cameraX;
        screenY = number.posY - cameraY;

        // アルファ値を計算する（フェードアウト効果）
        float alpha = 1.0f - (number.timer / number.lifeTime);

        // クリティカルかどうかで色を切り替える
        if (number.isCritical) {
            //SetColor(1.0f, 0.0f, 0.0f, alpha); // 赤いクリティカル表示
            SetColor(1.0f, 1.0f, 1.0f, alpha); // 白い通常表示
        }
        else {
            SetColor(1.0f, 1.0f, 1.0f, alpha); // 白い通常表示
        }

        // 既存の数字描画機能を使う
        RenderNumber(number.value, screenX, screenY, 0.07f, 0.1f, pTextureNum);
    }

    // 描画色をリセットする
    SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void DamageNumberManager::Clear() {
    damageNumbers.clear();
}