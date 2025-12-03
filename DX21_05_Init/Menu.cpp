#include "Menu.h"

MenuScene::MenuScene(SceneManager* manager)
{
	sceneManager = manager;

}

bool MenuScene::Init()
{

	LoadTexture(g_pDevice, "asset/menu.png", &backgroundTexture);      // abckground texture

	LoadTexture(g_pDevice, "asset/block.png", &buttonTexture); // for the button

	uiButtons.clear();
	g_mouseIndicator.ShowMouseIndicator(false);

	uiButtons.emplace_back(-0.55f, -0.3f, 0.25f, 0.3f, STAGE, buttonTexture);
	uiButtons.emplace_back(0.0f, -0.3f, 0.25f, 0.3f, HOWTOPLAY, buttonTexture);
	uiButtons.emplace_back(+0.55f, -0.3f, 0.25f, 0.3f, QUIT_GAME, buttonTexture);


	return true;
}

void MenuScene::Update(float deltaTime)
{
	g_inputSystem.Update();

	for (auto& btn : uiButtons)
	{
		if (btn.Process() == UIButtonResult::Clicked)
		{
			if (btn.GetTargetScene() == QUIT_GAME)
			{
				PostQuitMessage(0); // it quits the game
			}
			else
			{
				sceneManager->SwitchScene(btn.GetTargetScene());
			}
			return;
		}
	}
}

void MenuScene::Draw()
{
	if (backgroundTexture) {
		// Always set a color before drawing so the texture is visible
		SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, backgroundTexture, 0, 1, 1);
	}

	// for the buttons
	for (const auto& btn : uiButtons)
		btn.Draw(0.65f);

}

void MenuScene::Uninit()
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
