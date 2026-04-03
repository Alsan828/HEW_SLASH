#include "CakeScene.h"
#include <algorithm>
#include <cmath>

namespace {
    constexpr float kPi = 3.1415926535f;
    constexpr float kGeometryEpsilon = 0.0005f;
}

//the construct
CakeScene::CakeScene(SceneManager* manager)
{
    sceneManager = manager;

    isCakeCut = false;
    isPlateShown = false;
    cutAnimTimer = 0.0f;
    worldSplitProgress = 0.0f;
    wasAttackActive = false;
    cutAngle = 0.0f;
    cutCenterU = 0.5f;
    cutCenterV = 0.5f;
    UpdateCutVectors();
    ResetCakePieces();
}

// Initialize the stage
bool CakeScene::Init()
{
    Audio::PlayBGM(BackgroundMusic::THE_CAKE, true);

    g_gameState = STATE_PLAYING;

    g_mapManager.SwitchMap("cake", -1, -1);

    ResetGame();

    LoadTexture(g_pDevice, "asset/UI/cake/cake.png", &cakeTexture);
    LoadTexture(g_pDevice, "asset/UI/cake/plate.png", &plateTexture);

    LoadTexture(g_pDevice, "asset/UI/cake/nextbutton_normal.png", &continueTexture);
    LoadTexture(g_pDevice, "asset/UI/cake/nextbutton_hover.png", &continueHoverTexture);

    uiButtons.emplace_back(0.8f, -0.9f, 0.4f, 0.8f, RESULT, continueTexture, continueHoverTexture);
    uiButtons.back().SetHitboxScale(0.25f, 0.13f);
    uiButtons.back().SetHitboxOffset(-0.06f);

    isCakeCut = false;
    isPlateShown = false;
    cutAnimTimer = 0.0f;
    worldSplitProgress = 0.0f;
    wasAttackActive = false;
    cutAngle = 0.0f;
    cutCenterU = 0.5f;
    cutCenterV = 0.5f;
    UpdateCutVectors();
    ResetCakePieces();

    return true;
}

void CakeScene::ResetCakePieces()
{
    cakePieces.clear();
    cakePieces.emplace_back();
}

bool CakeScene::GetAttackSegment(float& startX, float& startY, float& endX, float& endY) const
{
    if (!g_player.isAttacking && !g_player.isDashing) {
        return false;
    }

    const float cutRange = 0.4f;
    const float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    const float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    startX = playerCenterX;
    startY = playerCenterY;
    endX = playerCenterX;
    endY = playerCenterY;

    if (g_player.isDashing) {
        const float velX = g_player.velocityX;
        const float velY = g_player.velocityY;
        const float magnitude = sqrtf((velX * velX) + (velY * velY));
        if (magnitude <= 0.001f) {
            return false;
        }

        endX += (velX / magnitude) * cutRange;
        endY += (velY / magnitude) * cutRange;
        return true;
    }

    endX += g_player.facingRight ? cutRange : -cutRange;
    return true;
}

bool CakeScene::GetAttackSegmentUV(float& startU, float& startV, float& endU, float& endV) const
{
    float startX = 0.0f;
    float startY = 0.0f;
    float endX = 0.0f;
    float endY = 0.0f;
    if (!GetAttackSegment(startX, startY, endX, endY)) {
        return false;
    }

    startU = (startX - cakeX) / cakeWidth;
    startV = 1.0f - ((startY - cakeY) / cakeHeight);
    endU = (endX - cakeX) / cakeWidth;
    endV = 1.0f - ((endY - cakeY) / cakeHeight);
    return true;
}

bool CakeScene::GetCakeEntryPoint(float& hitX, float& hitY) const
{
    float startX = 0.0f;
    float startY = 0.0f;
    float endX = 0.0f;
    float endY = 0.0f;
    if (!GetAttackSegment(startX, startY, endX, endY)) {
        return false;
    }

    const float minX = cakeX;
    const float maxX = cakeX + cakeWidth;
    const float minY = cakeY;
    const float maxY = cakeY + cakeHeight;
    const float dirX = endX - startX;
    const float dirY = endY - startY;
    const bool startInside = (startX >= minX && startX <= maxX && startY >= minY && startY <= maxY);
    const bool endInside = (endX >= minX && endX <= maxX && endY >= minY && endY <= maxY);

    if (fabsf(dirX) <= 0.0001f && fabsf(dirY) <= 0.0001f) {
        if (!endInside) {
            return false;
        }

        hitX = std::max(minX, std::min(endX, maxX));
        hitY = std::max(minY, std::min(endY, maxY));
        return true;
    }

    float tEnter = 0.0f;
    float tExit = 1.0f;
    auto clipAxis = [&](float start, float dir, float axisMin, float axisMax) {
        if (fabsf(dir) <= 0.0001f) {
            return start >= axisMin && start <= axisMax;
        }

        float invDir = 1.0f / dir;
        float t1 = (axisMin - start) * invDir;
        float t2 = (axisMax - start) * invDir;
        if (t1 > t2) {
            std::swap(t1, t2);
        }

        tEnter = std::max(tEnter, t1);
        tExit = std::min(tExit, t2);
        return tEnter <= tExit;
    };

    if (!clipAxis(startX, dirX, minX, maxX) || !clipAxis(startY, dirY, minY, maxY)) {
        return false;
    }

    if (startInside) {
        hitX = std::max(minX, std::min(endX, maxX));
        hitY = std::max(minY, std::min(endY, maxY));
        return true;
    }

    if (tExit < 0.0f || tEnter > 1.0f) {
        return false;
    }

    const float hitT = std::max(0.0f, std::min(tEnter, 1.0f));
    hitX = startX + dirX * hitT;
    hitY = startY + dirY * hitT;
    return true;
}

void CakeScene::UpdateCutCenterFromWorldPoint(float worldX, float worldY)
{
    const float localU = (worldX - cakeX) / cakeWidth;
    const float localV = 1.0f - ((worldY - cakeY) / cakeHeight);

    cutCenterU = std::max(0.0f, std::min(localU, 1.0f));
    cutCenterV = std::max(0.0f, std::min(localV, 1.0f));
}

// for checking if the player hit the cake or not
bool CakeScene::CheckPlayerAttackHitsCake()
{
    float startU = 0.0f;
    float startV = 0.0f;
    float endU = 0.0f;
    float endV = 0.0f;
    if (!GetAttackSegmentUV(startU, startV, endU, endV)) {
        return false;
    }

    for (const CakePiece& piece : cakePieces) {
        if (DoesSegmentHitPiece(startU, startV, endU, endV, piece)) {
            return true;
        }
    }

    return false;
}

float CakeScene::DetermineCutAngle() const
{
    if (g_player.isDashing) {
        const float velX = g_player.velocityX;
        const float velY = g_player.velocityY;
        const float length = sqrtf(velX * velX + velY * velY);
        if (length > 0.0001f) {
            return atan2f(velY, velX);
        }
    }

    if (g_player.isAttacking) {
        return g_player.facingRight ? 0.0f : kPi;
    }

    const float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    const float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;
    const float cakeCenterX = cakeX + cakeWidth * 0.5f;
    const float cakeCenterY = cakeY + cakeHeight * 0.5f;
    return atan2f(cakeCenterY - playerCenterY, cakeCenterX - playerCenterX);
}

void CakeScene::UpdateCutVectors()
{
    cutDirX = cosf(cutAngle);
    cutDirY = sinf(cutAngle);

    float length = sqrtf(cutDirX * cutDirX + cutDirY * cutDirY);
    if (length <= 0.0001f) {
        cutDirX = 1.0f;
        cutDirY = 0.0f;
        length = 1.0f;
    }

    cutDirX /= length;
    cutDirY /= length;

    // Split direction is perpendicular to the slash direction.
    cutNormalX = -cutDirY;
    cutNormalY = cutDirX;
}

LinearClipPlane CakeScene::BuildCurrentCutPlane() const
{
    LinearClipPlane plane = {};
    plane.normalX = cutNormalX;
    plane.normalY = -cutNormalY;
    plane.centerU = cutCenterU;
    plane.centerV = cutCenterV;
    plane.keepSide = 1.0f;
    return plane;
}

std::vector<CakeScene::CakePoint> CakeScene::ClipPolygonWithPlane(const std::vector<CakePoint>& polygon, const LinearClipPlane& plane) const
{
    std::vector<CakePoint> output;
    if (polygon.empty()) {
        return output;
    }

    auto signedDistance = [&](const CakePoint& point) {
        float side = ((point.u - plane.centerU) * plane.normalX) + ((point.v - plane.centerV) * plane.normalY);
        return side * plane.keepSide;
    };

    CakePoint previous = polygon.back();
    float previousDistance = signedDistance(previous);
    bool previousInside = previousDistance >= -kGeometryEpsilon;

    for (const CakePoint& current : polygon) {
        float currentDistance = signedDistance(current);
        bool currentInside = currentDistance >= -kGeometryEpsilon;

        if (previousInside != currentInside) {
            float t = 0.0f;
            float denominator = previousDistance - currentDistance;
            if (fabsf(denominator) > kGeometryEpsilon) {
                t = previousDistance / denominator;
            }

            t = std::max(0.0f, std::min(t, 1.0f));
            CakePoint intersection = {
                previous.u + ((current.u - previous.u) * t),
                previous.v + ((current.v - previous.v) * t)
            };
            output.push_back(intersection);
        }

        if (currentInside) {
            output.push_back(current);
        }

        previous = current;
        previousDistance = currentDistance;
        previousInside = currentInside;
    }

    return output;
}

std::vector<CakeScene::CakePoint> CakeScene::BuildPiecePolygon(const CakePiece& piece) const
{
    std::vector<CakePoint> polygon = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };

    for (const LinearClipPlane& plane : piece.clipPlanes) {
        polygon = ClipPolygonWithPlane(polygon, plane);
        if (polygon.empty()) {
            break;
        }
    }

    return polygon;
}

float CakeScene::ComputePolygonArea(const std::vector<CakePoint>& polygon) const
{
    if (polygon.size() < 3) {
        return 0.0f;
    }

    float twiceArea = 0.0f;
    for (size_t i = 0; i < polygon.size(); ++i) {
        const CakePoint& current = polygon[i];
        const CakePoint& next = polygon[(i + 1) % polygon.size()];
        twiceArea += (current.u * next.v) - (next.u * current.v);
    }

    return twiceArea * 0.5f;
}

CakeScene::CakePoint CakeScene::ComputePolygonCentroid(const std::vector<CakePoint>& polygon) const
{
    CakePoint centroid = { 0.5f, 0.5f };
    if (polygon.empty()) {
        return centroid;
    }

    const float area = ComputePolygonArea(polygon);
    if (fabsf(area) <= kGeometryEpsilon) {
        float sumU = 0.0f;
        float sumV = 0.0f;
        for (const CakePoint& point : polygon) {
            sumU += point.u;
            sumV += point.v;
        }

        centroid.u = sumU / polygon.size();
        centroid.v = sumV / polygon.size();
        return centroid;
    }

    float factor = 0.0f;
    float centroidU = 0.0f;
    float centroidV = 0.0f;
    for (size_t i = 0; i < polygon.size(); ++i) {
        const CakePoint& current = polygon[i];
        const CakePoint& next = polygon[(i + 1) % polygon.size()];
        factor = (current.u * next.v) - (next.u * current.v);
        centroidU += (current.u + next.u) * factor;
        centroidV += (current.v + next.v) * factor;
    }

    centroid.u = centroidU / (6.0f * area);
    centroid.v = centroidV / (6.0f * area);
    return centroid;
}

bool CakeScene::DoesSegmentHitPiece(float startU, float startV, float endU, float endV, const CakePiece& piece) const
{
    const std::vector<CakePoint> polygon = BuildPiecePolygon(piece);
    if (polygon.size() < 3 || fabsf(ComputePolygonArea(polygon)) <= kGeometryEpsilon) {
        return false;
    }

    auto pointInsidePolygon = [&](float u, float v) {
        bool inside = false;
        for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
            const CakePoint& current = polygon[i];
            const CakePoint& previous = polygon[j];
            bool crosses = ((current.v > v) != (previous.v > v));
            if (!crosses) {
                continue;
            }

            float edgeX = previous.u + ((current.u - previous.u) * ((v - previous.v) / (current.v - previous.v)));
            if (u < edgeX) {
                inside = !inside;
            }
        }
        return inside;
    };

    if (pointInsidePolygon(startU, startV) || pointInsidePolygon(endU, endV)) {
        return true;
    }

    auto orientation = [](float ax, float ay, float bx, float by, float cx, float cy) {
        return ((bx - ax) * (cy - ay)) - ((by - ay) * (cx - ax));
    };

    auto onSegment = [](float ax, float ay, float bx, float by, float px, float py) {
        return px >= std::min(ax, bx) - kGeometryEpsilon &&
            px <= std::max(ax, bx) + kGeometryEpsilon &&
            py >= std::min(ay, by) - kGeometryEpsilon &&
            py <= std::max(ay, by) + kGeometryEpsilon;
    };

    auto segmentsIntersect = [&](float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy) {
        float o1 = orientation(ax, ay, bx, by, cx, cy);
        float o2 = orientation(ax, ay, bx, by, dx, dy);
        float o3 = orientation(cx, cy, dx, dy, ax, ay);
        float o4 = orientation(cx, cy, dx, dy, bx, by);

        bool properIntersection =
            ((o1 > kGeometryEpsilon && o2 < -kGeometryEpsilon) || (o1 < -kGeometryEpsilon && o2 > kGeometryEpsilon)) &&
            ((o3 > kGeometryEpsilon && o4 < -kGeometryEpsilon) || (o3 < -kGeometryEpsilon && o4 > kGeometryEpsilon));

        if (properIntersection) {
            return true;
        }

        if (fabsf(o1) <= kGeometryEpsilon && onSegment(ax, ay, bx, by, cx, cy)) return true;
        if (fabsf(o2) <= kGeometryEpsilon && onSegment(ax, ay, bx, by, dx, dy)) return true;
        if (fabsf(o3) <= kGeometryEpsilon && onSegment(cx, cy, dx, dy, ax, ay)) return true;
        if (fabsf(o4) <= kGeometryEpsilon && onSegment(cx, cy, dx, dy, bx, by)) return true;

        return false;
    };

    for (size_t i = 0; i < polygon.size(); ++i) {
        const CakePoint& current = polygon[i];
        const CakePoint& next = polygon[(i + 1) % polygon.size()];
        if (segmentsIntersect(startU, startV, endU, endV, current.u, current.v, next.u, next.v)) {
            return true;
        }
    }

    return false;
}

// for when the player cuts the cake
bool CakeScene::CutCake()
{
    bool hadExistingCuts = isCakeCut;

    float hitX = 0.0f;
    float hitY = 0.0f;
    if (!GetCakeEntryPoint(hitX, hitY)) {
        return false;
    }

    float startU = 0.0f;
    float startV = 0.0f;
    float endU = 0.0f;
    float endV = 0.0f;
    if (!GetAttackSegmentUV(startU, startV, endU, endV)) {
        return false;
    }

    cutAngle = DetermineCutAngle();
    UpdateCutVectors();
    UpdateCutCenterFromWorldPoint(hitX, hitY);

    LinearClipPlane cutPlane = BuildCurrentCutPlane();
    std::vector<CakePiece> nextPieces;
    nextPieces.reserve(cakePieces.size() * 2);

    bool didSplitAnyPiece = false;
    for (const CakePiece& piece : cakePieces) {
        if (piece.clipPlanes.size() >= (MAX_LINEAR_CLIP_PLANES - 1) ||
            !DoesSegmentHitPiece(startU, startV, endU, endV, piece)) {
            nextPieces.push_back(piece);
            continue;
        }

        CakePiece positivePiece = piece;
        CakePiece negativePiece = piece;

        LinearClipPlane keepPositive = cutPlane;
        LinearClipPlane keepNegative = cutPlane;
        keepPositive.keepSide = 1.0f;
        keepNegative.keepSide = -1.0f;

        positivePiece.clipPlanes.push_back(keepPositive);
        negativePiece.clipPlanes.push_back(keepNegative);

        const std::vector<CakePoint> positivePolygon = BuildPiecePolygon(positivePiece);
        const std::vector<CakePoint> negativePolygon = BuildPiecePolygon(negativePiece);
        bool positiveValid = positivePolygon.size() >= 3 && fabsf(ComputePolygonArea(positivePolygon)) > kGeometryEpsilon;
        bool negativeValid = negativePolygon.size() >= 3 && fabsf(ComputePolygonArea(negativePolygon)) > kGeometryEpsilon;

        if (positiveValid && negativeValid) {
            nextPieces.push_back(positivePiece);
            nextPieces.push_back(negativePiece);
            didSplitAnyPiece = true;
        }
        else {
            nextPieces.push_back(piece);
        }
    }

    if (!didSplitAnyPiece) {
        return false;
    }

    cakePieces.swap(nextPieces);
    isCakeCut = true;
    isPlateShown = false;
    if (!hadExistingCuts) {
        worldSplitProgress = 0.0f;
    }
    cutAnimTimer = 0.0f;
    return true;
}

void CakeScene::Update(float deltaTime)
{
    if (!isPlateShown)
    {
        UpdateGame(deltaTime);

        bool attackActive = g_player.isAttacking || g_player.isDashing;
        if (attackActive && !wasAttackActive) {
            CutCake();
        }
        wasAttackActive = attackActive;

        if (isCakeCut) {
            worldSplitProgress = std::min(worldSplitProgress + (deltaTime / CUT_FEEDBACK_DURATION), 1.0f);
            cutAnimTimer += deltaTime;
            if (cutAnimTimer > CUT_ANIM_DURATION) {
                isPlateShown = true;
            }
        }
    }
    else
    {
        g_inputSystem.Update();

        for (auto& btn : uiButtons)
        {
            if (btn.Process() == UIButtonResult::Clicked)
            {
                g_gameStats.UpdateTime(g_gameElapsedTime);
                g_gameStats.CalculateFinalScore();
                sceneManager->SwitchScene(btn.GetTargetScene());
            }
        }
    }
}

void CakeScene::GetPiecePresentation(const CakePiece& piece, float splitOffset, float& offsetX, float& offsetY, float& rotation) const
{
    offsetX = 0.0f;
    offsetY = 0.0f;
    rotation = 0.0f;

    if (splitOffset <= 0.0f) {
        return;
    }

    const std::vector<CakePoint> polygon = BuildPiecePolygon(piece);
    if (polygon.size() < 3) {
        return;
    }

    const CakePoint centroid = ComputePolygonCentroid(polygon);
    float dirX = centroid.u - 0.5f;
    float dirY = 0.5f - centroid.v;
    float length = sqrtf((dirX * dirX) + (dirY * dirY));

    if (length <= kGeometryEpsilon) {
        dirX = 0.0f;
        dirY = 1.0f;
        length = 1.0f;
    }

    dirX /= length;
    dirY /= length;

    float radialDistance = sqrtf(((centroid.u - 0.5f) * (centroid.u - 0.5f)) + ((centroid.v - 0.5f) * (centroid.v - 0.5f)));
    float spreadScale = 0.75f + (radialDistance * 0.9f);

    offsetX = dirX * splitOffset * spreadScale;
    offsetY = dirY * splitOffset * spreadScale;
    rotation = splitOffset * 0.7f * ((dirX >= 0.0f) ? -1.0f : 1.0f);
}

void CakeScene::DrawCakePieces(float centerX, float centerY, float width, float height, float splitOffset)
{
    if (!cakeTexture) {
        return;
    }

    const float baseX = centerX - width * 0.5f;
    const float baseY = centerY - height * 0.5f;

    for (const CakePiece& piece : cakePieces) {
        const std::vector<CakePoint> polygon = BuildPiecePolygon(piece);
        if (polygon.size() < 3 || fabsf(ComputePolygonArea(polygon)) <= kGeometryEpsilon) {
            continue;
        }

        float offsetX = 0.0f;
        float offsetY = 0.0f;
        float rotation = 0.0f;
        GetPiecePresentation(piece, splitOffset, offsetX, offsetY, rotation);

        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        if (piece.clipPlanes.empty()) {
            SetLinearClip(false);
        }
        else {
            SetLinearClipPlanes(piece.clipPlanes.data(), static_cast<int>(piece.clipPlanes.size()));
        }

        RenderImage(
            baseX + offsetX,
            baseY + offsetY,
            width,
            height,
            cakeTexture,
            0,
            1,
            1,
            false,
            rotation,
            false
        );
    }

    SetLinearClip(false);
}

// for drawing the cake when I cut it
void CakeScene::DrawCakeSequence()
{
    const float screenX = cakeX - g_camera.GetX();
    const float screenY = cakeY - g_camera.GetY();
    const float cakeCenterX = screenX + cakeWidth * 0.5f;
    const float cakeCenterY = screenY + cakeHeight * 0.5f;

    if (!isCakeCut)
    {
        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(screenX, screenY, cakeWidth, cakeHeight, cakeTexture, 0, 1, 1);
    }
    else if (!isPlateShown)
    {
        DrawCakePieces(cakeCenterX, cakeCenterY, cakeWidth, cakeHeight, WORLD_SPLIT_DISTANCE * worldSplitProgress);
    }
    else
    {
        const float plateWidth = 1.2f;
        const float plateHeight = 1.2f;
        const float plateX = -plateWidth / 2;
        const float plateY = -plateHeight / 2;

        SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        RenderImage(plateX, plateY, plateWidth, plateHeight, plateTexture, 0, 1, 1);

        float pieceCountBonus = 0.0075f * static_cast<float>(std::max(0, static_cast<int>(cakePieces.size()) - 2));
        float plateSpread = std::min(PLATE_SPLIT_DISTANCE + pieceCountBonus, 0.17f);
        DrawCakePieces(0.0f, 0.0f, cakeWidth, cakeHeight, plateSpread);

        for (const auto& btn : uiButtons)
            btn.Draw(0.65f);
    }
}

void CakeScene::Draw()
{
    if (!isPlateShown)
    {
        DrawGame();
    }

    DrawCakeSequence();
}

// Cleanup
void CakeScene::Uninit()
{
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

    if (continueTexture)
    {
        continueTexture->Release();
        continueTexture = nullptr;
    }
    if (continueHoverTexture)
    {
        continueHoverTexture->Release();
        continueHoverTexture = nullptr;
    }

    uiButtons.clear();
    cakePieces.clear();
    g_mouseIndicator.Cleanup();
}
