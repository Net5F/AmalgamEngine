#include "StageGraphic.h"
#include "SpriteTools.h"
#include "Transforms.h"
#include "BoundingBox.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"
#include <SDL2_gfxPrimitives.h>

namespace AM
{
namespace ResourceImporter
{
StageGraphic::StageGraphic(const SDL_Rect& inLogicalExtent)
: AUI::Widget({320, 58, 1297, 1022}, "StageGraphic")
, stageXCoords{}
, stageYCoords{}
{
}

void StageGraphic::updateStage(const SDL_Rect& spriteTextureExtent,
                               const SDL_Point& stageOrigin,
                               const SDL_Point& actualSpriteImageOffset)
{
    // Calculate where the stage is on the screen, for our generated graphics.
    std::vector<SDL_Point> screenPoints{};
    calcStageScreenPoints(spriteTextureExtent, stageOrigin,
                          actualSpriteImageOffset, screenPoints);

    // Move the stage graphic coords to the correct positions.
    moveStageGraphic(screenPoints);
}

void StageGraphic::render(const SDL_Point& windowTopLeft)
{
    // If this widget is fully clipped, don't render it.
    if (SDL_RectEmpty(&clippedExtent)) {
        return;
    }

    renderStage(windowTopLeft);
}

void StageGraphic::calcStageScreenPoints(
    const SDL_Rect& spriteTextureExtent, const SDL_Point& stageOrigin,
    const SDL_Point& actualSpriteImageOffset,
    std::vector<SDL_Point>& stageScreenPoints)
{
    /* Transform the world positions to screen points. */
    std::array<SDL_FPoint, 4> screenPoints{};

    // Push the points in the correct order.
    BoundingBox stageWorldExtent{SpriteTools::calcSpriteStageWorldExtent(
        spriteTextureExtent, stageOrigin)};
    const Vector3& minPoint{stageWorldExtent.min};
    const Vector3& maxPoint{stageWorldExtent.max};
    Vector3 point{minPoint.x, minPoint.y, minPoint.z};
    screenPoints[0] = Transforms::worldToScreen(point, 1);

    point = {maxPoint.x, minPoint.y, minPoint.z};
    screenPoints[1] = Transforms::worldToScreen(point, 1);

    point = {maxPoint.x, maxPoint.y, minPoint.z};
    screenPoints[2] = Transforms::worldToScreen(point, 1);

    point = {minPoint.x, maxPoint.y, minPoint.z};
    screenPoints[3] = Transforms::worldToScreen(point, 1);

    // Account for the gizmo's position and the image's position.
    SDL_Point actualStageOrigin{
        AUI::ScalingHelpers::logicalToActual(stageOrigin)};
    int finalXOffset{actualSpriteImageOffset.x + actualStageOrigin.x};
    int finalYOffset{actualSpriteImageOffset.y + actualStageOrigin.y};

    // Scale and offset each point, then push it into the return vector.
    for (SDL_FPoint& point : screenPoints) {
        // Scale and round the point.
        point.x = std::round(AUI::ScalingHelpers::logicalToActual(point.x));
        point.y = std::round(AUI::ScalingHelpers::logicalToActual(point.y));

        // Offset the point.
        point.x += finalXOffset;
        point.y += finalYOffset;

        // Cast to int and push into the return vector.
        stageScreenPoints.push_back(
            {static_cast<int>(point.x), static_cast<int>(point.y)});
    }
}

void StageGraphic::moveStageGraphic(
    std::vector<SDL_Point>& stageScreenPoints)
{
    // Set the coords for the bottom face of the stage. (coords 0 - 3, starting
    // from top left and going clockwise.)
    stageXCoords[0] = stageScreenPoints[0].x;
    stageYCoords[0] = stageScreenPoints[0].y;
    stageXCoords[1] = stageScreenPoints[1].x;
    stageYCoords[1] = stageScreenPoints[1].y;
    stageXCoords[2] = stageScreenPoints[2].x;
    stageYCoords[2] = stageScreenPoints[2].y;
    stageXCoords[3] = stageScreenPoints[3].x;
    stageYCoords[3] = stageScreenPoints[3].y;
}

void StageGraphic::renderStage(const SDL_Point& windowTopLeft)
{
    /* Offset all the points. */
    std::array<Sint16, 4> offsetXCoords{};
    for (std::size_t i = 0; i < offsetXCoords.size(); ++i) {
        offsetXCoords[i] = stageXCoords[i] + windowTopLeft.x;
    }

    std::array<Sint16, 4> offsetYCoords{};
    for (std::size_t i = 0; i < offsetYCoords.size(); ++i) {
        offsetYCoords[i] = stageYCoords[i] + windowTopLeft.y;
    }

    /* Draw the stage's floor bounds. */
    filledPolygonRGBA(AUI::Core::getRenderer(), &(offsetXCoords[0]),
                      &(offsetYCoords[0]), 4, 0, 149, 0,
                      static_cast<Uint8>(STAGE_ALPHA));
}

} // End namespace ResourceImporter
} // End namespace AM
