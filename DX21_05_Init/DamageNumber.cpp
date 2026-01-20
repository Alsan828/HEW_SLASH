#include "Enemy.h"

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
    // Temporarily disable damage number display.
    return;

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