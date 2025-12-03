#include "HowToPlay.h"

HowToPlayScene::HowToPlayScene(SceneManager* manager, SCENE returnTo)
{
	sceneManager = manager;
	returnScene = returnTo;
}

bool HowToPlayScene::Init()
{
	LoadTexture(g_pDevice, "asset/howtoplay.png", &backgroundTexture);      // abckground texture

	LoadTexture(g_pDevice, "asset/block.png", &buttonTexture); // for the button

	uiButtons.clear();
	g_mouseIndicator.ShowMouseIndicator(false);

	uiButtons.emplace_back(0.8f, -0.8f, 0.3f, 0.1f, MENU, buttonTexture);

	return true;
}

void HowToPlayScene::Update(float deltaTime)
{
	g_inputSystem.Update();

	// it goes back to the menu scene
	//if (g_inputSystem.IsTogglePressed(VK_B)) // at the end you will use mouse
	//{
	//	sceneManager->SwitchScene(returnScene); 
	//}

	// for the buttons
	for (auto& btn : uiButtons)
	{
		if (btn.Process() == UIButtonResult::Clicked)
		{
			sceneManager->SwitchScene(returnScene);
			return;
		}
	}
}

void HowToPlayScene::Draw()
{
	if (backgroundTexture) {
		// Always set a color before drawing so the texture is visible
		SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, backgroundTexture, 0, 1, 1);
	}

	for (const auto& btn : uiButtons)
		btn.Draw(0.65f);
}

void HowToPlayScene::Uninit()
{
	if (backgroundTexture)
	{
		backgroundTexture->Release();
		backgroundTexture = nullptr;
	}

	if (buttonTexture) {
		buttonTexture->Release();
		buttonTexture = nullptr;
	}

	uiButtons.clear();
	g_mouseIndicator.Cleanup();
}
