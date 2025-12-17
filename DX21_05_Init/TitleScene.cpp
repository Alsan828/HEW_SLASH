//================================
//      TITLE
//================================

#include "TitleScene.h"
#include "SceneManager.h" // for switching scenes

// construct
TitleScene::TitleScene(SceneManager* manager) 
{
    sceneManager = manager;
}

//it initializes the objects in title
bool TitleScene::Init() 
{
    LoadTexture(g_pDevice, "asset/UI/title/cut_sheet.png", &tex);

    m_titleAnim.AddClip("titleScene",0,8,1,9, 0.06f, false, tex); // 0.06s per frame. lower number is faster
	m_titleAnim.SetClip("titleScene");
	m_titleAnim.Pause(); // start paused, will play when mouse is clicked
    uiButtons.clear();
    g_mouseIndicator.ShowMouseIndicator(false);
       
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
}

//it erases the objects in title
void TitleScene::Uninit() 
{

    // clean up the texture for the animation
    m_titleAnim.ClearClips();

    // clean up the buttons
    uiButtons.clear();
    g_mouseIndicator.Cleanup();
}