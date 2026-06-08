#pragma once
#include "SceneBase.h"
#include "SceneManager.h"
#include "Game.h"
#include "Enemy.h"
#include "Render.h"
#include <vector>

// 11 月 14 日追加
class CakeScene : public SceneBase
{
private:
    struct CakePoint {
        float u;
        float v;
    };

    struct CakePiece {
        std::vector<LinearClipPlane> clipPlanes;
    };

    SceneManager* sceneManager;

    ID3D11ShaderResourceView* cakeTexture = nullptr;
    ID3D11ShaderResourceView* plateTexture = nullptr;

    std::vector<UIButton> uiButtons;
    ID3D11ShaderResourceView* continueTexture = nullptr;
    ID3D11ShaderResourceView* continueHoverTexture = nullptr;

    bool isCakeCut = false;
    bool isPlateShown = false;
    float cutAnimTimer = 0.0f;
    float worldSplitProgress = 0.0f;
    bool wasAttackActive = false;
    const float CUT_ANIM_DURATION = 1.0f; // 最後のカット後、少し待ってから皿を表示する
    const float CUT_FEEDBACK_DURATION = 0.16f;
    const float WORLD_SPLIT_DISTANCE = 0.06f;
    const float PLATE_SPLIT_DISTANCE = 0.12f;

    // ケーキの位置とサイズ
    float cakeX = 0.0f;
    float cakeY = -0.5f;
    float cakeWidth = 0.4f;
    float cakeHeight = 0.4f;

    float cutAngle = 0.0f;    // 斬撃 / カットラインのワールド空間での角度
    float cutDirX = 1.0f;
    float cutDirY = 0.0f;
    float cutNormalX = 0.0f;
    float cutNormalY = 1.0f;
    float cutCenterU = 0.5f;
    float cutCenterV = 0.5f;
    std::vector<CakePiece> cakePieces;

public:
    CakeScene(SceneManager* manager);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;

private:
    bool CheckPlayerAttackHitsCake(); // プレイヤーの攻撃がケーキに当たったかを判定する
    void ResetCakePieces();
    bool CutCake();
    void DrawCakeSequence();
    float DetermineCutAngle() const;
    void UpdateCutVectors();
    bool GetAttackSegment(float& startX, float& startY, float& endX, float& endY) const;
    bool GetAttackSegmentUV(float& startU, float& startV, float& endU, float& endV) const;
    bool GetCakeEntryPoint(float& hitX, float& hitY) const;
    void UpdateCutCenterFromWorldPoint(float worldX, float worldY);
    LinearClipPlane BuildCurrentCutPlane() const;
    std::vector<CakePoint> BuildPiecePolygon(const CakePiece& piece) const;
    std::vector<CakePoint> ClipPolygonWithPlane(const std::vector<CakePoint>& polygon, const LinearClipPlane& plane) const;
    float ComputePolygonArea(const std::vector<CakePoint>& polygon) const;
    CakePoint ComputePolygonCentroid(const std::vector<CakePoint>& polygon) const;
    bool DoesSegmentHitPiece(float startU, float startV, float endU, float endV, const CakePiece& piece) const;
    void GetPiecePresentation(const CakePiece& piece, float splitOffset, float& offsetX, float& offsetY, float& rotation) const;
    void DrawCakePieces(float centerX, float centerY, float width, float height, float splitOffset);
};
