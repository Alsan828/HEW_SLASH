#include "CakeScene.h"

//the construct
CakeScene::CakeScene(SceneManager* manager)
{
    sceneManager = manager;

    isCakeCut = false;
    isPlateShown = false;
    cutAnimTimer = 0.0f;
    cutDirection = VERTICAL_CUT; // default to vertical
}

// Initialize the stage
bool CakeScene::Init()
{

    g_gameState = STATE_PLAYING;

    g_mapManager.SwitchMap("cake", -1, -1);

    ResetGame();

    LoadTexture(g_pDevice, "asset/UI/cake/cake.png", &cakeTexture);
    LoadTexture(g_pDevice, "asset/UI/cake/plate.png", &plateTexture);

    // reset the stats
  /*  isCakeCut = false;
    showPlate = false;
    cutAnimTimer = 0.0f;
    cutDirection = VERTICAL_CUT;*/
   
    return true;
}


// for checking if the player hit the cake or not
bool CakeScene::CheckPlayerAttackHitsCake()
{
    // if player is not attacking nor dashing dont do anything
    if (!g_player.isAttacking && !g_player.isDashing) {
        return false;
    }

    // for the player attack hitbox with the cake
    float cutRange = 0.4f;  // to check how far is the player from the cake to cut it

    // for the current position of the player. /2 so we know the actual center
    float playerCenterX = g_player.posX + PLAYER_WIDTH / 2;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT / 2;

    // Determine attack position at the center of the player
    float playerAttackX = playerCenterX;
    float playerAttackY = playerCenterY;

    if (g_player.isAttacking) { // if facing right
        // For normal attacks, only extend in facing direction
        if (g_player.facingRight) {
            playerAttackX += cutRange;
        }
        else { // if facing left
            playerAttackX -= cutRange;
        }
    }
    else if (g_player.isDashing) { // if the player is dashing
        float velX = g_player.velocityX; // horzontal velocity
        float velY = g_player.velocityY; // vertical velocity

        // for checking the lengh of the vector.
        // for example: is I move 3 units to the right and 4 up, we use pytagorean method which will be sqtr((3*3) + (4*4))
        float magnitude = sqrtf((velX * velX) + (velY * velY));

        if (magnitude > 0.001f) {
            playerAttackX += (velX / magnitude) * cutRange;
            playerAttackY += (velY / magnitude) * cutRange;
        }
    }

    // cake center position. /2 so we know the actual center
    float cakeCenterX = cakeX + cakeWidth / 2; 
    float cakeCenterY = cakeY + cakeHeight / 2;

    // Check collision. use absolute value to get always positive distance
    float distanceX = abs(playerAttackX - cakeCenterX); // horizontal distance
    float distanceY = abs(playerAttackY - cakeCenterY); // vertical distance

    // Hitbox matches cake size exactly
    bool hit = (distanceX < cakeWidth / 2 && distanceY < cakeHeight / 2);

    return hit;
}

// for the direction of the cut
CutDirection CakeScene::DetermineCutDirection()
{
    // if player is dashing
    if (g_player.isDashing) {
        float velX = g_player.velocityX;
        float velY = g_player.velocityY;

        // calculate angle of movement in radians
        float angle = atan2f(velY, velX);
        float angleDegrees = angle * 180.0f / 3.14159f; // it converts radians to degrees 
        if (angleDegrees < 0) angleDegrees += 360.0f;    // normalize the angle bwteeen 0 and 360 of range


        // Determine cut based on dash direction (8 directions)
        if ((angleDegrees >= 337.5f || angleDegrees < 22.5f) ||
            (angleDegrees >= 157.5f && angleDegrees < 202.5f)) {
            return HORIZONTAL_CUT; // cuts horizontally
        }
        else if ((angleDegrees >= 67.5f && angleDegrees < 112.5f) ||
            (angleDegrees >= 247.5f && angleDegrees < 292.5f)) {
            return VERTICAL_CUT; // cut vertically
        }
        else if ((angleDegrees >= 22.5f && angleDegrees < 67.5f) ||
            (angleDegrees >= 202.5f && angleDegrees < 247.5f)) {
            return DIAGONAL_FROM_BOTTOM_CUT; // diagonal cut from top left to bottom right or viceversa
        }
        else {
            return DIAGONAL_FROM_TOP_CUT; // diagonal cut from bottom left to top right or viceversa
        }
    }

    // if player attack the cake to cut it
    if (g_player.isAttacking) {
        return VERTICAL_CUT; // the cut will always be a vertical line ( the line will be different depending on which way the player cuts the cake)
    }

    // determine the cut based on theplayer position relative to cake
    float playerCenterX = g_player.posX;
    float playerCenterY = g_player.posY;
    float cakeCenterX = cakeX + cakeWidth / 2;
    float cakeCenterY = cakeY + cakeHeight / 2;

    float deltaX = playerCenterX - cakeCenterX;
    float deltaY = playerCenterY - cakeCenterY;

    // if the player is more above or below than left or right of the cake
    if (abs(deltaY) > abs(deltaX)) {
        return VERTICAL_CUT;
    }
    else {
        return HORIZONTAL_CUT;
    }
}

// for when the player cuts the cake
void CakeScene::CutCake()
{
    isCakeCut = true;
    cutAnimTimer = 0.0f; // starts the animation timer from 0

    cutDirection = DetermineCutDirection(); // to determine what direction has to be cut

}

void CakeScene::Update(float deltaTime)
{
    // if the cake has not been cut yet
    if (!isCakeCut)
    {
        UpdateGame(deltaTime); // normal gameplay update

        // if the player hit the cake
        if (CheckPlayerAttackHitsCake())
        {
            CutCake(); // it cuts the cake
        }
    }
    // after cutting the cake but before I see the plate
    else if (!isPlateShown)
    {
        UpdateGame(deltaTime); // updates the game

        cutAnimTimer += deltaTime; // so the animation of the cut happens

        // if the animation finished
        if (cutAnimTimer > CUT_ANIM_DURATION)
        {
            isPlateShown = true;
        }
    }
    else // if I see the plate
    {
        g_inputSystem.Update();

        // Check for mouse click to advance to results
        if (g_inputSystem.IsMouseLeftDown())
        {
            g_gameStats.UpdateTime(g_gameElapsedTime); // Update game statistics with elapsed time
            g_gameStats.CalculateFinalScore(); // Calculate final score based on performance
            sceneManager->SwitchScene(RESULT); // go to result scene
        }
    }
}

// for drawing the cake when I cut it
void CakeScene::DrawCakeSequence()
{
    // Convert cake world position to screen position using camera
    float screenX = cakeX - g_camera.GetX();
    float screenY = cakeY - g_camera.GetY();

    if (!isCakeCut)
    {
        // Draw whole cake
        RenderImage(screenX, screenY, cakeWidth, cakeHeight, cakeTexture, 0, 1, 1);
    }
    // for the moment when the plater is not shown yet
    else if (!isPlateShown)
    {
        // Draw splitting animation based on cut direction
        float progress = cutAnimTimer / CUT_ANIM_DURATION;
        float splitOffset = progress * SPLIT_DISTANCE;

        if (cutDirection == VERTICAL_CUT) { // cut vertically. split the cake to left and right
            float leftX = screenX - splitOffset;
            RenderImage(leftX, screenY, cakeWidth / 2, cakeHeight, cakeTexture, 0, 1, 2);

            float rightX = screenX + cakeWidth / 2 + splitOffset;
            RenderImage(rightX, screenY, cakeWidth / 2, cakeHeight, cakeTexture, 1, 1, 2);
        }
        else if (cutDirection == HORIZONTAL_CUT) { // cut horizontally. split cake top and bottom
            float topY = screenY + splitOffset;
            RenderImage(screenX, topY, cakeWidth, cakeHeight / 2, cakeTexture, 0, 2, 1);

            float bottomY = screenY - splitOffset;
            RenderImage(screenX, bottomY, cakeWidth, cakeHeight / 2, cakeTexture, 1, 2, 1);
        }
        else if (cutDirection == DIAGONAL_FROM_TOP_CUT) { // diagonal cut
            float leftX = screenX - splitOffset * 0.7f;
            float leftY = screenY + splitOffset * 0.7f;
            RenderImage(leftX, leftY, cakeWidth / 2, cakeHeight, cakeTexture, 0, 1, 2, false, 0.785f);

            // Right piece moves right-down, rotates +45 degrees
            float rightX = screenX + cakeWidth / 2 + splitOffset * 0.7f;
            float rightY = screenY - splitOffset * 0.7f;
            RenderImage(rightX, rightY, cakeWidth / 2, cakeHeight, cakeTexture, 1, 1, 2, false, 0.785f);
        }
        else { // diagonal cut
            float leftX = screenX - splitOffset * 0.7f;
            float leftY = screenY - splitOffset * 0.7f;
            RenderImage(leftX, leftY, cakeWidth / 2, cakeHeight, cakeTexture, 0, 1, 2, false, -0.785f);

            // Right piece moves right-up, rotates -45 degrees
            float rightX = screenX + cakeWidth / 2 + splitOffset * 0.7f;
            float rightY = screenY + splitOffset * 0.7f;
            RenderImage(rightX, rightY, cakeWidth / 2, cakeHeight, cakeTexture, 1, 1, 2, false, -0.785f);
        }
    }
    else // for when the plate is also shown and the cake on top
    {
        // draw plate in the back
        float plateWidth = 1.2f;
        float plateHeight = 1.2f;
        float plateX = -plateWidth / 2;
        float plateY = -plateHeight / 2;

        RenderImage(plateX, plateY, plateWidth, plateHeight, plateTexture, 0, 1, 1);


        // draw the cut cake pieces on plate based on cut direction
        float cakePieceWidth = cakeWidth / 2;
        float cakePieceHeight = cakeHeight;
        float cakeOffsetY = 0.0f;

        if (cutDirection == VERTICAL_CUT) // when the cut is vertical
        { 
            // left piece of cake
            RenderImage(-cakePieceWidth - 0.05f, cakeOffsetY - cakePieceHeight / 2,
                cakePieceWidth, cakePieceHeight, cakeTexture, 0, 1, 2);
            // right piece of cake
            RenderImage(0.05f, cakeOffsetY - cakePieceHeight / 2,
                cakePieceWidth, cakePieceHeight, cakeTexture, 1, 1, 2);
        }
        else if (cutDirection == HORIZONTAL_CUT) // when the cut is horizontal
        { 
            // top piece of cake
            RenderImage(-cakeWidth / 2, cakeOffsetY + 0.05f,
                cakeWidth, cakeHeight / 2, cakeTexture, 0, 2, 1);
            // bottom piece of cake
            RenderImage(-cakeWidth / 2, cakeOffsetY - cakeHeight / 2 - 0.05f,
                cakeWidth, cakeHeight / 2, cakeTexture, 1, 2, 1);
        }
        else if (cutDirection == DIAGONAL_FROM_TOP_CUT)  // when the cut is diagonal from top left to bottom right or viceversa
        { 
            // left piece of cake
            RenderImage(-cakePieceWidth - 0.05f, cakeOffsetY - cakePieceHeight / 2,
                cakePieceWidth, cakePieceHeight, cakeTexture, 0, 1, 2, false, 0.785f);
            // right piece of cake
            RenderImage(0.05f, cakeOffsetY - cakePieceHeight / 2,
                cakePieceWidth, cakePieceHeight, cakeTexture, 1, 1, 2, false, 0.785f);
        }
        else  // when the cut is diagonal from bottom left to top right or viceversa
        {
            // left piece of cake
            RenderImage(-cakePieceWidth - 0.05f, cakeOffsetY - cakePieceHeight / 2,
                cakePieceWidth, cakePieceHeight, cakeTexture, 0, 1, 2, false, -0.785f);
            // righr piece of cake
            RenderImage(0.05f, cakeOffsetY - cakePieceHeight / 2,
                cakePieceWidth, cakePieceHeight, cakeTexture, 1, 1, 2, false, -0.785f);
        }
    }
}


void CakeScene::Draw()
{
    if (!isPlateShown)
    {
        // Draw normal game
        DrawGame();
    }

    // if the plate if shown, draw cake sequence
    DrawCakeSequence();
}



// Draw the stage
//void CakeScene::Draw()
//{
//    // Call your global draw function
//    DrawGame();
//
//}


// Cleanup
void CakeScene::Uninit()
{
    //ResetGame();  // Reset game state

    //CleanUpGameWorld();  // Release all textures and cleanup
    
    if (cakeTexture)
    {
        cakeTexture->Release();
        cakeTexture = nullptr;
    }

    if (plateTexture)
    {
        plateTexture->Release();
        plateTexture = nullptr;
    }

}