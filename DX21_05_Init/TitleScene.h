#pragma once
#include "SceneBase.h"
#include "InputSystem.h"
#include "SceneManager.h"
#include <vector>
#include "Render.h"
#include "UIButton.h"
//#include "Animation.h"

// title background structure
struct Background 
{
    float posX, posY;
    float width, height;
};

class TitleScene : public SceneBase 
{
private:
    SceneManager* sceneManager;   // pointer to the scene manager

    //ID3D11ShaderResourceView* backgroundTexture;

    // for the title intro animation
    std::vector<ID3D11ShaderResourceView*> frames;
    ID3D11ShaderResourceView* tex;

    Animation m_titleAnim;
    bool m_playing = false;


    std::vector<UIButton> uiButtons;

public:
    TitleScene(SceneManager* manager); // constructor

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Uninit() override;
};

extern InputSystem g_inputSystem;   // use the global input system
extern ID3D11Device* g_pDevice;     // device for texture loading
