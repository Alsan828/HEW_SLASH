#include "StageSelect.h"


StageSelect::StageSelect(SceneManager* manager)
{
	sceneManager = manager;
}


bool StageSelect::Init()
{

	LoadTexture(g_pDevice, "asset/stageselect.png", &backgroundTexture);      // abckground texture

	LoadTexture(g_pDevice, "asset/UI/button_normal.png", &buttonTexture); // for the button
	LoadTexture(g_pDevice, "asset/UI/button_hover.png", &buttonHoverTexture);

	uiButtons.clear();
	g_mouseIndicator.ShowMouseIndicator(false);

	uiButtons.emplace_back(-0.65f, 0.1f, 0.4f, 0.8f, STAGE, buttonTexture, buttonHoverTexture); // go to 1-1
	uiButtons.emplace_back(-0.25f, 0.1f, 0.4f, 0.8f, STAGE2, buttonTexture, buttonHoverTexture); // go to 1-2
	uiButtons.emplace_back(0.15f, 0.1f, 0.4f, 0.8f, STAGE3, buttonTexture, buttonHoverTexture); // go to 1-3

	uiButtons.emplace_back(0.8f, -0.9f, 0.4f, 0.8f, MENU, buttonTexture, buttonHoverTexture); // go back to menu


	for (auto& btn : uiButtons)
	{
		btn.SetHitboxScale(0.7f, 0.2f);
		btn.SetHitboxOffset(-0.05f);
	}


	return true;
}


void StageSelect::Update(float deltaTime)
{
	g_inputSystem.Update();

	// for the buttons
	for (auto& btn : uiButtons)
	{
		if (btn.Process() == UIButtonResult::Clicked)
		{
			sceneManager->SwitchScene(btn.GetTargetScene());
			return;
		}
	}
}


void StageSelect::Draw()
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


void StageSelect::Uninit()
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

	if (buttonHoverTexture)
	{
		buttonHoverTexture->Release();
		buttonHoverTexture = nullptr;
	}

	uiButtons.clear();
	g_mouseIndicator.Cleanup();
}
