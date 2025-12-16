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
    LoadTexture(g_pDevice, "asset/UI/title/title0.png", &tex);
    frames.push_back(tex);

    LoadTexture(g_pDevice, "asset/UI/title/cut/cut0.png", &tex);
    frames.push_back(tex);
    LoadTexture(g_pDevice, "asset/UI/title/cut/cut1.png", &tex);
    frames.push_back(tex);
    LoadTexture(g_pDevice, "asset/UI/title/cut/cut2.png", &tex);
    frames.push_back(tex);
    LoadTexture(g_pDevice, "asset/UI/title/cut/cut3.png", &tex);
    frames.push_back(tex);
    LoadTexture(g_pDevice, "asset/UI/title/cut/cut4.png", &tex);
    frames.push_back(tex);
    LoadTexture(g_pDevice, "asset/UI/title/cut/cut5.png", &tex);
    frames.push_back(tex);
    LoadTexture(g_pDevice, "asset/UI/title/cut/cut6.png", &tex);
    frames.push_back(tex);
    LoadTexture(g_pDevice, "asset/UI/title/cut/cut7.png", &tex);
    frames.push_back(tex);


    m_titleAnim.InitFromTextures(frames, 0.06f, false); // 0.06s per frame. lower number is faster


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

    m_titleAnim.UpdateTexture(deltaTime);

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

    ID3D11ShaderResourceView* tex = m_titleAnim.GetCurrentTexture();
    if (tex) 
    {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, tex, 0, 1, 1);
    }
}

//it erases the objects in title
void TitleScene::Uninit() 
{

    // clean up the texture for the animation
    m_titleAnim.CleanupTextures();

    // clean up the buttons
    uiButtons.clear();
    g_mouseIndicator.Cleanup();
}