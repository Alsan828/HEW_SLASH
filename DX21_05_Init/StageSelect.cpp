#include "StageSelect.h"


StageSelect::StageSelect(SceneManager* manager, SCENE returnTo)
{
	backgroundTexture = nullptr;
	sceneManager = manager;
	returnScene = returnTo;
}


bool StageSelect::Init()
{
	LoadTexture(g_pDevice, "asset/stageselect.png", &backgroundTexture);      // abckground texture

	// for test now
	LoadTexture(g_pDevice, "asset/UI/button_normal.png", &buttonTexture); // for the button
	LoadTexture(g_pDevice, "asset/UI/button_hover.png", &buttonHoverTexture);

	
	//uiButtons.emplace_back(-0.65f, 0.1f, 0.4f, 0.8f, STAGE, buttonTexture, buttonHoverTexture); // go to 1-1
	//uiButtons.emplace_back(-0.25f, 0.1f, 0.4f, 0.8f, STAGE2, buttonTexture, buttonHoverTexture); // go to 1-2
	//uiButtons.emplace_back(0.15f, 0.1f, 0.4f, 0.8f, STAGE3, buttonTexture, buttonHoverTexture); // go to 1-3
	//uiButtons.emplace_back(0.65f, 0.1f, 0.4f, 0.8f, STAGE4, buttonTexture, buttonHoverTexture); // go to 1-4
	//uiButtons.emplace_back(-0.65f, -0.5f, 0.4f, 0.8f, STAGE5, buttonTexture, buttonHoverTexture); // go to 1-5
	//uiButtons.emplace_back(-0.25f, -0.5f, 0.4f, 0.8f, STAGE6, buttonTexture, buttonHoverTexture); // go to 1-6
	//uiButtons.emplace_back(0.15f, -0.5f, 0.4f, 0.8f, STAGE7, buttonTexture, buttonHoverTexture); // go to 1-6
	//uiButtons.emplace_back(0.65f, -0.5f, 0.4f, 0.8f, BOSS, buttonTexture, buttonHoverTexture); // go to boss

	uiButtons.emplace_back(-0.65f, 0.3f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // go to 1-1
	uiButtons.emplace_back(-0.25f, 0.3f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // go to 1-2
	uiButtons.emplace_back(0.15f, 0.3f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // go to 1-3
	uiButtons.emplace_back(0.65f, 0.3f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // go to 1-4
	uiButtons.emplace_back(-0.65f, -0.7f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // go to 1-5
	uiButtons.emplace_back(-0.25f, -0.7f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // go to 1-6
	uiButtons.emplace_back(0.15f, -0.7f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // go to 1-7
	uiButtons.emplace_back(0.65f, -0.7f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // go to boss
	for (auto& btn : uiButtons)
	{
		btn.SetHitboxScale(0.7f, 0.2f);
		btn.SetHitboxOffset(-0.05f);
	}

	// right arrow so I can go to stage select world 2
	uiButtons.emplace_back(0.9f, -0.2f, 0.3f, 0.2f, STAGESELECT2, buttonTexture, buttonHoverTexture);
	uiButtons.back().SetHitboxScale(0.15f, 0.15f);
	uiButtons.back().SetHitboxOffset(-0.04f);


	LoadTexture(g_pDevice, "asset/UI/back/back_normal.png", &backTexture); // for the button
	LoadTexture(g_pDevice, "asset/UI/back/back_hover.png", &backHoverTexture);

	uiButtons.emplace_back(0.8f, -0.9f, 0.4f, 0.8f, returnScene/*MENU*/, backTexture, backHoverTexture);
	uiButtons.back().SetHitboxScale(0.25f, 0.13f);  // change this values if needed depending on the size of the button
	uiButtons.back().SetHitboxOffset(-0.06f);

	//uiButtons.clear();
	g_mouseIndicator.ShowMouseIndicator(false);

	return true;
}

// new update stage select with the GAMEPLAY 
void StageSelect::Update(float deltaTime)
{
	g_inputSystem.Update();

	for (int i = 0; i < uiButtons.size(); i++)
	{
		if (uiButtons[i].Process() == UIButtonResult::Clicked)
		{
			// the first 8 buttons are for the stage
			if (i >= 0 && i < 8)
			{
				// for world 1
				sceneManager->SwitchToStage(1, i + 1);
				return;
			}
			// for the right arrow
			else if (i == 8)
			{
				sceneManager->SwitchScene(STAGESELECT2);
				return;
			}
			// back button
			else
			{
				sceneManager->SwitchScene(returnScene);
				return;
			}
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

	if (buttonTexture) 
	{
		buttonTexture->Release();
		buttonTexture = nullptr;
	}
	if (buttonHoverTexture)
	{
		buttonHoverTexture->Release();
		buttonHoverTexture = nullptr;
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
