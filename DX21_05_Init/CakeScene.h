#pragma once
#include "SceneBase.h"
#include "SceneManager.h"
#include "Game.h"
#include "Enemy.h"

enum CutDirection 
{
    VERTICAL_CUT,
    HORIZONTAL_CUT,
    DIAGONAL_FROM_TOP_CUT,    // from top left to bottom right or viceversa
    DIAGONAL_FROM_BOTTOM_CUT  // from bottom left to top right or viceversa
};

// added November 14th
class CakeScene : public SceneBase
{
private:
    SceneManager* sceneManager;

    ID3D11ShaderResourceView* cakeTexture = nullptr;
    ID3D11ShaderResourceView* plateTexture = nullptr;

    bool isCakeCut = false;
    bool isPlateShown = false;
    float cutAnimTimer = 0.0f;
    const float CUT_ANIM_DURATION = 1.0f; // for how long it takes for the animation of the cake being cut
    const float SPLIT_DISTANCE = 0.3f;    // for checking how far the cake splits

    // Cake position and size
    float cakeX = 0.0f;
    float cakeY = -0.5f;
    float cakeWidth = 0.4f;
    float cakeHeight = 0.4f;

    CutDirection cutDirection;
 

public:
    CakeScene(SceneManager* manager);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;

    bool CheckPlayerAttackHitsCake(); // for checking if the player hit the cake or not
    void CutCake();
    void DrawCakeSequence();
    CutDirection DetermineCutDirection();
};