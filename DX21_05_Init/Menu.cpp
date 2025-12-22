#include "Menu.h"

MenuScene::MenuScene(SceneManager* manager)
{
	sceneManager = manager;

}

bool MenuScene::Init()
{

	LoadTexture(g_pDevice, "asset/UI/menu/background.png", &backgroundTexture);
	
	LoadTexture(g_pDevice, "asset/UI/menu/start_normal.png", &startTexture);
	LoadTexture(g_pDevice, "asset/UI/menu/start_hover.png", &startHoverTexture);

	LoadTexture(g_pDevice, "asset/UI/menu/control_normal.png", &controlTexture);
	LoadTexture(g_pDevice, "asset/UI/menu/control_hover.png", &controlHoverTexture);
	
	LoadTexture(g_pDevice, "asset/UI/menu/quit_normal.png", &quitTexture);
	LoadTexture(g_pDevice, "asset/UI/menu/quit_hover.png", &quitHoverTexture);


	// to do: make the stageselect part and move it to the left a bit
	// because each puase button size is different so the hitbox will be different as well.
	uiButtons.emplace_back(-0.7f, -0.3f, 0.5f, 0.9f, STAGESELECT, startTexture, startHoverTexture);
	uiButtons.back().SetHitboxScale(0.4f, 0.13f);
	uiButtons.back().SetHitboxOffset(-0.06f);

	uiButtons.emplace_back(-0.7f, -0.55f, 0.5f, 0.9f, HOWTOPLAY, controlTexture, controlHoverTexture);
	uiButtons.back().SetHitboxScale(0.48f, 0.13f);
	uiButtons.back().SetHitboxOffset(-0.06f);

	uiButtons.emplace_back(-0.7f, -0.8f, 0.5f, 0.9f, QUIT_GAME, quitTexture, quitHoverTexture);
	uiButtons.back().SetHitboxScale(0.47f, 0.13f);
	uiButtons.back().SetHitboxOffset(-0.06f);



	//uiButtons.clear();
	g_mouseIndicator.ShowMouseIndicator(false);

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
				CleanUpGameWorld();
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

	if (startTexture) { 
		startTexture->Release();      
		startTexture = nullptr; 
	}
	if (startHoverTexture)
	{
		startHoverTexture->Release();
		startHoverTexture = nullptr;
	}

	if (controlTexture)
	{
		controlTexture->Release();
		controlTexture = nullptr;
	}
	if (controlHoverTexture)
	{
		controlHoverTexture->Release();
		controlHoverTexture = nullptr;
	}

	if (quitTexture)
	{	
		quitTexture->Release();
		quitTexture = nullptr;
	}
	if (quitHoverTexture)
	{
		quitHoverTexture->Release();
		quitHoverTexture = nullptr;
	}

	uiButtons.clear();
	g_mouseIndicator.Cleanup();
}
