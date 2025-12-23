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

	
	uiButtons.emplace_back(-0.65f, 0.1f, 0.4f, 0.8f, STAGE, buttonTexture, buttonHoverTexture); // go to 1-1
	uiButtons.emplace_back(-0.25f, 0.1f, 0.4f, 0.8f, STAGE2, buttonTexture, buttonHoverTexture); // go to 1-2
	uiButtons.emplace_back(0.15f, 0.1f, 0.4f, 0.8f, STAGE3, buttonTexture, buttonHoverTexture); // go to 1-3
	uiButtons.emplace_back(0.65f, 0.-0.5f, 0.4f, 0.8f, BOSS, buttonTexture, buttonHoverTexture); // go to boss
	for (auto& btn : uiButtons)
	{
		btn.SetHitboxScale(0.7f, 0.2f);
		btn.SetHitboxOffset(-0.05f);
	}



	LoadTexture(g_pDevice, "asset/UI/back/back_normal.png", &backTexture); // for the button
	LoadTexture(g_pDevice, "asset/UI/back/back_hover.png", &backHoverTexture);

	uiButtons.emplace_back(0.8f, -0.9f, 0.4f, 0.8f, returnScene/*MENU*/, backTexture, backHoverTexture);
	uiButtons.back().SetHitboxScale(0.25f, 0.13f);  // change this values if needed depending on the size of the button
	uiButtons.back().SetHitboxOffset(-0.06f);

	//uiButtons.clear();
	g_mouseIndicator.ShowMouseIndicator(false);

	return true;
}


void StageSelect::Update(float deltaTime)
{
	g_inputSystem.Update();

	// for the buttons
	for (auto& btn : uiButtons)
	{
		/*if (btn.Process() == UIButtonResult::Clicked)
		{
			sceneManager->SwitchScene(btn.GetTargetScene());
			return;
		}*/
		if (btn.Process() == UIButtonResult::Clicked) { // If this is the BACK button 仺 use returnScene 
			if (btn.GetTargetScene() == MENU || btn.GetTargetScene() == PAUSE) 
			{ 
				sceneManager->SwitchScene(returnScene); 
			} 
			else 
			{ // Stage buttons 仺 go to the stage 
				sceneManager->SwitchScene(btn.GetTargetScene()); 
			} 
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
