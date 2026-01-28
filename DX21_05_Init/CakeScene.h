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

    std::vector<UIButton> uiButtons;
    ID3D11ShaderResourceView* continueTexture = nullptr;
    ID3D11ShaderResourceView* continueHoverTexture = nullptr;

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


    // FOR THE CAKE WHEN CUTTING IT IN 8 SLICES
    float HALF_SLICE_DEG = 22.5f; // bc its 45 divided by 2
    // 45 bc there are 8 slices. divide 360 which is an entire cirlce by 8 and thats why you get 45

    float RIGHT = 0.0f;
    float TOP_RIGHT = 45.0f;
    float TOP = 90.0f;
    float TOP_LEFT = 135.0f;
    float LEFT = 180.0f;
    float BOTTOM_LEFT = 225.0f;
    float BOTTOM = 270.0f;
    float BOTTOM_RIGHT = 315.0f;

    float rightPartMax = RIGHT + HALF_SLICE_DEG;               // 22.5
    float topRightPartMax = TOP_RIGHT + HALF_SLICE_DEG;        // 67.5
    float topMiddlePartMax = TOP + HALF_SLICE_DEG;             // 112.5
    float topLeftPartMax = TOP_LEFT + HALF_SLICE_DEG;          // 157.5
    float leftPartMax = LEFT + HALF_SLICE_DEG;                 // 202.5
    float bottomLeftPartMax = BOTTOM_LEFT + HALF_SLICE_DEG;    // 247.5
    float bottomMiddlePartMax = BOTTOM + HALF_SLICE_DEG;       // 292.5
    float bottomRightPartMax = BOTTOM_RIGHT + HALF_SLICE_DEG;  // 337.5
 

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