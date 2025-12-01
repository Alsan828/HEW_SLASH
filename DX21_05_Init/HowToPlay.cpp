#include "HowToPlay.h"

HowToPlayScene::HowToPlayScene(SceneManager* manager, SCENE returnTo)
{
	sceneManager = manager;
	returnScene = returnTo;
}

bool HowToPlayScene::Init()
{
	LoadTexture(g_pDevice, "asset/howtoplay.png", &backgroundTexture);      // abckground texture

	return true;
}

void HowToPlayScene::Update(float deltaTime)
{
	g_inputSystem.Update();

	// it goes back to the menu scene
	if (g_inputSystem.IsTogglePressed(VK_B)) // at the end you will use mouse
	{
		sceneManager->SwitchScene(returnScene); 
	}
}

void HowToPlayScene::Draw()
{
	if (backgroundTexture) {
		// Always set a color before drawing so the texture is visible
		SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, backgroundTexture, 0, 1, 1);
	}
}

void HowToPlayScene::Uninit()
{
	if (backgroundTexture)
	{
		backgroundTexture->Release();
		backgroundTexture = nullptr;
	}
}
