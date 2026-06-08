#pragma once
// Projectile.h
#include "Enemy.h"
#include "Game.h"
#include "Map.h"
#include <vector>

class Enemy;
// 射弾タイプ列挙
enum class ProjectileType {
    FIREBALL,       // ファイアボール - 直進し、命中で爆発する
    ICE_SHARD,      // 氷片 - 直進し、減速効果を与える
    MAGIC_MISSILE,  // 魔法弾 - 敵を追尾する
    LIGHTNING,      // 雷撃 - 短時間だけ存在する
    POISON_DART,    // 毒ダーツ - 継続ダメージを与える
    HOLY_BOLT,       // 聖光弾 - アンデッドに特効
    BULLET
};

// 射弾効果構造体
struct ProjectileEffect {
    float damage = 10.0f;
    float burnDamage = 0.0f;      // 燃焼の継続ダメージ
    float slowEffect = 0.0f;      // 減速効果 (0-1)
    float stunDuration = 0.0f;    // スタン時間
    bool pierce = false;          // 貫通するかどうか
    int maxPierceCount = 0;       // 最大貫通数
    float areaRadius = 0.0f;      // 範囲爆発半径
};

// 射弾クラス
class Projectile {
public:
    Projectile(ProjectileType type, float startX, float startY,
        float targetX, float targetY, float speed,
        const ProjectileEffect& effect, bool fromPlayer = true);

    void Update(float deltaTime, MapManager* mapManager, std::vector<Enemy*>& enemies);
    void Render(const Camera& camera);
    bool IsActive() const { return isActive; }
    void Deactivate() { isActive = false; }

    // 任意の矩形との衝突を確認する（プレイヤーの斬撃で射弾を消すため）
    bool CheckCollisionWithRect(float rectX, float rectY, float rectW, float rectH) const;
    // プレイヤーがこの射弾に命中したときに呼ばれる
    void OnHitByPlayer();
    // 敵性射弾で、かつプレイヤーを狙っている場合に true を返す
    bool IsHostileAndAimedAtPlayer() const;

    // 射弾情報を取得する
    float GetDamage() const { return effect.damage; }
    bool IsFromPlayer() const { return fromPlayer; }
    ProjectileType GetType() const { return type; }

private:
    // 射弾属性
    ProjectileType type;
    float posX, posY;
    float velocityX, velocityY;
    float speed;
    float lifeTime;
    float maxLifeTime = 5.0f;
    bool isActive;
    bool fromPlayer;

    // 見た目関連
    float size = 0.5f;
    float rotation;
    float scaleEffect;

    // 射弾効果
    ProjectileEffect effect;

    // 追尾関連
    Enemy* homingTarget;
    float homingStrength = 0.0f;
    int currentPierceCount;

    // 補助メソッド
    void Move(float deltaTime);
    bool CheckMapCollision(MapManager* mapManager);
    void CheckEnemyCollision(std::vector<Enemy*>& enemies);
    void CheckPlayerCollision();
    void ApplyEffectToEnemy(Enemy* enemy);
    void CreateImpactEffect();

    float CalculateDirectionAngle()const;
    float GetRotationAngle()const;
    void SetRotation(float r);
    // タイプ固有の挙動
    void UpdateFireball(float deltaTime);
    void UpdateBullet(float deltaTime);
    void UpdateIceShard(float deltaTime);
    void UpdateMagicMissile(float deltaTime, std::vector<Enemy*>& enemies);
    void UpdateLightning(float deltaTime);
    void UpdatePoisonDart(float deltaTime);
    void UpdateHolyBolt(float deltaTime);
};

// 射弾管理クラス
class ProjectileManager {
public:
    static ProjectileManager& GetInstance();

    void AddProjectile(ProjectileType type, float startX, float startY,
        float targetX, float targetY, float speed,
        const ProjectileEffect& effect, bool fromPlayer = true);

    void Update(float deltaTime, MapManager* mapManager, std::vector<Enemy*>& enemies);
    void Render(const Camera& camera);
    void ClearAll();

    // プレイヤーの斬撃で敵射弾に当たり判定を行うときに呼ばれる
    void HandlePlayerSlashHitRect(float rectX, float rectY, float rectW, float rectH);
    void HandlePlayerSlashHitCircle(float centerX, float centerY, float radius);

    // 補助関数: 定義済み効果の射弾を生成する
    void CreateFireball(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreateIceShard(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreateMagicMissile(float startX, float startY, Enemy* target, bool fromPlayer = true);
    void CreateLightningStrike(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreatePoisonDart(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreateHolyBolt(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    void CreateBullet(float startX, float startY, float targetX, float targetY, bool fromPlayer = true);
    ID3D11ShaderResourceView* GetTextureForType(ProjectileType type);
    void LoadTextures(ID3D11Device* device);

private:
    ProjectileManager() = default;
    std::vector<Projectile> projectiles;

    // 射弾テクスチャ
    ID3D11ShaderResourceView* fireballTexture = nullptr;
    ID3D11ShaderResourceView* bulletTexture = nullptr;
    ID3D11ShaderResourceView* iceShardTexture = nullptr;
    ID3D11ShaderResourceView* magicMissileTexture = nullptr;
    ID3D11ShaderResourceView* lightningTexture = nullptr;
    ID3D11ShaderResourceView* poisonDartTexture = nullptr;
    ID3D11ShaderResourceView* holyBoltTexture = nullptr;
};