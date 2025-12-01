#include "Menu.h"

MenuScene::MenuScene(SceneManager* manager)
{
	sceneManager = manager;
}

bool MenuScene::Init()
{
	LoadTexture(g_pDevice, "asset/menu.png", &backgroundTexture);      // abckground texture

	return true;
}

void MenuScene::Update(float deltaTime)
{
	g_inputSystem.Update();

	// it goes to the stage scene
	if (g_inputSystem.IsKeyDown(VK_S))   // at the end you will use mouse
	{
		sceneManager->SwitchScene(STAGE); // it goes to the stage scene
	}

	// it goes to the how to play scene
	if (g_inputSystem.IsKeyDown(VK_C))   // at the end you will use mouse
	{
		sceneManager->SwitchScene(HOWTOPLAY); // it goes to the stage scene
	}
}

void MenuScene::Draw()
{
	if (backgroundTexture) {
		// Always set a color before drawing so the texture is visible
		SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		RenderImage(-1.0f, -1.0f, 2.0f, 2.0f, backgroundTexture, 0, 1, 1);
	}
}

void MenuScene::Uninit()
{
	if (backgroundTexture)
	{
		backgroundTexture->Release();
		backgroundTexture = nullptr;
	}
}
