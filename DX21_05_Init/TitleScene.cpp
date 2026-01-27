//================================
//      TITLE
//================================

#include "TitleScene.h"
#include "SceneManager.h" // for switching scenes

// construct
TitleScene::TitleScene(SceneManager* manager) 
{
    sceneManager = manager;
	tex = nullptr;
}

//it initializes the objects in title
bool TitleScene::Init() 
{
	SetInGameCursorEnabled(true);
    LoadTexture(g_pDevice, "asset/UI/title/UI_title_background_animation_v8.png", &tex);
    LoadTexture(g_pDevice, "asset/UI/title/padding_animation.png", &g_paddingTitleAnim);

    //InitGameWorld(); // I added this in the main.cpp
    m_titleAnim.AddClip("titleScene",0,13,1,14, 0.08f, false, tex); // 0.06s per frame. lower number is faster
	m_titleAnim.SetClip("titleScene");
	m_titleAnim.Pause(); // start paused, will play when mouse is clicked

   
    paddingTitleAnim.AddClip("padddingAnimation", 0, 13, 1, 14, 0.09f, true, g_paddingTitleAnim);
    paddingTitleAnim.SetClip("paddingAnimation");

    uiButtons.clear();

       
    return true;
}

//it updates the objects in tile
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

    // switch as soon as the animation finishes
    if (m_playing && m_titleAnim.IsFinished()) 
    {
        sceneManager->SwitchScene(MENU); // and then it goes to mneu scene
        return;
    }



}

//it draws the objects in title
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

//it erases the objects in title
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

    // clean up the texture for the animation
    m_titleAnim.ClearClips();
    paddingTitleAnim.ClearClips();

    // clean up the buttons
    uiButtons.clear();
    g_mouseIndicator.Cleanup();
}