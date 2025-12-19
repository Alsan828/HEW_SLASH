#include "HowToPlay.h"


HowToPlayScene::HowToPlayScene(SceneManager* manager, SCENE returnTo)
{
	sceneManager = manager;
	returnScene = returnTo;
}

bool HowToPlayScene::Init()
{
	LoadTexture(g_pDevice, "asset/UI/control/control.png", &backgroundTexture);      // abckground texture

	LoadTexture(g_pDevice, "asset/UI/back/back_normal.png", &backTexture); // for the button
	LoadTexture(g_pDevice, "asset/UI/back/back_hover.png", &backHoverTexture);

	uiButtons.emplace_back(0.8f, -0.9f, 0.4f, 0.8f, MENU, backTexture, backHoverTexture);
	uiButtons.back().SetHitboxScale(0.25f, 0.13f);  // change this values if needed depending on the size of the button
	uiButtons.back().SetHitboxOffset(-0.06f);


	//uiButtons.clear();
	g_mouseIndicator.ShowMouseIndicator(false);

	return true;
}

void HowToPlayScene::Update(float deltaTime)
{
	g_inputSystem.Update();

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

	if (backTexture) 
	{
		backTexture->Release();
		backTexture = nullptr;
	}
	if (backHoverTexture)
	{
		backHoverTexture->Release();
		backHoverTexture = nullptr;
	}

	uiButtons.clear();
	g_mouseIndicator.Cleanup();
}
