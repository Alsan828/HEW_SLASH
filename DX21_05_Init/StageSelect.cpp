#include "StageSelect.h"


StageSelect::StageSelect(SceneManager* manager, SCENE returnTo)
{
	backgroundTexture = nullptr;
	sceneManager = manager;
	returnScene = returnTo;
}


bool StageSelect::Init()
{
	LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_background.png", &backgroundTexture);      // abckground texture

	LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_stage_nomalsize.png", &buttonTexture); // for the button
	LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_stage_bigsize.png", &buttonHoverTexture);
	LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_Boss_nomalsize.png", &bossButtonTexture);
	LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_Boss_bigsize.png", &bossButtonHoverTexture);

	// for the arrow to go to next stageselect screens
	LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_next_normalsize.png", &arrowTexture);
	LoadTexture(g_pDevice, "asset/UI/stage_select/StageSelect_next_bigsize.png", &arrowHoverTexture);

	
	//uiButtons.emplace_back(-0.65f, 0.1f, 0.4f, 0.8f, STAGE, buttonTexture, buttonHoverTexture); // go to 1-1
	//uiButtons.emplace_back(-0.25f, 0.1f, 0.4f, 0.8f, STAGE2, buttonTexture, buttonHoverTexture); // go to 1-2
	//uiButtons.emplace_back(0.15f, 0.1f, 0.4f, 0.8f, STAGE3, buttonTexture, buttonHoverTexture); // go to 1-3
	//uiButtons.emplace_back(0.65f, 0.1f, 0.4f, 0.8f, STAGE4, buttonTexture, buttonHoverTexture); // go to 1-4
	//uiButtons.emplace_back(-0.65f, -0.5f, 0.4f, 0.8f, STAGE5, buttonTexture, buttonHoverTexture); // go to 1-5
	//uiButtons.emplace_back(-0.25f, -0.5f, 0.4f, 0.8f, STAGE6, buttonTexture, buttonHoverTexture); // go to 1-6
	//uiButtons.emplace_back(0.15f, -0.5f, 0.4f, 0.8f, STAGE7, buttonTexture, buttonHoverTexture); // go to 1-6
	//uiButtons.emplace_back(0.65f, -0.5f, 0.4f, 0.8f, BOSS, buttonTexture, buttonHoverTexture); // go to boss

	uiButtons.emplace_back(-0.65f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // go to 1-1
	uiButtons.emplace_back(-0.3f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // go to 1-2
	uiButtons.emplace_back(0.06f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // go to 1-3
	uiButtons.emplace_back(0.43f, 0.15f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);   // go to 1-4
	uiButtons.emplace_back(-0.5f, -0.33f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // go to 1-5
	uiButtons.emplace_back(-0.12f, -0.33f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture); // go to 1-6
	uiButtons.emplace_back(0.26f, -0.33f, 0.4f, 0.8f, GAMEPLAY, buttonTexture, buttonHoverTexture);  // go to 1-7
	uiButtons.emplace_back(0.63f, -0.33f, 0.4f, 0.8f, GAMEPLAY, bossButtonTexture, bossButtonHoverTexture);  // go to boss
	for (auto& btn : uiButtons)
	{
		btn.SetHitboxScale(0.5f, 0.3f);
		btn.SetHitboxOffset(-0.05f);
	}

	// right arrow so I can go to stage select world 2
	uiButtons.emplace_back(0.92f, -0.2f, 0.4f, 0.6f, STAGESELECT2, arrowTexture, arrowHoverTexture);
	uiButtons.back().SetHitboxScale(0.4f, 0.6f);
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

	if (g_numberTexture)
	{
		SetColor(1.0f, 1.0f, 1.0f, 1.0f);

		float numberSize = 0.04f;  // for the size of the numbers
		float spacing = 0.06f;     // for the space between the numbers

		DrawAreaNumber(1, 1, -0.7f, 0.16f, numberSize, spacing);    // 1-1
		DrawAreaNumber(1, 2, -0.35f, 0.16f, numberSize, spacing);   // 1-2
		DrawAreaNumber(1, 3, 0.01f, 0.16f, numberSize, spacing);    // 1-3
		DrawAreaNumber(1, 4, 0.38f, 0.16f, numberSize, spacing);    // 1-4
		DrawAreaNumber(1, 5, -0.55f, -0.32f, numberSize, spacing);  // 1-5
		DrawAreaNumber(1, 6, -0.17f, -0.32f, numberSize, spacing);  // 1-6	
		DrawAreaNumber(1, 7, 0.21f, -0.32f, numberSize, spacing);   // 1-7
	}

}

void StageSelect::DrawAreaNumber(int world, int stage, float x, float y, float size, float space)
{
	// Draw world number (first digit: 1)
	RenderImage(x, y, size, size, g_numberTexture, world, 1, 10, false, 0.0f, false);

	// Draw stage number (second digit: 1-8) with spacing
	RenderImage(x + space, y, size, size, g_numberTexture, stage, 1, 10, false, 0.0f, false);
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
	if (bossButtonTexture)
	{
		bossButtonTexture->Release();
		bossButtonTexture = nullptr;
	}
	if (bossButtonHoverTexture)
	{
		bossButtonHoverTexture->Release();
		bossButtonHoverTexture = nullptr;
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

	if (arrowTexture)
	{
		arrowTexture->Release();
		arrowTexture = nullptr;
	}
	if (arrowHoverTexture)
	{
		arrowHoverTexture->Release();
		arrowHoverTexture = nullptr;
	}

	uiButtons.clear();
	g_mouseIndicator.Cleanup();
}
