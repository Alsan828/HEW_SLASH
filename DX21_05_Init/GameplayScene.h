#pragma once
#include "SceneBase.h"
#include "StageInfo.h"
#include "SceneManager.h"
#include "Enemy.h"

// ボスエリアを含むすべてのエリア（全 8 エリア）で使う
class GameplayScene : public SceneBase
{
private:
    SceneManager* sceneManager;
    int worldNumber;  // ワールド番号（world1、2、3）
    int areaNumber;  // エリア番号（area1、area2、...、area8）
    bool isBossStage; // ボスステージかどうか

    // World1 前半エリア用のチュートリアルオーバーレイ
    bool m_showTutorial = false;
    int m_currentTutorialIndex = 0; // 表示するチュートリアル番号（1-4）
    bool m_tutorialTriggered[4] = { false, false, false, false }; // どのチュートリアルを表示済みかを管理する
    ID3D11ShaderResourceView* m_tutorialTexture = nullptr;
    ID3D11ShaderResourceView* m_tutorialButtonTexture = nullptr;
    ID3D11ShaderResourceView* m_tutorialButtonHoverTexture = nullptr;
    UIButton m_tutorialButton;

    // ボス用
    Enemy* m_boss;
    float m_bossCheckpointHP;   // ボス再出現時に HP バーをどこまで戻すかを保持する
    bool m_checkpoint1Reached;  // HP バーが 2/3 に到達したか
    bool m_checkpoint2Reached;  // HP バーが 1/3 に到達したか

    StageInfo stageInfo;

public:
    GameplayScene(SceneManager* manager, int world, int area);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;


    // ボス HP ロジック用
    void UpdateBossLogic(float deltaTime);
    void CheckBossCheckpoints();      // HP バーのチェックポイントに到達したかを確認する
    void RespawnBossAtCheckpoint();   // チェックポイントからボスを再出現させる
    void RenderBossHealthBar();       // ボス HP バーを描画する

    void CheckTutorialTriggers(); // プレイヤーがチュートリアルトリガーに触れたかを確認する
};