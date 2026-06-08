// ================================
//            TITLE
// ================================

#include "TitleScene.h"
#include "SceneManager.h" // シーン切り替え用

// コンストラクタ
TitleScene::TitleScene(SceneManager* manager) 
{
    sceneManager = manager;
	tex = nullptr;
}

// タイトル画面のオブジェクトを初期化する
bool TitleScene::Init() 
{
	SetInGameCursorEnabled(true);
    LoadTexture(g_pDevice, "asset/UI/title/UI_title_background_animation_v8.png", &tex);
    LoadTexture(g_pDevice, "asset/UI/title/padding_animation.png", &g_paddingTitleAnim);

    //InitGameWorld(); // main.cpp で追加した処理
    m_titleAnim.AddClip("titleScene",0,13,1,14, 0.08f, false, tex); // 1 フレーム 0.06 秒。小さいほど速い
    m_titleAnim.SetClip("titleScene");
    m_titleAnim.Pause(); // 最初は一時停止し、マウスクリックで再生する

   
    paddingTitleAnim.AddClip("padddingAnimation", 0, 13, 1, 14, 0.09f, true, g_paddingTitleAnim);
    paddingTitleAnim.SetClip("paddingAnimation");

    uiButtons.clear();

       
    return true;
}

// タイトル画面のオブジェクトを更新する
void TitleScene::Update(float deltaTime) 
{
    g_inputSystem.Update();

    if (!m_playing && g_inputSystem.IsMouseLeftDown()) 
    {
        m_playing = true;
        m_titleAnim.Resume();
    }

    m_titleAnim.Update(deltaTime);
    paddingTitleAnim.Update(deltaTime);

    // アニメーションが終わったらすぐに切り替える
    if (m_playing && m_titleAnim.IsFinished()) 
    {
        sceneManager->SwitchScene(MENU); // その後メニューシーンへ移動する
        return;
    }



}

// タイトル画面のオブジェクトを描画する
void TitleScene::Draw() 
{

    ID3D11ShaderResourceView* tex = m_titleAnim.GetCurrentClipTexture();
    if (tex) 
    {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, tex, m_titleAnim.GetCurrentFrame(), m_titleAnim.GetSplitX(), m_titleAnim.GetSplitY());
    }

    ID3D11ShaderResourceView* paddingTex = paddingTitleAnim.GetCurrentClipTexture();
    if (paddingTex) {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, paddingTex,
           paddingTitleAnim.GetCurrentFrame(),
           paddingTitleAnim.GetSplitX(),
           paddingTitleAnim.GetSplitY());
    }
}

// タイトル画面のオブジェクトを解放する
void TitleScene::Uninit() 
{
    if (tex)
    {
        tex->Release();
        tex = nullptr;
    }

    if (g_paddingTitleAnim)
    {
        g_paddingTitleAnim->Release();
        g_paddingTitleAnim = nullptr;
    }

    // アニメーション用テクスチャを解放する
    m_titleAnim.ClearClips();
    paddingTitleAnim.ClearClips();

    // ボタンを解放する
    uiButtons.clear();
    g_mouseIndicator.Cleanup();
}