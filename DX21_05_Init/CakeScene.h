#pragma once
#include "SceneBase.h"
#include "SceneManager.h"
#include "Game.h"
#include "Enemy.h"


// added November 14th
class CakeScene : public SceneBase
{
private:
    SceneManager* sceneManager;

    //ID3D11ShaderResourceView* cakeTexture = nullptr;
    //ID3D11ShaderResourceView* plateTexture = nullptr;
    //bool isCakeCut = false; // to check if I cut the cake or not
    //bool isPlateShown = false;
    //float cutTimer = 0.0f;

public:
    CakeScene(SceneManager* manager);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};